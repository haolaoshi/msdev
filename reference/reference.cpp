// The myPuts function writes a null-terminated string to
// the standard output device.
 
// The export mechanism used here is the __declspec(export)
// method supported by Microsoft Visual Studio, but any
// other export method supported by your development
// environment may be substituted.
 
#include <windows.h>

#include <tchar.h>
#include<stdarg.h>
#include <stdio.h>

#define EOF (-1)
 
#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

#ifdef _UNICODE 
#define _memtchr wmemch
#else
#define _memtchr memchr
#endif
		
__declspec(dllexport) VOID __cdecl  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage)
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

__declspec(dllexport) VOID __cdecl  ReportException(LPCTSTR userMessage,DWORD exceptionCode)
{
	ReportError(userMessage,0,TRUE);
	if(exceptionCode != 0)
		RaiseException(
		(0x0FFFFFFF & exceptionCode ) | 0xE0000000,0,0,NULL);
	return;
}

__declspec(dllexport) DWORD __cdecl Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...)
{
	va_list pFlagList;
	LPBOOL pFlag;
	int iFlag = 0, iArg;

	va_start(pFlagList, OptStr);
	while ((pFlag = va_arg(pFlagList, LPBOOL)) != NULL && iFlag < (int)_tcslen(OptStr)) {
		*pFlag = FALSE;
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == _T('-'); iArg++)
			*pFlag = _memtchr(argv[iArg], OptStr[iFlag], _tcslen(argv[iArg])) != NULL;
		iFlag++;
	}
	va_end(pFlagList);
	for (iArg = 1; iArg < argc && argv[iArg][0] == _T('-'); iArg++);

	return iArg;
}


__declspec(dllexport) int __cdecl myPuts(LPWSTR lpszMsg)
{
    DWORD cchWritten;
    HANDLE hConout;
    BOOL fRet;
 
    // Get a handle to the console output device.

    hConout = CreateFileW(L"CONOUT$",
                         GENERIC_WRITE,
                         FILE_SHARE_WRITE,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if (INVALID_HANDLE_VALUE == hConout)
        return EOF;
 
    // Write a null-terminated string to the console output device.
 
    while (*lpszMsg != L'\0')
    {
        fRet = WriteConsole(hConout, lpszMsg, 1, &cchWritten, NULL);
        if( (FALSE == fRet) || (1 != cchWritten) )
            return EOF;
        lpszMsg++;
    }
    return 1;
}
__declspec(dllexport) BOOL __cdecl PrintStrings(HANDLE hOut, ...)
{
	DWORD msgLen, count;
	LPCTSTR pMsg;
	va_list pMsgList;
	va_start(pMsgList, hOut);
	while ((pMsg = va_arg(pMsgList, LPCTSTR)) != NULL) {
		msgLen = _tcslen(pMsg);
		if (!WriteConsole(hOut, pMsg, msgLen, &count, NULL) && !WriteFile(hOut, pMsg, msgLen * sizeof(TCHAR), &count, NULL))
			va_end(pMsgList);
		return FALSE;
	}
	va_end(pMsgList);
	return TRUE;
}

__declspec(dllexport) BOOL __cdecl PrintMsg(HANDLE hOut, LPCTSTR pMsg)
{
	return PrintStrings(hOut, pMsg, NULL);
}

__declspec(dllexport) BOOL __cdecl ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo) {
	HANDLE hIn, hOut;
	DWORD charIn, flag;
	BOOL success;

	hIn = CreateFile(_T("CONIN$"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	hOut = CreateFile(_T("CONOUT$"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	flag = echo ? ENABLE_ECHO_INPUT : 0;
	success = SetConsoleMode(hIn, ENABLE_LINE_INPUT | flag | ENABLE_PROCESSED_INPUT) && SetConsoleMode(hOut, ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_PROCESSED_OUTPUT)
		&& PrintStrings(hOut, pPromptMsg, NULL)
		&& ReadConsole(hIn, pResponse, maxChar - 2, &charIn, NULL);

	if (success)
		pResponse[charIn - 2] = '\0';
	else
		ReportError(_T("ConsolePrompt failure."), 0, TRUE);
	CloseHandle(hIn);
	CloseHandle(hOut);

	return success;
} 
#ifdef __cplusplus
}
#endif