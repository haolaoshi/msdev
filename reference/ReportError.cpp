#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include "refer.h"
/* General-purpose function for reporting system errors.
	Obtain the error number and turn it into the system error message.
	Display this information and the user-specified message to the standard error device.
	UserMessage:		Message to be displayed to standard error device.
	ExitCode:		0 - Return.
				\x11> 0 - ExitProcess with this code.
	PrintErrorMessage:	Display the last system error message if this flag is set. */
	
	
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
			_ftprintf(stderr, _T("%s\n"), lpvSysMsg);
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

VOID ReportException(LPCTSTR userMessage,DWORD exceptionCode)
{
	ReportError(userMessage,0,TRUE);
	if(exceptionCode != 0)
		RaiseException(
		(0x0FFFFFFF & exceptionCode ) | 0xE0000000,0,0,NULL);
	return;
}