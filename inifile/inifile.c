/*                                       vim:set ts=4 sw=4 noai sr sta et cin:
 * inifile.c
 * by Keith Gaughan <kmgaughan@eircom.net>
 *
 * Manages .ini Files.
 *
 * Copyright (c) Keith Gaughan, 2004.
 * All Rights Reserved.
 *
 * Permission is granted to anyone to use this software for any purpose on any
 * computer system, and to alter it and redistribute it, subject to the
 * following restrictions:
 *
 *  1. The author is not responsible for the consequences of use of this
 *     software, no matter how awful, even if they arise from flaws in it.
 *
 *  2. The origin of this software must not be misrepresented, either by
 *     explicit claim or by omission. Since few users ever read sources,
 *     credits must appear in the documentation.
 *
 *  3. Altered versions must be plainly marked as such, and must not be
 *     misrepresented as being the original software. Since few users ever
 *     read sources, credits must appear in the documentation.
 *
 *  4. The author reserves the right to change the licencing details on any
 *     future releases of this package.
 *
 *  5. This notice may not be removed or altered.
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "inifile.h"

/*
 * Maintainer's Notes
 * ==================
 *
 * The file is organised with linked lists right now, but later on, it may use
 * hashing to speed up access. I can't see how the extra speed would be needed
 * though. Linked lists are *just* fine*. The point is, *don't* depend on
 * INIFile having a static layout, so don't go poking about at its innards.
 *
 * On Windows, I could have used Get/WritePrivateProfileBlah, but they're
 * genuinely slow, and at least these can be used everywhere and it's a better
 * interface. And the format's a good one, so why not?
 *
 * Practically speaking, all you're probably going to need is INI_Load(),
 * INI_Free(), INI_Read(), and *maybe* the ones for listing off or checking if
 * entries or sections exist. The rest, on the other hand, are editing, and I
 * doubt you'll need that most of the time.
 *
 * Hmmm... there's an idea... a read-only version of the library...
 *
 * If the double-indirected pointers don't make sense, read the note in
 * INI_Write() for an explaination. It's not as tricky as it seems.
 */

static int MarkEnd(char* p, char breaker)
{
    while (*p != breaker && *p != '\n')
        p++;
    if (*p != breaker)
        return 0;
    *p = '\0';
    return 1;
}

static char* BreakLine(char* line)
{
    char* pch;
    char* val;

    /* Search for the seperator, but check that it's there for idiots incapable
       of passing a well-formed file. */
    for (pch = line; *pch != '=' && *pch != '\n'; pch++);
    if (*pch != '=')
    {
        fprintf(stderr, "Bad key/value pair.\n");
        return NULL;
    }

    val = pch + 1;

    /* Find the end of the whitespace, if any. */
    do
    {
        *pch = '\0';
        pch--;
    } while (*pch == ' ' || *pch == '\t');

    /* Add the null to the value. */
    for (pch = val; *pch != '\n'; pch++);
    *pch = '\0';

    return val;
}

/********************************************* Loading, Saving and Freeing **/

INIFile* INI_Load(const char* path)
{
    FILE*        fp;

    INIFile*     ini;
    INISection** ppSect;
    INIEntry**   ppEntry;

    char         buf[4096];
    char*        pch;
    char*        val;

    assert(path != NULL);
    assert(strlen(path) > 0);

    /*
     * The `goto' is used only to make error handling less painful. It's error
     * handling, so it's ok. I'm not utterly happy with this code anyway.
     */

    ini = NULL;
    fp = fopen(path, "rt");
    if (fp == NULL)
        goto CATASTROPHE;

    ini = (INIFile*) malloc(sizeof(INIFile) + strlen(path) + 1);
    if (ini == NULL)
        goto CATASTROPHE;

    strcpy(ini->path, path);
    ppSect  = &ini->pHead;
    *ppSect = NULL;
    ppEntry = NULL;

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        pch = buf;

        /* Skip whitespace. */
        while (*pch == ' ' || *pch == '\t' || *pch == '\n')
            pch++;

        /*
         * You might say to yourself, "why not just call INI_Write() for each
         * entry?" But that would end up being awfully slow. If it used
         * red-black trees (as it may in the future), it would. However, it
         * doesn't, so to make loading fast and efficient, the code's in here.
         */

        if (*pch == '[')
        {
            /*
             * Section header.
             */

            pch++;

            /* Search for the end. */
            if (MarkEnd(pch, ']'))
            {
                /* Set up the new section. */
                *ppSect = (INISection*) malloc(sizeof(INISection) + strlen(pch) + 1);
                if (*ppSect == NULL)
                    goto CATASTROPHE;
                strcpy((*ppSect)->name, pch);

                /* Set up for the next entry. */
                ppEntry = &(*ppSect)->pHead;

                /* Set up for the next section. */
                ppSect  = &(*ppSect)->pNext;
                *ppSect = NULL;
            }
        }
        else if (ppEntry != NULL && *pch != ';' && *pch != '\0')
        {
            /*
             * Key/token pair.
             *
             * NOTE: The `ppEntry != NULL' above ensures that this doesn't get
             * executed unless we're in a section. We ignore any key/value
             * pairs before the first section header.
             */

            /* Break the line we've read into a value and a key. It does this
               in the buffer, and the key is at the start. */
            val = BreakLine(pch);
            /* Badly formed pair? */
            if (val == NULL)
                goto CATASTROPHE;

            /* Set up the new entry. */
            *ppEntry = (INIEntry*) malloc(sizeof(INIEntry) + strlen(pch) + 1);
            if (*ppEntry == NULL)
                goto CATASTROPHE;
            strcpy((*ppEntry)->key, pch);

            /* Now load the value. Can't use strdup() for portability. */
            (*ppEntry)->val = malloc(strlen(val) + 1);
            if ((*ppEntry)->val == NULL)
            {
                free(*ppEntry);
                *ppEntry = NULL;
                goto CATASTROPHE;
            }
            strcpy((*ppEntry)->val, val);

            /* Set up for the next entry. */
            ppEntry  = &(*ppEntry)->pNext;
            *ppEntry = NULL;
        }
    }

    fclose(fp);
    return ini;

    /* The error handler. */
CATASTROPHE:
    perror("INI_Load");
    if (ini != NULL)
        INI_Free(ini);
    if (fp != NULL)
        fclose(fp);
    return NULL;
}

void INI_Save(INIFile* ini)
{
    FILE*       fp;

    INISection* pSect;
    INIEntry*   pEntry;

    assert(ini != NULL);

    fp = fopen(ini->path, "wt");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open %s.\n", ini->path);
        return;
    }

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
    {
        /* Seems needless, but when the file was read in, there may have been
           empty sections, and we don't want to write them. */
        if (pSect->pHead != NULL)
            fprintf(fp, "\n[%s]\n", pSect->name);

        for (pEntry = pSect->pHead; pEntry != NULL; pEntry = pEntry->pNext)
            fprintf(fp, "%s=%s\n", pEntry->key, pEntry->val);
    }

    fclose(fp);
}

void INI_Free(INIFile* ini)
{
    INISection* pSect;
    INIEntry*   pEntry;
    void*       pToFree;

    assert(ini != NULL);

    pSect = ini->pHead;
    while (pSect != NULL)
    {
        pEntry = pSect->pHead;
        while (pEntry != NULL)
        {
            free(pEntry->val);

            pToFree = pEntry;
            pEntry = pEntry->pNext;
            free(pToFree);
        }

        pToFree = pSect;
        pSect = pSect->pNext;
        free(pToFree);
    }

    free(ini);
}

/*************************************************** Querying and Updating **/

const char* INI_Read(INIFile* ini, const char* section, const char* key)
{
    INISection* pSect;
    INIEntry*   pEntry;

    assert(ini     != NULL);
    assert(section != NULL);
    assert(key     != NULL);

    assert(strlen(section) > 0);
    assert(strlen(key)     > 0);

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
    {
        if (strcmp(pSect->name, section) == 0)
        {
            for (pEntry = pSect->pHead; pEntry != NULL; pEntry = pEntry->pNext)
                if (strcmp(pEntry->key, key) == 0)
                    return pEntry->val;

            break;
        }
    }

    return NULL;
}

int INI_Write(INIFile* ini, const char* section, const char* key, const char* val)
{
    INISection** ppSect;
    INIEntry**   ppEntry;
    char*        pNew;

    assert(ini     != NULL);
    assert(section != NULL);
    assert(key     != NULL);
    assert(val     != NULL);

    assert(strlen(section) > 0);
    assert(strlen(key)     > 0);

    /*
     * What's with the double indirection (ppSect and ppEntry)? Why not just
     * use regular pointers? Well, the code's a minor variant on the classic
     * `dummy header' linked list algorithm for handling empty lists without
     * any special-case code.
     *
     * It may seem clever and tricky, but it's not really.
     *
     * Rather than holding a pointer to the node we're looking at, we hold a
     * pointer to the location *holding* that pointer. This means that we can
     * read from and write to it. The alternative to this is to hold a pointer
     * to the previous node, and this makes things awkward to work with and
     * handling empty lists difficult.
     *
     * Doing things this way, an empty list can be treated the same as if we
     * just didn't find the entry being looked for in the list. When it's
     * empty, the double indirected pointer refers to the head pointer,
     * otherwise to one in the nodes. Nice and clean, isn't it?
     *
     * I discovered this variant in Steve Maguire's book, Writing Solid Code.
     */

    /* Find the section. */
    for (ppSect = &ini->pHead; *ppSect != NULL; ppSect = &(*ppSect)->pNext)
        if (strcmp((*ppSect)->name, section) == 0)
            break;

    if (*ppSect == NULL)
    {
        /* Doesn't exist, so allocate it. */
        *ppSect = (INISection*) malloc(sizeof(INISection) + strlen(section) + 1);
        if (*ppSect == NULL)
            return 0;
        strcpy((*ppSect)->name, section);
        (*ppSect)->pNext = NULL;
        (*ppSect)->pHead = NULL;
    }

    /* Find the entry. */
    for (ppEntry = &(*ppSect)->pHead; *ppEntry != NULL; ppEntry = &(*ppEntry)->pNext)
        if (strcmp((*ppEntry)->key, key) == 0)
            break;

    if (*ppEntry == NULL)
    {
        /* Doesn't exist, so allocate it. */
        *ppEntry = (INIEntry*) malloc(sizeof(INIEntry) + strlen(key) + 1);
        if (*ppEntry == NULL)
            return 0;

        (*ppEntry)->pNext = NULL;
        (*ppEntry)->val   = NULL;
        strcpy((*ppEntry)->key, key);
    }

    /* Allocate storage for the new value. */
    pNew = realloc((*ppEntry)->val, strlen(val) + 1);
    if (pNew == NULL)
    {
        free(*ppEntry);
        *ppEntry = NULL;
        return 0;
    }

    /* Put the new value in. */
    strcpy(pNew, val);
    (*ppEntry)->val = pNew;

    return 1;
}

/**************************************************************** Deletion **/

void INI_DeleteSection(INIFile* ini, const char* section)
{
    INISection** ppSect;
    INIEntry*    pEntry;
    void*        pToFree;

    assert(ini     != NULL);
    assert(section != NULL);

    assert(strlen(section) > 0);

    /* Find the section */
    for (ppSect = &ini->pHead; *ppSect != NULL; ppSect = &(*ppSect)->pNext)
    {
        if (strcmp((*ppSect)->name, section) == 0)
        {
            /* Found it: delete the entries. */
            pEntry = (*ppSect)->pHead;
            while (pEntry != NULL)
            {
                free(pEntry->val);

                pToFree = pEntry;
                pEntry  = pEntry->pNext;
                free(pToFree);
            }

            /* Rechain, and deallocate. */
            pToFree = *ppSect;
            *ppSect = (*ppSect)->pNext;
            free(pToFree);
            break;
        }
    }
}

void INI_DeleteEntry(INIFile* ini, const char* section, const char* key)
{
    INISection** ppSect;
    INIEntry**   ppEntry;
    void*        pToFree;

    assert(ini     != NULL);
    assert(section != NULL);
    assert(key     != NULL);

    assert(strlen(section) > 0);
    assert(strlen(key)     > 0);

    /* Find the section. */
    for (ppSect = &ini->pHead; *ppSect != NULL; ppSect = &(*ppSect)->pNext)
    {
        if (strcmp((*ppSect)->name, section) == 0)
        {
            /* Find the entry. */
            for (ppEntry = &(*ppSect)->pHead; *ppEntry != NULL; ppEntry = &(*ppEntry)->pNext)
            {
                if (strcmp((*ppEntry)->key, key) == 0)
                {
                    free((*ppEntry)->val);

                    /* Rechain and deallocate. */
                    pToFree  = *ppEntry;
                    *ppEntry = (*ppEntry)->pNext;
                    free(pToFree);
                    break;
                }
            }

            /* If the section's empty, rechain and deallocate it. */
            if ((*ppSect)->pHead == NULL)
            {
                pToFree = *ppSect;
                *ppSect = (*ppSect)->pNext;
                free(pToFree);
            }

            break;
        }
    }
}

/********************************************************* Metainformation **/

int INI_HasSection(INIFile* ini, const char* section)
{
    INISection* pSect;

    assert(ini     != NULL);
    assert(section != NULL);

    assert(strlen(section) > 0);

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
        if (strcmp(pSect->name, section) == 0)
            return 1;

    return 0;
}

int INI_HasEntry(INIFile* ini, const char* section, const char* key)
{
    INISection* pSect;
    INIEntry*   pEntry;

    assert(ini     != NULL);
    assert(section != NULL);
    assert(key     != NULL);

    assert(strlen(section) > 0);
    assert(strlen(key)     > 0);

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
    {
        if (strcmp(pSect->name, section) == 0)
        {
            for (pEntry = pSect->pHead; pEntry != NULL; pEntry = pEntry->pNext)
                if (strcmp(pEntry->key, key) == 0)
                    return 1;

            break;
        }
    }

    return 0;
}

size_t INI_SectionCount(INIFile* ini)
{
    INISection* pSect;
    size_t      n;

    assert(ini != NULL);

    n = 0;
    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
        n++;

    return n;
}

size_t INI_EntryCount(INIFile* ini, const char* section)
{
    INISection* pSect;
    INIEntry*   pEntry;
    size_t      n;

    assert(ini     != NULL);
    assert(section != NULL);

    assert(strlen(section) > 0);

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
    {
        if (strcmp(pSect->name, section) == 0)
        {
            n = 0;
            for (pEntry = pSect->pHead; pEntry != NULL; pEntry = pEntry->pNext)
                n++;

            return n;
        }
    }

    return 0;
}

/*
 * If this function or the next one have a bad pointer reference, it's because
 * the idiot who called it didn't allocate enough space in list. Not my fault.
 */

void INI_ListSections(INIFile* ini, char** list)
{
    INISection* pSect;

    assert(ini  != NULL);
    assert(list != NULL);

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
    {
        *list = pSect->name;
        list++;
    }
}

void INI_ListEntries(INIFile* ini, const char* section, char** list)
{
    INISection* pSect;
    INIEntry*   pEntry;

    assert(ini     != NULL);
    assert(section != NULL);
    assert(list    != NULL);

    assert(INI_HasSection(ini, section));

    for (pSect = ini->pHead; pSect != NULL; pSect = pSect->pNext)
    {
        if (strcmp(pSect->name, section) == 0)
        {
            for (pEntry = pSect->pHead; pEntry != NULL; pEntry = pEntry->pNext)
            {
                *list = pEntry->key;
                list++;
            }
        }
    }
}

/************************************************************* Diagnostics **/

void INI_Dump(INIFile* ini)
{
    char** sections;
    char** entries;
    const char* value;

    size_t iSect;
    size_t iEntry;
    size_t nSects;
    size_t nEntries;

    assert(ini != NULL);

    printf("Dump of %s:\n\n", ini->path);
    nSects = INI_SectionCount(ini);
    sections = (char**) malloc(nSects * sizeof(char*));
    INI_ListSections(ini, sections);
    for (iSect = 0; iSect < nSects; iSect++)
    {
        printf("[%s]\n", sections[iSect]);
        nEntries = INI_EntryCount(ini, sections[iSect]);
        entries = malloc(nEntries * sizeof(char*));
        INI_ListEntries(ini, sections[iSect], entries);
        for (iEntry = 0; iEntry < nEntries; iEntry++)
        {
            value = INI_Read(ini, sections[iSect], entries[iEntry]);
            printf(" |\n +- `%s' = `%s'\n", entries[iEntry], value);
        }
        free(entries);
        putchar('\n');
    }
    free(sections);
}

