/* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ */
/* Visual Basic Disassembled String Reference Analyser        (c) DEATH  */
/* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ */
/* export.c                                                              */
/*                                                                       */
/* Export references handling functions                                  */
/* ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ */
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

#include "include/util.h"
#include "include/pe.h"
#include "include/export.h"

/* Export references */
char szExportRef[MAX_EXPREF][256];

/* Get all exports */
DWORD GetExports(char *Filename)
{
    IMAGE_DOS_HEADER        imgDos;
    IMAGE_NT_HEADERS        imgNt;
    IMAGE_EXPORT_DIRECTORY  imgExport;
    FILE *filep;
    short iCount;
    DWORD dwSave;
    DWORD dwOffset;

    iCount = 0;

    filep = fopen(Filename, "rb");

    if (filep) {

        fread(&imgDos, sizeof(imgDos), 1, filep);

        if (imgDos.e_magic == IMAGE_DOS_SIGNATURE) {

            fseek(filep, imgDos.e_lfanew, SEEK_SET);

            fread(&imgNt, IMAGE_SIZEOF_NT_OPTIONAL_HEADER, 1, filep);

            if (imgNt.Signature == IMAGE_NT_SIGNATURE) {

                if (imgNt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size != 0) {

                    fseek(filep, VA2Phys(Filename, imgNt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), SEEK_SET);

                    fread(&imgExport, sizeof(imgExport), 1, filep);

                    if (imgExport.Name != 0) {

                        fseek(filep, VA2Phys(Filename, (DWORD )imgExport.AddressOfNames), SEEK_SET);

                        for (iCount = 0; iCount < imgExport.NumberOfNames; iCount++) {

                            fread(&dwOffset, sizeof(DWORD), 1, filep);
                            dwSave = ftell(filep);

                            fseek(filep, VA2Phys(Filename, dwOffset), SEEK_SET);

                            GetASCIIZString(&szExportRef[iCount], filep);

                            fseek(filep, dwSave, SEEK_SET);

                        }

                    }

                }

            }

        }

        fclose(filep);
    }

    return(iCount);
}
