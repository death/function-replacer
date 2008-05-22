/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴� */
/* Visual Basic Disassembled String Reference Analyser        (c) DEATH  */
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴� */
/* pe.c                                                                  */
/*                                                                       */
/* Portable Executable handling functions                                */
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴� */
#include <windows.h>
#include <imagehlp.h>
#include <stdio.h>
#include <string.h>

#include "include/pe.h"

/* Physical to Virtual Address Routine by NeuRaL_NoiSE */
/* Ported to C by DEATH */
long Phys2VA(char *filename, long phys)
{
    long va;
    long filesize;
    HANDLE hFile;
    HANDLE hMap;
    char *pMap;
    char *pFile;
    short *nFile;
    long *dFile;
    IMAGE_DOS_HEADER *imgDos;
    IMAGE_FILE_HEADER *imgPE;
    IMAGE_SECTION_HEADER *hdrSECTION;
    short nCounter;
    long dwTemp1;
    long dwTemp2;

    hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return(-1);

    filesize = GetFileSize(hFile, NULL);
    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, filesize, NULL);

    pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);

    pFile = pMap;
    nFile = (short *)pFile;
    imgDos = (IMAGE_DOS_HEADER *)pFile;

    if (*nFile == IMAGE_DOS_SIGNATURE) {

        pFile += imgDos->e_lfanew;
        dFile = (long *)pFile;

        if (*dFile == IMAGE_NT_SIGNATURE) {

            pFile += 4;
            imgPE = (IMAGE_FILE_HEADER *)pFile;

            nCounter = imgPE->NumberOfSections;
            pFile += imgPE->SizeOfOptionalHeader;
            pFile += IMAGE_SIZEOF_FILE_HEADER;
            hdrSECTION = (IMAGE_SECTION_HEADER *)pFile;

            while (nCounter != 0) {

                if (nCounter == 1) {
                    goto LastSec;
                } else {

                    dwTemp1 = hdrSECTION->PointerToRawData;
                    hdrSECTION++;
                    dwTemp2 = hdrSECTION->PointerToRawData;

                    if (dwTemp2 == 0) {
                        /* Raw offset of 2nd section is 0 */
                        hdrSECTION--;
                        goto LastSec;
                    }

Check:
                    if ((dwTemp1 < phys && dwTemp2 > phys) || dwTemp1 == phys) {
                        goto FoundSec_1;
                    } else if (dwTemp2 == phys) {
                        dwTemp1 = dwTemp2;
                        goto FoundSec_2;
                    }

                    nCounter--;

                }

            }

            goto Offset_Out;

LastSec:
            dwTemp1 = hdrSECTION->PointerToRawData;
            dwTemp2 = hdrSECTION->SizeOfRawData + dwTemp1;
            hdrSECTION++;
            goto Check;

FoundSec_1:
            hdrSECTION--;
FoundSec_2:
            va = phys;
            va -= dwTemp1;
            dwTemp2 = hdrSECTION->VirtualAddress;
            va += dwTemp2;

        } else {
            va = -1;
        }

    } else {
Offset_Out:
        va = -1;
    }

    UnmapViewOfFile(pMap);

    CloseHandle(hMap);
    CloseHandle(hFile);

    return(va);
}

/* Physical to Relative Virtual Address */
long Phys2RVA(char *filename, long phys)
{
    long rva;
    IMAGE_NT_HEADERS *imgHeader;
    char *pMap;
    char *pFile;
    HANDLE hFile;
    HANDLE hMap;
    IMAGE_DOS_HEADER *imgDos;
    long filesize;

    rva = Phys2VA(filename, phys);

    if (rva != -1) {
        hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            return(-1);

        filesize = GetFileSize(hFile, NULL);
        hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, filesize, NULL);

        pMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);

        pFile = pMap;

        imgDos = (IMAGE_DOS_HEADER *)pFile;
        pFile += imgDos->e_lfanew;
        imgHeader = (IMAGE_NT_HEADERS *)pFile;

        rva += imgHeader->OptionalHeader.ImageBase;

        UnmapViewOfFile(pMap);

        CloseHandle(hMap);
        CloseHandle(hFile);
    }

    return(rva);
}

/* Virtual to Physical Address */
DWORD VA2Phys(char *Filename, DWORD va)
{
    FILE *filep;
    IMAGE_DOS_HEADER        imgDos;
    IMAGE_NT_HEADERS        imgNt;
    IMAGE_SECTION_HEADER    imgSection1;
    IMAGE_SECTION_HEADER    imgSection2;
    short iSections;
    short iCount;

    filep = fopen(Filename, "rb");

    if (filep) {
        fread(&imgDos, sizeof(imgDos), 1, filep);

        if (imgDos.e_magic == IMAGE_DOS_SIGNATURE) {

            fseek(filep, imgDos.e_lfanew, SEEK_SET);
            fread(&imgNt, IMAGE_SIZEOF_NT_OPTIONAL_HEADER, 1, filep);

            if (imgNt.Signature == IMAGE_NT_SIGNATURE) {

                fseek(filep, 0x18, SEEK_CUR);

                iSections = imgNt.FileHeader.NumberOfSections - 1;
                fread(&imgSection1, IMAGE_SIZEOF_SECTION_HEADER, 1, filep);

                for (iCount = 0; iCount < iSections; iCount++) {

                    fread(&imgSection2, IMAGE_SIZEOF_SECTION_HEADER, 1, filep);

                    if (va >= imgSection1.VirtualAddress) {

                        if (va < imgSection2.VirtualAddress) {

                            va -= imgSection1.VirtualAddress;
                            va += imgSection1.PointerToRawData;

                            fclose(filep);

                            return(va);

                        }

                    }

                    memcpy(&imgSection1, &imgSection2, IMAGE_SIZEOF_SECTION_HEADER);

                }

                va -= imgSection1.VirtualAddress;
                va += imgSection1.PointerToRawData;

            }

        }
        fclose(filep);
    }

    return(va);
}

/* Relative Virtual to Physical Address */
DWORD RVA2Phys(char *Filename, DWORD rva)
{
    FILE *filep;
    IMAGE_DOS_HEADER        imgDos;
    IMAGE_NT_HEADERS        imgNt;

    filep = fopen(Filename, "rb");

    if (filep) {

        fread(&imgDos, sizeof(imgDos), 1, filep);

        if (imgDos.e_magic == IMAGE_DOS_SIGNATURE) {

            fseek(filep, imgDos.e_lfanew, SEEK_SET);
            fread(&imgNt, IMAGE_SIZEOF_NT_OPTIONAL_HEADER, 1, filep);

            if (imgNt.Signature == IMAGE_NT_SIGNATURE) {

                fclose(filep);
                rva -= imgNt.OptionalHeader.ImageBase;
                return(VA2Phys(Filename, rva));

            }

        }

        fclose(filep);

    }

    return(0L);
}

/* Get PE entry point */
DWORD GetEP(char *Filename)
{
    FILE *filep;
    IMAGE_DOS_HEADER        imgDos;
    IMAGE_NT_HEADERS        imgNt;
    DWORD dwEntryPoint;

    dwEntryPoint = 0L;

    filep = fopen(Filename, "rb");

    if (filep) {

        fread(&imgDos, sizeof(imgDos), 1, filep);

        if (imgDos.e_magic == IMAGE_DOS_SIGNATURE) {

            fseek(filep, imgDos.e_lfanew, SEEK_SET);
            fread(&imgNt, IMAGE_SIZEOF_NT_OPTIONAL_HEADER, 1, filep);

            if (imgNt.Signature == IMAGE_NT_SIGNATURE) {

                dwEntryPoint += imgNt.OptionalHeader.ImageBase;
                dwEntryPoint += imgNt.OptionalHeader.AddressOfEntryPoint;

            }

        }

        fclose(filep);

    }

    return(dwEntryPoint);
}
