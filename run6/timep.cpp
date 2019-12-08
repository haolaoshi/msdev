#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

int _tmain(int argc,LPTSTR argv[])
{
	STARTUPINFO startup;
	PROCESS_INFORMATION procInfo;
	union 
	{
		LONGLONG li;
		FILETIME ft;
	}createTime,exitTime,elapsedTime;

	FILETIME kernelTime,userTime;
	SYSTEMTIME eltiSys,ketiSys,ustiSys,startSys;
	LPTSTR targv = SkipArg(GetCommandLine());
	HANDLE hProc;
	HANDLE hProc;
	GetStartupInfo(&startup);
	GetSystemTime(&startSys);

	CreateProcess(NULL,targv,NULL,NULL,TRUE,NORMAL_PRIORITY_CLASS,NULL,NULL,&startup,&procInfo);
	hProc = procInfo.hProcess;
	WaitForSingleObject(hProc,INFINITE);
	GetProcessTimes(hProc,&createTime.ft,&exitTime.ft,&kernelTime,&userTime);
	elapsedTime.li = exitTime.li - createTime.li;
	FileTimeToSystemTime(&elapsedTime.ft,&eltiSys);
	FileTimeToSystemTime(&kernelTime,&ketiSys);
	FileTimeToSystemTime(&userTime,&ustiSys);
	_tprintf(_T("Real Time : %02d:%02d:%02d:%03d\n"),eltiSys.wHour,eltiSys.wMinute,eltiSys.wSecond,eltiSys.wMilliseconds);
	_tprintf(_T("User Time : %02d:%02d:%02d:%03d\n"),ustiSys.wHour,ustiSys.wMinute,ustiSys.wSecond,ustiSys.wMilliseconds);
	_tprintf(_T("Sys Time  : %02d:%02d:%02d:%03d\n"),ketiSys.wHour,ketiSys.wMinute,ketiSys.wSecond,ketiSys.wMilliseconds);
	
	CloseHandle(procInfo.hThread);
	CloseHandle(procInfo.hProcess);
	CloseHandle(hProc);

	return 0;
}