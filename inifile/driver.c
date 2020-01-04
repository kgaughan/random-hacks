/*                                       vim:set ts=4 sw=4 noai sr sta et cin:
 * Debugging driver file for inifile.
 * This file is in the Public Domain.
 */

#include <stdio.h>
#include <stdlib.h>
#include "inifile.h"

int main(void)
{
    INIFile* ini;

    ini = INI_Load("test.ini");
    if (ini != NULL)
    {
        INI_Dump(ini);

        INI_Write(ini, "First Section", "doobie", "dah");
        INI_Dump(ini);

        INI_Write(ini, "New Section", "diddley", "dee");
        INI_Dump(ini);

        INI_DeleteEntry(ini, "First Section", "doobie");
        INI_Dump(ini);

        INI_DeleteEntry(ini, "New Section", "diddley");
        INI_Dump(ini);

        INI_Write(ini, "Yet another section", "rubbish", "Dublin");
        INI_Dump(ini);

        INI_DeleteSection(ini, "Yet another section");
        INI_Dump(ini);

        INI_Write(ini, "First Section", "Value1", "New Value 1");
        INI_Dump(ini);

        INI_Write(ini, "First Section", "Value1", "New Value 2");
        INI_Dump(ini);

        if (INI_HasSection(ini, "First Section"))
            puts("Section Found!");
        else
            puts("Bugger!");

        if (INI_HasEntry(ini, "Second Section", "Value3"))
            puts("Entry Found!");
        else
            puts("Bugger!");

        INI_Save(ini);
        INI_Free(ini);
    }

    ini = INI_Load("nonexistant.ini");
    if (ini != NULL)
        INI_Free(ini);

    return EXIT_SUCCESS;
}

