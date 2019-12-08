#include <windows.h>


VOID   ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
VOID   ReportException(LPCTSTR userMessage,DWORD exceptionCode);
DWORD  Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);
BOOL  PrintStrings(HANDLE hOut, ...);
BOOL  PrintMsg(HANDLE hOut, LPCTSTR pMsg);
BOOL  ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo);
