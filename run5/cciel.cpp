/* Chapter 5 cci explicit link version */
#include <Windows.h>
#include <tchar.h>


BOOL CreateProcessA(
  LPCSTR                lpApplicationName,
  LPSTR                 lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCSTR                lpCurrentDirectory,
  LPSTARTUPINFOA        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
);


VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);

int _tmain(int argc,LPTSTR argv[])
{
	BOOL (__cdecl *cci_f)(LPCTSTR,LPCTSTR,DWORD);
	HMODULE hDll;
	FARPROC pcci;
	hDll = LoadLibrary(argv[4]);
	pcci = GetProcAddress(hDll,"cci_f");
	cci_f = (BOOL (__cdecl *)(LPCTSTR,LPCTSTR,DWORD)) pcci;

	if(!cci_f(argv[2],argv[3],atoi(argv[1]))){
		FreeLibrary(hDll);
		ReportError(_T("cci failed."),6,TRUE);
	}
	FreeLibrary(hDll);
	return 0;
}