#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "include/util.h"

/* Get ASCIIZ (NULL terminated) string */
void GetASCIIZString(char *szStr, FILE *pFile)
{
    DWORD dwCount;
    short ch;

    ch = fgetc(pFile);
    dwCount = 0;

    if (ch == EOF)
        szStr[dwCount] = '\0';
    else
        szStr[dwCount] = ch;

    while (ch != EOF && ch != '\0') {
        dwCount++;
        ch = fgetc(pFile);

        if (ch == EOF)
            szStr[dwCount] = '\0';
        else
            szStr[dwCount] = ch;
    }
}

void AddStatus(HWND hwnd, char *pszStr)
{
    SendDlgItemMessage(hwnd, IDC_STATUS, LB_ADDSTRING, 0, (LPARAM )(LPCSTR )pszStr);
}

void ClearStatus(HWND hwnd)
{
    SendDlgItemMessage(hwnd, IDC_STATUS, LB_RESETCONTENT, 0, 0);
}
