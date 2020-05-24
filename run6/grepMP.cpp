#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

int _tmain(int argc,LPTSTR argv[])
{
	if (argc <= 2) {
		_tprintf(_T("Create a separate process to search each file on the cmd line.\nEach process is a given a temporary file,in the current directory, \nto receive the results\n usage:  grep  pattern  f1...\n"));
		return TRUE;
	}
	HANDLE hTempFile;
	SECURITY_ATTRIBUTES stdOutSA = {sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
	TCHAR commandLine [MAX_PATH + 100 ];
	STARTUPINFO startupSearch , startup;
	PROCESS_INFORMATION processInfo;
	DWORD iProc,exitCode,dwCreationFlags = 0;
	HANDLE *hProc; /*Pointer to an array of proc handles*/
	typedef struct{
		TCHAR tempFile[MAX_PATH];
	} PROCFILE;
	PROCFILE *procFile; /*Pointer to an array of temp files names*/
	GetStartupInfo(&startupSearch);
	GetStartupInfo(&startup);

	procFile =(PROCFILE*)malloc((argc - 2) * sizeof(PROCFILE));
	hProc = (HANDLE*)malloc((argc - 2 ) * sizeof(HANDLE));
	/*create a separate grep process for each file.*/
	for(iProc = 0 ; iProc < argc - 2; iProc++){
		_stprintf_s(commandLine,_T("grep \"%s\"  \"%s\""),argv[1],argv[iProc + 2]);

		GetTempFileName(_T("."),_T("gtm"),0,procFile[iProc].tempFile);/*FOR SEARCH RESULT*/

		hTempFile = CreateFile(/*This handle is inheritable*/
			procFile[iProc].tempFile,
			GENERIC_WRITE,FILE_SHARE_READ| FILE_SHARE_WRITE,
			&stdOutSA,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		startupSearch.dwFlags = STARTF_USESTDHANDLES;
		startupSearch.hStdOutput = hTempFile;
		startupSearch.hStdError = hTempFile;
		startupSearch.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		/*create a process to execute the cmd line*/
		CreateProcess(NULL,commandLine,NULL,NULL,TRUE,dwCreationFlags,NULL,NULL,&startupSearch,&processInfo);
		CloseHandle(hTempFile);
		CloseHandle(processInfo.hThread);
		hProc[iProc] = processInfo.hProcess;
	}
	/*process all running. wait for complete*/
	for(iProc = 0; iProc < argc - 2; iProc += MAXIMUM_WAIT_OBJECTS)
		WaitForMultipleObjects(min(MAXIMUM_WAIT_OBJECTS,argc - 2 - iProc),&hProc[iProc],TRUE,INFINITE);
	
	/*Result file sent to stdout usiong cat*/
	for(iProc = 0 ; iProc < argc - 2; iProc ++){
		if(GetExitCodeProcess(hProc[iProc],&exitCode) && exitCode == 0){
			if(argc > 3) _tprintf(_T("%s:\n"),argv[iProc + 2]);
			_stprintf_s(commandLine,_T("cat \"%s\""),procFile[iProc].tempFile);
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