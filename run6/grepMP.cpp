#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

int _tmain(int argc,LPTSTR argv[])
{
	HANDLE hTempFile;
	SECURITY_ATTRIBUTES stdOutSA = {sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
	TCHAR commandLine [MAX_PATH + 100 ];
	STARTUPINFO startupSearch , startup;
	PROCESS_INFORMATION processInfo;
	DWORD iProc,exitCode,dwCreationFlags = 0;
	HANDLE *hProc;
	typedef struct{
		TCHAR tempFile[MAX_PATH];
	} PROCFILE;
	PROCFILE *procFile;
	GetStartupInfo(&startupSearch);
	GetStartupInfo(&startup);

	procFile =(PROCFILE*)malloc((argc - 2) * sizeof(PROCFILE));
	hProc = (HANDLE*)malloc((argc - 2 ) * sizeof(HANDLE));

	for(iProc = 0 ; iProc < argc - 2; iProc++){
		_stprintf(commandLine,_T("grep \"%s\"  \"%s\""),argv[1],argv[iProc + 2]);
		GetTempFileName(_T("."),_T("gtm"),0,procFile[iProc].tempFile);
		hTempFile = CreateFile(procFile[iProc].tempFile,GENERIC_WRITE,FILE_SHARE_READ| FILE_SHARE_WRITE,&stdOutSA,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		startupSearch.dwFlags = STARTF_USESTDHANDLES;
		startupSearch.hStdOutput = hTempFile;
		startupSearch.hStdError = hTempFile;
		startupSearch.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

		CreateProcess(NULL,commandLine,NULL,NULL,TRUE,dwCreationFlags,NULL,NULL,&startupSearch,&processInfo);
		CloseHandle(hTempFile);
		CloseHandle(processInfo.hThread);

	}
	for(iProc = 0; iProc < argc - 2; iProc += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(MAXIMUM_WAIT_OBJECTS,argc - 2 - iProc),&hProc[iProc],TRUE,INFINITE);
	for(iProc = 0 ; iProc < argc - 2; iProc ++){
		if(GetExitCodeProcess(hProc[iProc],&exitCode) && exitCode == 0){
			if(argc > 3) _tprintf(_T("%s:\n"),argv[iProc + 2]);
			_stprintf(commandLine,_T("cat \"%s\""),procFile[iProc].tempFile);
			CreateProcess(NULL,commandLine,NULL,NULL,TRUE,dwCreationFlags,NULL,NULL,&startup,&processInfo);
			WaitForSingleObject(processInfo.hProcess,INFINITE);
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}
		CloseHandle(hProc[iProc]);
		DeleteFile(procFile[iProc].tempFile);
	}
	free(procFile);
	free(hProc);

	return 0;
}