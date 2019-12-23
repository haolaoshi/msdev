#pragma once
#include <Windows.h>
#ifdef __cplusplus
extern "C" {  // only need to export C interface if
              // used by C++ source code
#endif

// The export mechanism used here is the __declspec(export)
__declspec(dllexport) VOID __cdecl  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
__declspec(dllexport) VOID __cdecl  ReportException(LPCTSTR userMessage, DWORD exceptionCode);
__declspec(dllexport) DWORD __cdecl Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);

__declspec(dllexport) BOOL __cdecl PrintStrings(HANDLE hOut, ...);
__declspec(dllexport) BOOL __cdecl PrintMsg(HANDLE hOut, LPCTSTR pMsg);
__declspec(dllexport) BOOL __cdecl ConsolePrompt(LPCTSTR pPromptMsg, LPTSTR pResponse, DWORD maxChar, BOOL echo);

__declspec(dllexport) VOID __cdecl  GetArgs(LPCTSTR command, int* pargc, LPTSTR argstr[]);
__declspec(dllexport) LPTSTR __cdecl SkipArg(LPCTSTR targv);
#ifdef __cplusplus
}
#endif