#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage)
{
	DWORD eMsgLen, errNum = GetLastError();
	LPTSTR lpvSysMsg;

	_ftprintf(stderr, _T("%s\n"), userMessage);
	if (printfErrorMessage) {
		eMsgLen = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errNum,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpvSysMsg, 0, NULL);
		if (eMsgLen > 0) {
			_ftprintf(stderr, "%s\n", lpvSysMsg);
		}
		else {
			_ftprintf(stderr, _T("Last Error Number : %d.\n"), errNum);
		}
		if (lpvSysMsg != NULL) LocalFree(lpvSysMsg);
	}

	if (exitCode > 0)
		ExitProcess(exitCode);

	return;
}

