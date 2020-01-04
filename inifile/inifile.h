#ifndef INIFILE_H_INCLUDED
#define INIFILE_H_INCLUDED
/*                                       vim:set ts=4 sw=4 noai sr sta et cin:
 * inifile.h
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

#include <stddef.h>

/*
 * File Format
 * ===========
 *
 * For those of you unfamiliar (!) with the .ini file format, heres an
 * overview:
 *
 *  1. Comment lines are marked by a `;' at the start of the line.
 *  2. The file is broken into sections, the start of each being marked by a
 *     single line containing its name inside square brackets, e.g.
 *     `[my section]'.
 *  3. Each line following (until the next section header) contains a key/value
 *     pair. The format of each line is `key=value'. Whitespace is stripped
 *     from either side of the key, and the value can't spread over multiple
 *     lines.
 *  4. Empty lines are ignored.
 *
 * Here's an example file:
 *
 * > ; Here's a lovely comment.
 * >
 * > [my section]
 * > key 1=value 1
 * > key 2=value 2
 *
 * Simple, isn't it? Now isn't that much better than buggering about with the
 * registry? I wish everybody else thought it was. :-(
 *
 * Limitations
 * ===========
 *
 * The maximum line length is 4096 bytes, including the newline. It's not
 * Unicode aware.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents a file section entry.
 *
 * @note It stores the size of the value buffer so that if a value that's as
 *       long or shorter than the old value is put in, we won't have to
 *       reallocate the buffer, further fragmenting the heap.
 */
typedef struct INIEntry
{
    struct INIEntry* pNext;       /* Next sibling in this section. */
    char*            val;         /* Value of this entry.          */
    char             key[1];      /* Key identifying this entry.   */
} INIEntry;

/**
 * Represents a file section.
 */
typedef struct INISection
{
    struct INISection* pNext;     /* Next sibling in the file.     */
    struct INIEntry*   pHead;     /* Header for its entry list.    */
    char               name[1];   /* Name of this section.         */
} INISection;

/**
 * Represents a file.
 */
typedef struct
{
    struct INISection* pHead;     /* Header for its section list.  */
    char               path[1];   /* Path of .ini file.            */
} INIFile;

/**
 * Loads an .ini file into memory.
 *
 * @param  path  Path to .ini file to load.
 *
 * @return Handle of .ini file, or NULL if could not be loaded.
 */
INIFile* INI_Load(const char* path);

/**
 * Saves an .ini file.
 *
 * @param  ini  Handle.
 *
 * @note  Any comments that were in the file will have been stripped.
 */
void INI_Save(INIFile* ini);

/**
 * Frees an .ini file.
 *
 * @param  ini  Handle.
 *
 * @note Don't free() it yourself unless you like memory leakage.
 */
void INI_Free(INIFile* ini);

/**
 * Reads an entry value.
 *
 * @param  ini      Handle.
 * @param  section  Name of section to query.
 * @param  key      Name of entry to query.
 *
 * @return Entry value, or NULL if nonexistant.
 */
const char* INI_Read(INIFile* ini, const char* section, const char* key);

/**
 * Writes an entry value.
 *
 * @param  ini      Handle.
 * @param  section  Name of section.
 * @param  key      Name of entry to modify.
 * @param  val      New value for the key.
 *
 * @return Non-zero if written, else zero (out of memory).
 */
int INI_Write(INIFile* ini, const char* section, const char* key, const char* val);

/**
 * Deletes a section.
 *
 * @param  ini      Handle.
 * @param  section  Name of section to delete.
 */
void INI_DeleteSection(INIFile* ini, const char* section);

/**
 * Deletes an entry from a section.
 *
 * @param  ini      Handle.
 * @param  section  Name of section.
 * @param  key      Name of entry to delete.
 *
 * @note If the section in question becomes empty it's automatically deleted.
 */
void INI_DeleteEntry(INIFile* ini, const char* section, const char* key);

/**
 * Queries if a section exists.
 *
 * @param  ini      Handle.
 * @param  section  Name of section.
 *
 * @return Non-zero if it exists, else zero.
 */
int INI_HasSection(INIFile* ini, const char* section);

/**
 * Queries if a section contains an entry.
 *
 * @param  ini      Handle.
 * @param  section  Name of section.
 * @param  key      Name of entry.
 *
 * @return Non-zero if it exists, else zero.
 *
 * @note It doesn't matter if the section exists or not.
 */
int INI_HasEntry(INIFile* ini, const char* section, const char* key);

/**
 * Counts the number of sections in the file.
 *
 * @param  ini  Handle.
 *
 * @return Number of sections.
 */
size_t INI_SectionCount(INIFile* ini);

/**
 * Counts the number of entries in a section.
 *
 * @param  ini      Handle.
 * @param  section  Name of section.
 *
 * @return Number of entries.
 */
size_t INI_EntryCount(INIFile* ini, const char* section);

/**
 * Lists the names of the sections in the ini file.
 *
 * @param  ini   Handle.
 * @param  list  Buffer of character pointers to hold list.
 *
 * @note Buffer must be big enough to hold all the pointers.
 * @note Don't mess with the strings! There be dragons!
 * @note Calling INI_DeleteEntry(), INI_DeleteSection(), &c will
 *       invalidate this list.
 */
void INI_ListSections(INIFile* ini, char** list);

/**
 * Lists the entries in a section.
 *
 * @param  ini      Handle.
 * @param  section  Section to list.
 * @param  list     Buffer of character pointers to hold list.
 *
 * @note See INI_ListSections() for further details.
 */
void INI_ListEntries(INIFile* ini, const char* section, char** list);

/**
 * Dumps the contents of the file to the screen.
 *
 * @param  ini  Handle.
 *
 * @note This was primarily written for testing various functions, so it
 *       isn't as fast as it might be.
 */
void INI_Dump(INIFile* ini);

#ifdef __cplusplus
}
#endif

#endif /* INIFILE_H_INCLUDED */

