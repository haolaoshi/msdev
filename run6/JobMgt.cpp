#include <Windows.h>
#include <tchar.h>
#include "JobMgt.h"



LONG GetJobNumber(PROCESS_INFORMATION* processInfo, LPCTSTR command)
{
	HANDLE hJobdata, hProcess;
	JM_JOB jobRecord;
	DWORD  jobNumber = 0, nXfer, exitCode, fileSizeLow, fileSizeHigh;
	TCHAR jobMgtFileName[MAX_PATH];
	OVERLAPPED reginStart;
	if (!GetJobMgtFileName(jobMgtFileName)) return -1;

	hJobdata = CreateFile(jobMgtFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hJobdata == INVALID_HANDLE_VALUE) return -1;

	reginStart.Offset = 0;
	reginStart.OffsetHigh = 0;
	reginStart.hEvent = (HANDLE)0;

	fileSizeLow = GetFileSize(hJobdata, &fileSizeHigh);
	LockFileEx(hJobdata, LOCKFILE_EXCLUSIVE_LOCK, 0, fileSizeLow, 0, &reginStart);

	__try {
		while (ReadFile(hJobdata, &jobRecord, SJM_JOB, &nXfer, NULL) && (nXfer > 0)) {
			if (jobRecord.ProcessId == 0) break;
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, jobRecord.ProcessId);
			if (hProcess == NULL) break;
			if (GetExitCodeProcess(hProcess, &exitCode) && (exitCode != STILL_ACTIVE)) break;
			jobNumber++;
		}
		if (nXfer != 0)
			SetFilePointer(hJobdata, -(LONG)SJM_JOB, NULL, FILE_CURRENT);
		jobRecord.ProcessId = processInfo->dwProcessId;
		//_tcsnccpy(hJobdata, &jobRecord, SJM_JOB, &nXfer, NULL);
		_tcsnccpy_s(jobRecord.CommandLine, command, MAX_PATH);
		WriteFile(hJobdata, &jobRecord, SJM_JOB, &nXfer, NULL);
	}
	__finally {
		UnlockFileEx(hJobdata, 0, fileSizeLow + SJM_JOB, 0, &reginStart);
		CloseHandle(hJobdata);
	}
	return jobNumber + 1;
}

BOOL DisplayJobs(void)
{
	HANDLE hJobData, hProcess;
	JM_JOB jobRecord;
	DWORD jobNumber = 0, nXfer, exitCode, fileSizeLow, fileSizeHigh;
	TCHAR jobMgtFileName[MAX_PATH];
	OVERLAPPED regionStart;
	GetJobMgtFileName(jobMgtFileName);
	hJobData = CreateFile(jobMgtFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	regionStart.Offset = 0;
	regionStart.OffsetHigh = 0;
	regionStart.hEvent = (HANDLE)0;

	fileSizeLow = GetFileSize(hJobData, &fileSizeHigh);
	LockFileEx(hJobData, LOCKFILE_EXCLUSIVE_LOCK, 0, fileSizeLow, fileSizeHigh, &regionStart);
	__try {
		while (ReadFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL) && (nXfer > 0)) {
			jobNumber++;
			if (jobRecord.ProcessId == 0)
				continue;
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, jobRecord.ProcessId);
			if (hProcess != NULL)
				GetExitCodeProcess(hProcess, &exitCode);
			_tprintf(_T(" [%d] "), jobNumber);
			if (hProcess == NULL)
				_tprintf(_T(" Done. "));
			else if (exitCode != STILL_ACTIVE)
				_tprintf(_T(" +Done "));
			else _tprintf(_T("  "));
			_tprintf(_T(" %s\n"), jobRecord.CommandLine);
			if (hProcess == NULL) {
				SetFilePointer(hJobData, -(LONG)nXfer, NULL, FILE_CURRENT);
				jobRecord.ProcessId = 0;
				WriteFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL);

			}
		}
	}

	__finally {
		UnlockFileEx(hJobData, 0, fileSizeLow, fileSizeHigh, &regionStart);
		CloseHandle(hJobData);
	}

	return TRUE;
}

DWORD FindProcessId(DWORD jobNumber)
{
	HANDLE hJobData;
	JM_JOB jobRecord;
	DWORD nXfer;
	TCHAR jobMgtFileName[MAX_PATH];
	OVERLAPPED regionStart;

	GetJobMgtFileName(jobMgtFileName);
	hJobData = CreateFile(jobMgtFileName, GENERIC_READ , FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hJobData == INVALID_HANDLE_VALUE) return 0;
	SetFilePointer(hJobData, SJM_JOB * (jobNumber - 1), NULL, FILE_BEGIN);
	regionStart.Offset = SJM_JOB * (jobNumber - 1);
	regionStart.OffsetHigh = 0;
	regionStart.hEvent = (HANDLE)0;
	LockFileEx(hJobData, 0, 0, SJM_JOB, 0, &regionStart);
	ReadFile(hJobData, &jobRecord, SJM_JOB, &nXfer, NULL);
	UnlockFileEx(hJobData, 0, SJM_JOB, 0, &regionStart);
	CloseHandle(hJobData);
	return jobRecord.ProcessId;
}

BOOL GetJobMgtFileName(LPTSTR JobMgtFileName)
{
	TCHAR UserName[MAX_PATH], TempPath[MAX_PATH];
	DWORD UNSize = MAX_PATH, TPSize = MAX_PATH;
	if (!GetUserName(UserName, &UNSize)) return FALSE;
	if (GetTempPath(TPSize, TempPath) > TPSize)
		return FALSE;
	_stprintf(JobMgtFileName, _T("%s%s%s"), TempPath, UserName, _T(".JobMgt"));
	return TRUE;
}