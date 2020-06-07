#include <Windows.h>
#include <tchar.h> 

LPTSTR  SkipArg(LPCTSTR targv);
VOID  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);


int _tmain(int argc, LPTSTR argv[])
{

	DWORD i;
	HANDLE hReadPitpe, hWritePipe;
	TCHAR command1[MAX_PATH];
	SECURITY_ATTRIBUTES pipeSA = { sizeof(SECURITY_ATTRIBUTES),NULL,TRUE };

	PROCESS_INFORMATION processInfo1, processInfo2;
	STARTUPINFO startupInfo1, startupInfo2;
	LPTSTR targv = GetCommandLine();

	GetStartupInfo(&startupInfo1);
	GetStartupInfo(&startupInfo2);

	targv = SkipArg(targv);
	i = 0;
	while (*targv != _T('=') && *targv != _T('\0')) {
		command1[i] = *targv;
		targv++;
		i++;
	}
	command1[i] = '\0';
	if (*targv == '\0')
		ReportError(_T("No command separator found."), 2, FALSE);
	
	targv = SkipArg(targv);

	CreatePipe(&hReadPitpe, &hWritePipe, &pipeSA, 0);

	startupInfo1.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	startupInfo1.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	startupInfo1.hStdOutput = hWritePipe;
	startupInfo1.dwFlags = STARTF_USESTDHANDLES;

	CreateProcess(NULL, command1, NULL,NULL,
		TRUE,/*Inherit handle*/
		0, NULL, NULL, &startupInfo1, &processInfo1);
	CloseHandle(processInfo1.hThread);
	CloseHandle(hWritePipe);

	startupInfo2.hStdInput = hReadPitpe;
	startupInfo2.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	startupInfo2.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	startupInfo2.dwFlags = STARTF_USESTDHANDLES;

	CreateProcess(NULL, targv, NULL,NULL,
		TRUE,/*iNHREITA */
		0, NULL, NULL, &startupInfo2, &processInfo2);

	CloseHandle(processInfo2.hThread);
	CloseHandle(hReadPitpe);


	WaitForSingleObject(processInfo1.hProcess, INFINITE);
	WaitForSingleObject(processInfo2.hProcess, INFINITE);
	CloseHandle(processInfo1.hProcess);
	CloseHandle(processInfo2.hProcess);
	return 0;

}
