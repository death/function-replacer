#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commdlg.h>

#include "resource.h"
#include "include/export.h"
#include "include/util.h"
#include "include/rep.h"

HINSTANCE hInst;
BOOL fl2BPDLL;                          /* To-be-patched DLL */
BOOL flREPDLL;                          /* DLL to replace with */
BOOL fl2BRFUNC;                         /* To-be-replaced function */
BOOL flREPFUNC;                         /* Function to replace with */
BOOL flSelect2BRF;                      /* Select to-be-replaced function? */
DWORD dwNumExports;                     /* Number of exports */
char sz2BRFunc[256];                    /* To-be-replaced export name */
char szRepFunc[256];                    /* Replace function */

BOOL CALLBACK dlgMain(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateButtons(HWND hwnd);
void AddStatus(HWND hwnd, char *pszStr);
BOOL GetFileName(HWND hwnd, char *pszFileName, DWORD cbFileName, char *pszTitle);
BOOL CALLBACK dlgAbout(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK dlgSelect(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;

    hInst = hInstance;
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, dlgMain);

    return(msg.wParam);
}

BOOL CALLBACK dlgMain(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char szFileName[512];
    char szBuffer[512];

    switch (uMsg) {
        case WM_INITDIALOG:
            /* Dialog initialize */
            fl2BPDLL = FALSE;
            flREPDLL = FALSE;
            fl2BRFUNC = FALSE;
            flREPFUNC = FALSE;
            UpdateButtons(hDlg);
            AddStatus(hDlg, "Ready...");
            break;
        case WM_COMMAND:
            /* Dialog command */
            switch (LOWORD(wParam)) {
                case IDC_EXIT:
                    /* Exit programme */
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;
                case IDC_BR2BPDLL:
                    /* Browse To-be-patched DLL */
                    GetDlgItemText(hDlg, IDC_2PDLL, szFileName, sizeof(szFileName));

                    switch (GetFileName(hDlg, szFileName, sizeof(szFileName), "Select to-be-patched DLL")) {
                        case TRUE:
                            SetDlgItemText(hDlg, IDC_2PDLL, szFileName);
                            fl2BPDLL = TRUE;
                            fl2BRFUNC = FALSE;
                            SetDlgItemText(hDlg, IDC_F2REP, "Function to replace");
                            UpdateButtons(hDlg);
                            break;
                        default:
                            break;
                    }
                    break;
                case IDC_BRREPDLL:
                    /* Browse replace DLL */
                    GetDlgItemText(hDlg, IDC_REP, szFileName, sizeof(szFileName));

                    switch (GetFileName(hDlg, szFileName, sizeof(szFileName), "Select replace DLL")) {
                        case TRUE:
                            SetDlgItemText(hDlg, IDC_REP, szFileName);
                            flREPDLL = TRUE;
                            flREPFUNC = FALSE;
                            SetDlgItemText(hDlg, IDC_REPFUNC, "Replace with");
                            UpdateButtons(hDlg);
                            break;
                        default:
                            break;
                    }
                    break;
                case IDC_GET2RF:
                    /* Get to-be-replaced function */
                    flSelect2BRF = TRUE;
                    GetDlgItemText(hDlg, IDC_2PDLL, szFileName, sizeof(szFileName));
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SELECT), hDlg, dlgSelect, (LPARAM )szFileName);
                    if (fl2BRFUNC == TRUE) {
                        wsprintf(szBuffer, "Function to replace: %s", sz2BRFunc);
                        SetDlgItemText(hDlg, IDC_F2REP, szBuffer);
                    }
                    UpdateButtons(hDlg);
                    break;
                case IDC_GETREPF:
                    /* Get replace function */
                    flSelect2BRF = FALSE;
                    GetDlgItemText(hDlg, IDC_REP, szFileName, sizeof(szFileName));
                    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SELECT), hDlg, dlgSelect, (LPARAM )szFileName);
                    if (flREPFUNC == TRUE) {
                        wsprintf(szBuffer, "Replace with: %s", szRepFunc);
                        SetDlgItemText(hDlg, IDC_REPFUNC, szBuffer);
                    }
                    UpdateButtons(hDlg);
                    break;
                case IDC_REPLACE:
                    /* Replace the function */
                    ClearStatus(hDlg);
                    GetDlgItemText(hDlg, IDC_2PDLL, szFileName, sizeof(szFileName));
                    GetDlgItemText(hDlg, IDC_REP, szBuffer, sizeof(szBuffer));
                    while (strchr(szBuffer, '\\'))
                        strcpy(szBuffer, strchr(szBuffer, '\\') + 1);
                    ReplaceFunction(hDlg, szFileName, sz2BRFunc, szBuffer, szRepFunc);
                    break;
                case IDC_ABOUT:
                    /* About function replacer */
                    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hDlg, dlgAbout);
                    break;
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            /* Close dialog */
            EndDialog(hDlg, TRUE);
            break;
        default:
            break;
    }

    return(FALSE);
}

void UpdateButtons(HWND hwnd)
{
    if (fl2BPDLL == FALSE)
        EnableWindow(GetDlgItem(hwnd, IDC_GET2RF), FALSE);
    else
        EnableWindow(GetDlgItem(hwnd, IDC_GET2RF), TRUE);

    if (flREPDLL == FALSE)
        EnableWindow(GetDlgItem(hwnd, IDC_GETREPF), FALSE);
    else
        EnableWindow(GetDlgItem(hwnd, IDC_GETREPF), TRUE);

    if (fl2BRFUNC == FALSE || flREPFUNC == FALSE)
        EnableWindow(GetDlgItem(hwnd, IDC_REPLACE), FALSE);
    else
        EnableWindow(GetDlgItem(hwnd, IDC_REPLACE), TRUE);
}

BOOL GetFileName(HWND hwnd, char *pszFileName, DWORD cbFileName, char *pszTitle)
{
    OPENFILENAME ofn;
    char szDefExt[] = "DLL";
    char szFilter[] = {
        "Dynamic Linked Libraries\0"
        "*.DLL\0"
        "All files\0"
        "*.*\0\0"
    };

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hwnd;
    ofn.hInstance = hInst;
    ofn.lpstrFilter = (LPCTSTR )szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = pszFileName;
    ofn.nMaxFile = cbFileName;
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle = pszTitle;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = szDefExt;

    return(GetOpenFileName(&ofn));
}

BOOL CALLBACK dlgAbout(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            EndDialog(hDlg, TRUE);
            break;
    }

    return(FALSE);
}

BOOL CALLBACK dlgSelect(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD dwCount;
    DWORD dwIndex;

    switch (uMsg) {
        case WM_INITDIALOG:
            /* lParam = filename */
            dwNumExports = GetExports((char *)lParam);

            for (dwCount = 0; dwCount < dwNumExports; dwCount++)
                SendDlgItemMessage(hDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM )szExportRef[dwCount]);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDOK:
                    dwIndex = SendDlgItemMessage(hDlg, IDC_LIST, LB_GETCURSEL, 0, 0);

                    if (flSelect2BRF == TRUE) {
                        strcpy(sz2BRFunc, szExportRef[dwIndex]);
                        fl2BRFUNC = TRUE;
                    } else {
                        strcpy(szRepFunc, szExportRef[dwIndex]);
                        flREPFUNC = TRUE;
                    }

                case IDCANCEL:
                    PostMessage(hDlg, WM_CLOSE, 0, 0);
                    break;
                default:
                    break;
            }

            switch (HIWORD(wParam)) {
                case LBN_DBLCLK:
                    PostMessage(hDlg, WM_COMMAND, IDOK, 0);
                    break;
                default:
                    break;
            }
            break;
        case WM_CLOSE:
            EndDialog(hDlg, TRUE);
            break;
    }

    return(FALSE);
}
