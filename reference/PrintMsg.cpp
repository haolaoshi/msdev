/* PrintMsg.c:
	ConsolePrompt
	PrintStrings
	PrintMsg */
	
	
#include <Windows.h>
#include <stdarg.h>
#include <tchar.h>
#include "refer.h"
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
BOOL PrintStrings(HANDLE hOut, ...)
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

BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg)
{
	return PrintStrings(hOut, pMsg, NULL);
}

BOOL ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo) {
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