#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#define BUFF_SIZE 256


LONG RegQueryValueEx(
	HKEY hKey,
	LPTSTR lpValueName,
	LPDWORD lpReserved,
	LPDWORD lpType,
	LPBYTE lpData,
	LPDWORD lpcbData
);

void _tmain(int argc, TCHAR* argv[])
{


	STARTUPINFO si;

	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (argc <  2) {
		LPTSTR lpValue = (LPTSTR)malloc(BUFF_SIZE * sizeof(TCHAR));
		TCHAR lpName[] = _T("HOST_NAME");

		GetEnvironmentVariable(lpName,lpValue,BUFF_SIZE);
		int n = GetLastError();
		if (n == ERROR_ENVVAR_NOT_FOUND) {
			BOOL succ = SetEnvironmentVariable(lpName, _T("LIU JIAHAO"));
			n = GetLastError();
			if (!succ) printf("Set Failed\n");
			else printf("Set success : %d \n", n);
			GetEnvironmentVariable(lpName, lpValue, BUFF_SIZE);
		}
		while(n == 0 && *lpValue)
			printf("%c", *lpValue++);
		 

		printf("\nUsage : %s [cmdline]\n", argv[0]);

		//free(lpValue); 会引发断点为啥
		return;
	}

	if (!CreateProcess(NULL,//use cmdLine,no module name
		argv[1],
		NULL,//Process no inheritate
		NULL,//Thread no inheritae 
		FALSE,// Handle inheritae false
		0,//No creation flags 
		NULL,//parent env
		NULL,//parent directory
		&si, //STARTUPINFO
		&pi //PROCESS_INFOMATOION
	)) {
		printf("Create Process Failed.\n");
			return;
	}
	//ExitProcess(0);
	//printf("Exit");
	DWORD processID;
	 
	 

	 TerminateProcess(pi.hProcess, 110);


	WaitForSingleObject(pi.hProcess, INFINITE);
	GetExitCodeProcess(pi.hProcess, &processID);
	printf("exit code %d", processID);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

}