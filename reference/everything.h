#pragma once
#include <Windows.h>
#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

// The export mechanism used here is the __declspec(export)
 VOID  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
 VOID  ReportException(LPCTSTR userMessage, DWORD exceptionCode);
 DWORD  Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);

 BOOL  PrintStrings(HANDLE hOut, ...);
 BOOL  PrintMsg(HANDLE hOut, LPCTSTR pMsg);
 BOOL  ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo);

 VOID  GetArgs(LPCTSTR command, int* pargc, LPTSTR argstr[]);
 LPTSTR  SkipArg(LPCTSTR targv);
#ifdef __cplusplus
}
#endif