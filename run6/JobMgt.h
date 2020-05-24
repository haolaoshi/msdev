#pragma once
 

#include <Windows.h>
#define JM_EXIT_CODE 0x1000
#define MAX_JOBS_ALLOWED	10000
typedef struct _JM_JOB
{
	DWORD ProcessId;
	TCHAR CommandLine [MAX_PATH];
} JM_JOB;

#define SJM_JOB sizeof (JM_JOB)

LONG GetJobNumber(PROCESS_INFORMATION*, LPCTSTR);
BOOL DisplayJobs(void);
DWORD FindProcessId(DWORD);
BOOL GetJobMgtFileName(LPTSTR);

int Jobbg(int argc, LPTSTR argv[], LPTSTR command);
int Jobs(int argc, LPTSTR argv[], LPTSTR command);
int Kill(int argc, LPTSTR argv[], LPTSTR command);
int Kill(int argc, LPTSTR argv[], LPTSTR command);


LPTSTR SkipArg(LPCTSTR targv);
void GetArgs(LPCTSTR command, int* pargc, LPTSTR argstr[]);