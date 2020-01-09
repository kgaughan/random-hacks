/*                                      vim:set ts=4 sw=4 noai sr sta et cin:
 * ksig.c
 * by Keith Gaughan
 *
 * Randomly selects a signature from a `fortune' file.
 *
 * Copyright (c) Keith Gaughan, 2003
 * This software is free; you can redistribute it and/or modify it under the
 * terms of the Design Science License (DSL). If you didn't receive a copy of
 * the DSL, one can be obtained at <http://www.dsl.org/copyleft/dsl.txt>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static const char* RCS_ID = "$Id: ksig.c,v 1.2 2003/12/15 18:52:43 kgaughan Exp $";

#define SIGS_FILE  ".sigs"
#define INDEX_FILE ".sigs.idx"
#define FIXED_FILE ".fixedsig"

int FileExists(const char* name)
{
    struct stat sb;
    return stat(name, &sb) == 0;
}

off_t GetFileSize(const char* name)
{
    struct stat sb;

    if (stat(name, &sb) == 0)
        return sb.st_size;
    return 0;
}

time_t GetModificationTime(const char* name)
{
    struct stat sb;

    if (stat(name, &sb) == 0)
        return sb.st_mtime;
    return 0;
}

int main(void)
{
    char          buf[BUFSIZ];
    FILE*         f;
    FILE*         fIdx;
    unsigned long nEntries;
    fpos_t        pos;
    unsigned long offset;
    char*         home;

    /* Attempt to set the CWD to `~' */
    home = getenv("HOME");
    if (home != NULL)
      chdir(home);

    /* First, read and print out the fixed signature file, if it exists */
    f = fopen(FIXED_FILE, "r");
    if (f != NULL)
    {
        while (fgets(buf, sizeof buf, f) != NULL)
            fputs(buf, stdout);
        fclose(f);
    }

    f = fopen(SIGS_FILE, "r");
    if (f == NULL)
        return 0;

    /* Does index need to be rebuilt? */
    if (!FileExists(INDEX_FILE) ||
        difftime(GetModificationTime(INDEX_FILE),
                 GetModificationTime(SIGS_FILE)) < 0)
    {
        fIdx = fopen(INDEX_FILE, "wb");
        if (fIdx == NULL)
        {
            /* Aarrgh! Can't build index! */
            fclose(f);
            return 1;
        }

        while (!feof(f))
        {
            fgetpos(f, &pos);
            fwrite(&pos, sizeof(fpos_t), 1, fIdx);

            /* Find the next signature, or the end of the file */
            while (fgets(buf, sizeof buf, f) != NULL &&
                   buf[0] != '%' && buf[1] != '\n');
        }

        fclose(fIdx);
    }

    /* Select a random entry */
    srand(time(NULL));
    nEntries = (unsigned long) GetFileSize(INDEX_FILE) / sizeof(fpos_t);
    offset   = (rand() % nEntries) * sizeof(fpos_t);

    /* Read it in */
    fIdx = fopen(INDEX_FILE, "rb");
    fseek(fIdx, offset, SEEK_SET);
    fread(&pos, sizeof(fpos_t), 1, fIdx);
    fclose(fIdx);

    /* Fetch the signature & print it out */
    fsetpos(f, &pos);
    while (fgets(buf, sizeof buf, f) != NULL && buf[0] != '%' && buf[1] != '\n')
        fputs(buf, stdout);
    fclose(f);

    return 0;
}
