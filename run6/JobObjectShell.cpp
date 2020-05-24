#include <Windows.h>
#include "JobMgt.h"
#include "reference.h"
#include <tchar.h>
#include <stdio.h>


#define MILLION	1000000
HANDLE hJobObject = NULL;

JOBOBJECT_BASIC_LIMIT_INFORMATION basicLimits = { 0,0,JOB_OBJECT_LIMIT_PROCESS_TIME };

int _tmain(int argc, LPTSTR argv[])
{
	LARGE_INTEGER processTimeLimit;
	/**/
	hJobObject = NULL;
	processTimeLimit.QuadPart = 0;
	if (argc >= 2) processTimeLimit.QuadPart = atoi((const char*)argv[1]);
	basicLimits.PerProcessUserTimeLimit.QuadPart = processTimeLimit.QuadPart * 10 * MILLION;
	hJobObject = CreateJobObject(NULL, NULL);
	SetInformationJobObject(hJobObject, JobObjectBasicLimitInformation, &basicLimits, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
	/*process command call jobbg, jobs , etc.*/
	CloseHandle(hJobObject);
	return 0;
}

int Jobbg(int argc, LPTSTR argv[], LPTSTR command)
{
	DWORD fCreate;
	LONG jobNumber;
	BOOL flags[2];

	STARTUPINFO startup;
	PROCESS_INFORMATION processInfo;
	LPTSTR targv = SkipArg(command);
	GetStartupInfo(&startup);

	Options(argc, argv, _T("cd"), &flags[0], &flags[1], NULL);

	if (argv[1][0] == '-')
		targv = SkipArg(targv);

	fCreate = flags[0] ? CREATE_NEW_CONSOLE : flags[1] ? DETACHED_PROCESS : 0;
	CreateProcess(NULL, targv, NULL, NULL, TRUE, fCreate | CREATE_SUSPENDED | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &startup, &processInfo);
	AssignProcessToJobObject(hJobObject, processInfo.hProcess);
	jobNumber = GetJobNumber(&processInfo, targv);
	if (jobNumber > 0)
		ResumeThread((HANDLE)processInfo.dwThreadId);
	else {
		TerminateProcess((HANDLE)processInfo.dwProcessId, 3);
		CloseHandle((HANDLE)processInfo.dwThreadId);
		CloseHandle((HANDLE)processInfo.dwProcessId);
		return 5;
	}
	CloseHandle((HANDLE)processInfo.dwThreadId);
	CloseHandle((HANDLE)processInfo.dwProcessId);
	_tprintf(_T("[%d] %d \n"), jobNumber, processInfo.dwProcessId);
	return 0;
}

int Jobs(int argc, LPTSTR argv[])
{
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION basicInfo;
	DisplayJobs();

	QueryInformationJobObject(hJobObject, JobObjectBasicAccountingInformation, &basicInfo
	,sizeof(JOBOBJECT_BASIC_ACCOUNTING_INFORMATION),NULL);
	_tprintf(_T("Total Process : %d , Active: %d, Terminated: %d \n"), basicInfo.TotalProcesses, basicInfo.TotalUserTime.QuadPart / MILLION,
		(basicInfo.TotalUserTime.QuadPart / MILLION) / 1000);
	return 0;
}