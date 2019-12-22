#include <Windows.h>
#include <tchar.h>
#include "refer.h"
#include "JobMgt.h"


/*
	JobShell.c job management commands;
	jobbg	Run a job in the background.
	jobs	List all background jobs
	kill	Terminate a specified job of job fammily
			There is an option to generate a console control signal.*/

#define MAX_COMMAND_LINE	250
#define MAX_ARG	10


int _tmain(int argc, LPTSTR argv[])
{
	BOOL exitFlag = FALSE;
	TCHAR command[MAX_COMMAND_LINE], * pc;
	DWORD i;
	int localArgc;
	TCHAR argstr[MAX_ARG][MAX_COMMAND_LINE];
	LPTSTR pArgs[MAX_ARG];

	for (i = 0; i < MAX_ARG; i++) pArgs[i] = argstr[i];

	_tprintf(_T("Windows Job Management\n"));

	while (!exitFlag) {

		_tprintf(_T("%s"), _T("JM$"));
		_fgetts(command, MAX_COMMAND_LINE, stdin);
		pc = _tcschr(command, '\n');  //strchr 
		*pc = '\0';
		GetArgs(command, &localArgc, pArgs);
		CharLower(argstr[0]);

		if (_tcscmp(argstr[0], _T("jobbg")) == 0) {

			Jobbg(localArgc, pArgs, command);

		}
		else if (_tcscmp(argstr[0], _T("jobs")) == 0) {
			Jobs(localArgc, pArgs, command);
		}
		else if (_tcscmp(argstr[0], _T("kill")) == 0) {
			Kill(localArgc, pArgs, command);
		}
		else if (_tcscmp(argstr[0], _T("quit")) == 0) {
			exitFlag = TRUE;
		}
		else _tprintf(_T("Illegal command . Try Again!\n"));
	}
	return 0;
}

int Jobbg(int argc, LPTSTR argv[], LPTSTR command)
{
	DWORD fCreate;
	LONG jobNumber;
	BOOL flags[2];
	STARTUPINFO	startup;
	PROCESS_INFORMATION processInfo;
	LPTSTR targv = SkipArg(command);
	GetStartupInfo(&startup);
	Options(argc, argv, _T("cd"), &flags[0], &flags[1], NULL);

	if (argv[1][0] == '-') targv = SkipArg(targv);
	fCreate = flags[0] ? CREATE_NEW_CONSOLE : flags[1] ? DETACHED_PROCESS : 0;
	CreateProcess(NULL, targv, NULL, NULL, TRUE, fCreate | CREATE_SUSPENDED | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup, &processInfo);
	jobNumber = GetJobNumber(&processInfo, targv);
	if (jobNumber >= 0)
		ResumeThread(processInfo.hThread);
	else {
		TerminateProcess(processInfo.hProcess, 3);
		CloseHandle(processInfo.hProcess);
		ReportError(_T("Error : no room in job list."), 0, FALSE);
		return 5;
	}
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	_tprintf(_T("[%d] %d \n"), jobNumber, processInfo.dwProcessId);
	return 0;
}

int Jobs(int argc, LPTSTR argv[], LPTSTR command)
{
	if (!DisplayJobs()) return 1;
	return 0;
}

int Kill(int argc, LPTSTR argv[], LPTSTR command)
{
	DWORD ProcessId, jobNumber, iJobno;
	HANDLE hProcess;
	BOOL ctrlc, ctrlb;
	iJobno = Options(argc, argv, _T("bc"), &ctrlb, &ctrlc, NULL);
	jobNumber = _ttoi(argv[iJobno]);
	ProcessId = FindProcessId(jobNumber);
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessId);
	if (hProcess == NULL) {
		ReportError(_T("Process already terminated\n"), 0, FALSE);
		return 2;
	}
	if (ctrlb)
		GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, ProcessId);
	else if (ctrlc)
		GenerateConsoleCtrlEvent(CTRL_C_EVENT, ProcessId);
	else
		TerminateProcess(hProcess, JM_EXIT_CODE);
	WaitForSingleObject(hProcess, 5000);
	CloseHandle(hProcess);
	_tprintf(_T("Job [%d] terminated or timed out\n"), jobNumber);
	return 0;
}

