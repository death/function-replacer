#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>

#include "include/pe.h"
#include "include/util.h"

char aRealCode[] = {
    0x56,                               /* push esi                 */
    0xE8, 0x00, 0x00, 0x00, 0x00,       /* call Delta               */
    0x5E,                               /* Delta: pop esi           */
    0x81, 0xEE, 0x05, 0x10, 0x40, 0x00, /* sub esi, 401006          */
    0x8D, 0x86, 0x2C, 0x10, 0x40, 0x00, /* lea eax, [esi][40102C]   */
    0x50,                               /* push eax                 */
    0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, /* call d, [?] <- LL        */
    0x53,                               /* push ebx                 */
    0x8D, 0x9E, 0x2C, 0x10, 0x40, 0x00, /* lea ebx, [esi][40102C + ?] <- DllNameLen + 1 */
    0x53,                               /* push ebx                 */
    0x50,                               /* push eax                 */
    0xFF, 0x15, 0x00, 0x00, 0x00, 0x00, /* call d, [?] <- GPA       */
    0x5B,                               /* pop ebx                  */
    0x5E,                               /* pop esi                  */
    0xFF, 0xE0,                         /* jmp eax                  */
    /* DllName */
    /* FunctionName */
};

char aCode[sizeof(aRealCode)];

DWORD GetImportAddress(char *szFileName, char *szExpectedDll, char *szExpectedImport)
{
    IMAGE_DOS_HEADER        imgDos;
    IMAGE_NT_HEADERS        imgNt;
    IMAGE_IMPORT_DESCRIPTOR imgImport;
    IMAGE_THUNK_DATA        imgTData;
    FILE                   *pFile;
    BOOL                    flExit;
    BOOL                    flFound;
    DWORD                   dwSave1;
    DWORD                   dwSave2;
    char                    szDllName[256];
    char                    szImportName[256];
    DWORD                   dwImportAddress;
    DWORD                   dwCount;

    flFound = FALSE;

    pFile = fopen(szFileName, "rb");

    if (pFile) {

        fread(&imgDos, sizeof(IMAGE_DOS_HEADER), 1, pFile);

        if (imgDos.e_magic == IMAGE_DOS_SIGNATURE) {
            /* File is MZ */

            fseek(pFile, imgDos.e_lfanew, SEEK_SET);
            fread(&imgNt, IMAGE_SIZEOF_NT_OPTIONAL_HEADER, 1, pFile);

            if (imgNt.Signature == IMAGE_NT_SIGNATURE) {
                /* File is PE */

                fseek(pFile, VA2Phys(szFileName, imgNt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress), SEEK_SET);
                flExit = FALSE;

                do {
                    fread(&imgImport, sizeof(IMAGE_IMPORT_DESCRIPTOR), 1, pFile);
                    dwSave1 = ftell(pFile);
                    if (imgImport.Name == 0) break;
                    fseek(pFile, VA2Phys(szFileName, imgImport.Name), SEEK_SET);
                    GetASCIIZString(&szDllName, pFile);

                    if (stricmp(szDllName, szExpectedDll) == 0) {
                        /* Found correct dll */

                        dwCount = 0L;

                        if (imgImport.OriginalFirstThunk != 0)
                            fseek(pFile, VA2Phys(szFileName, (DWORD )imgImport.OriginalFirstThunk), SEEK_SET);
                        else
                            fseek(pFile, VA2Phys(szFileName, (DWORD )imgImport.FirstThunk), SEEK_SET);

                        do {
                            /* Search imports */

                            fread(&imgTData, sizeof(IMAGE_THUNK_DATA), 1, pFile);
                            dwSave2 = ftell(pFile);
                            if (imgTData.u1.Function == NULL) break;

                            fseek(pFile, VA2Phys(szFileName, (DWORD )imgTData.u1.Function) + 2, SEEK_SET);
                            GetASCIIZString(&szImportName, pFile);

                            fseek(pFile, dwSave2, SEEK_SET);

                            if (strcmp(szImportName, szExpectedImport) == 0) {
                                /* Found correct import */

                                dwImportAddress = (DWORD )imgImport.FirstThunk + ((dwCount + 1) * 4) - 4;
                                dwImportAddress += imgNt.OptionalHeader.ImageBase;
                                flFound = TRUE;
                                break;
                            }

                            dwCount++;

                        } while (1);

                        flExit = TRUE;

                    }

                    fseek(pFile, dwSave1, SEEK_SET);

                } while (flExit == FALSE);

            }

        }

        fclose(pFile);

    }

    if (flFound == TRUE)
        return(dwImportAddress);

    return(0L);
}

/* Get all exports */
DWORD GetExportAddress(char *szFileName, char *szExpectedExport)
{
    IMAGE_DOS_HEADER        imgDos;
    IMAGE_NT_HEADERS        imgNt;
    IMAGE_EXPORT_DIRECTORY  imgExport;
    FILE                   *pFile;
    DWORD                   dwCount;
    DWORD                   dwSave;
    DWORD                   dwOffset;
    char                    szExportName[256];
    DWORD                   dwExportAddress;
    BOOL                    flFound;
    WORD                    wOrdinal;

    flFound = FALSE;

    pFile = fopen(szFileName, "rb");

    if (pFile) {

        fread(&imgDos, sizeof(imgDos), 1, pFile);

        if (imgDos.e_magic == IMAGE_DOS_SIGNATURE) {

            fseek(pFile, imgDos.e_lfanew, SEEK_SET);
            fread(&imgNt, IMAGE_SIZEOF_NT_OPTIONAL_HEADER, 1, pFile);

            if (imgNt.Signature == IMAGE_NT_SIGNATURE) {

                if (imgNt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size != 0) {

                    fseek(pFile, VA2Phys(szFileName, imgNt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), SEEK_SET);
                    fread(&imgExport, sizeof(imgExport), 1, pFile);

                    if (imgExport.Name != 0) {

                        fseek(pFile, VA2Phys(szFileName, (DWORD )imgExport.AddressOfNames), SEEK_SET);

                        for (dwCount = 0; dwCount < imgExport.NumberOfNames; dwCount++) {

                            fread(&dwOffset, sizeof(DWORD), 1, pFile);
                            dwSave = ftell(pFile);

                            fseek(pFile, VA2Phys(szFileName, dwOffset), SEEK_SET);
                            GetASCIIZString(&szExportName, pFile);

                            if (strcmp(szExportName, szExpectedExport) == 0) {
                                fseek(pFile, VA2Phys(szFileName, (DWORD )imgExport.AddressOfNameOrdinals) + (dwCount + 1) * 2 - 2 , SEEK_SET);
                                fread(&wOrdinal, sizeof(WORD), 1, pFile);
                                fseek(pFile, VA2Phys(szFileName, (DWORD )imgExport.AddressOfFunctions) + (wOrdinal + 1) * 4 - 4 , SEEK_SET);
                                fread(&dwExportAddress, sizeof(DWORD), 1, pFile);
                                dwExportAddress += imgNt.OptionalHeader.ImageBase;
                                flFound = TRUE;
                                break;
                            }

                            fseek(pFile, dwSave, SEEK_SET);

                        }

                    }

                }

            }

        }

        fclose(pFile);
    }

    if (flFound == TRUE)
        return(dwExportAddress);

    return(0L);
}

void FixCode(DWORD dwLL_VA, DWORD dwGPA_VA, char *DllName)
{
    DWORD *dwPtr;

    memcpy(aCode, aRealCode, sizeof(aCode));

    dwPtr = (DWORD *)&aCode[22];
    *dwPtr = dwLL_VA;

    dwPtr = (DWORD *)&aCode[29];
    *dwPtr += strlen(DllName) + 1;

    dwPtr = (DWORD *)&aCode[37];
    *dwPtr = dwGPA_VA;
}

void WriteDLL(char *szFileName, DWORD dwWriteAddress, char *DllName, char *FuncName)
{
    FILE *pFile;

    pFile = fopen(szFileName, "r+b");

    if (pFile) {

        /* Seek to exported function start */
        fseek(pFile, RVA2Phys(szFileName, dwWriteAddress), SEEK_SET);

        /* Write code */
        fwrite(&aCode, sizeof(aCode), 1, pFile);

        /* Write data */
        fwrite(DllName, strlen(DllName) + 1, 1, pFile);
        fwrite(FuncName, strlen(FuncName) + 1, 1, pFile);

        fclose(pFile);

    }
}

void ReplaceFunction(HWND hwnd, char *psz2BPDLL, char *pszFunc2R, char *pszRepDLL, char *pszRepFunc)
{
    DWORD dwGPA_VA;
    DWORD dwLL_VA;
    DWORD dwDllExport_VA;

    AddStatus(hwnd, "Getting needed function addresses...");
    dwLL_VA = GetImportAddress(psz2BPDLL, "kernel32.dll", "LoadLibraryA");
    dwGPA_VA = GetImportAddress(psz2BPDLL, "kernel32.dll", "GetProcAddress");

    if ((dwLL_VA == 0L) || (dwGPA_VA == 0L)) {
        AddStatus(hwnd, "Cannot find needed function addresses!");
        return;
    }

    AddStatus(hwnd, "Getting DLL export address...");
    dwDllExport_VA = GetExportAddress(psz2BPDLL, pszFunc2R);

    if (dwDllExport_VA == 0L) {
        AddStatus(hwnd, "Cannot find the exported function address!");
        return;
    }

    AddStatus(hwnd, "Fixing code...");
    FixCode(dwLL_VA, dwGPA_VA, pszRepDLL);

    AddStatus(hwnd, "Writing to DLL...");
    WriteDLL(psz2BPDLL, dwDllExport_VA, pszRepDLL, pszRepFunc);

    AddStatus(hwnd, "All done!");
}
