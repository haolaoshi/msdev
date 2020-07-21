#include <Windows.h>
#include <tchar.h>

static BOOL WINAPI Handler(DWORD CntrlEvent);
static VOID APIENTRY Beeper(LPVOID,DWORD,DWORD);
volatile static LONG exitFlag = 0;

HANDLE hTimer;

int _tmain(int argc,LPTSTR argv[])
{
	DWORD count = 0, period;
	LARGE_INTEGER dueTime;
	if(argc >= 2) period = _ttoi(argv[1]) * 1000;

	SetConsoleCtrlHandler(Handler,TRUE);

	dueTime.QuadPart = -(LONGLONG) period * 10000;
	hTimer = CreateWaitableTimer(NULL,FALSE,NULL);//not mannual reset (notification) timer,but a synchronization timer
	SetWaitableTimer(hTimer,&dueTime,period,Beeper,&count,FALSE);
	while(!exitFlag){
		_tprintf(_T("Count = %d\n"),count);
		//enter an alertable wait state ,enabling the timer routing.
		//the timer handle is a synchronization obj. so u can also wait on it.
		SleepEx(INFINITE,TRUE);
		//WaitForSingleObjectEx(hTimer,INFINITE,TRUE);
		
	}
	_tprintf(_T("shut down , count = %d"),count);
	CancelWaitableTimer(hTimer);
	CloseHandle(hTimer);

	return 0;
}

static VOID APIENTRY Beeper(LPVOID lpCount,DWORD dwTimerLowValue,DWORD dwTimerHighValue)
{
	*(LPDWORD)lpCount = *(LPDWORD)lpCount+1;
	_tprintf(_T("About to perform beep number : %d\n"),*(LPDWORD)lpCount);
	Beep(1000,250);
	return;
}
BOOL WINAPI Handler(DWORD CntrlEvent)
{
	InterlockedIncrement(&exitFlag);
	_tprintf(_T("shutting down. \n"));
	return TRUE;
}
