#include <windows.h>
#include <tchar.h>

#include <stdio.h>

LPCTSTR SlotName = TEXT("\\\\.\\mailslot\\sample_mailslot");

VOID  GetArgs(LPCTSTR command, int* pargc, LPTSTR argstr[]);

VOID  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
BOOL WriteSlot(HANDLE hSlot, LPCTSTR lpszMessage)
{
    BOOL fResult;
    DWORD cbWritten;

    fResult = WriteFile(hSlot,
        lpszMessage,
        (DWORD)(lstrlen(lpszMessage) + 1) * sizeof(TCHAR),
        &cbWritten,
        (LPOVERLAPPED)NULL);

    if (!fResult)
    {
        printf("WriteFile failed with %d.\n", GetLastError());
        return FALSE;
    }

    _tprintf(_T("received [%s]\n"), lpszMessage);

    return TRUE;
}

int main()
{
    HANDLE hFile;

    hFile = CreateFile(SlotName,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("CreateFile failed with [%s].\n"), SlotName);
        ReportError(_T("CreateFile failed \n"), GetLastError(),TRUE);
        return FALSE;
    }
#define MAX_COMMAND_LINE	250
#define MAX_ARG	10
    BOOL exitFlag = FALSE;
    TCHAR command[MAX_COMMAND_LINE], * pc;
    int localArgc;
    LPTSTR pArgs[MAX_ARG];
   // TCHAR argstr[MAX_ARG][MAX_COMMAND_LINE];

    while (!exitFlag) {

        _tprintf(_T("%s"), _T("JM$"));
        _fgetts(command, MAX_COMMAND_LINE, stdin);
        pc = _tcschr(command, '\n');  //strchr 
        *pc = '\0';
        CharLower(command);

        if (_tcscmp(command, _T("quit")) == 0 || _tcscmp(command, _T("bye")) == 0) {

            exitFlag = TRUE;

        }
        else {
            WriteSlot(hFile, command);
        }
        
    }

    

    CloseHandle(hFile);

    return TRUE;
}