#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
/*Note the WINAPI is for user functions passed as argu to Windows functions to assure proper calling conventios.*/
static BOOL WINAPI ControlCHandler(DWORD cntrlEvent);
static BOOL exitFlag = FALSE;

int _tmain(int argc,LPTSTR argv[])
{
	/* Add an event handler */
	SetConsoleCtrlHandler(ControlCHandler,TRUE);
	while(!exitFlag){
		Sleep(5000);
		Beep(1000/*Frequency.*/,250/*duration.*/);
	}
	_tprintf(_T("Stopping the program as requested.\n"));
	return 0;
}

BOOL WINAPI ControlCHandler(DWORD cntrlEvent)
{
	exitFlag = TRUE;
	switch(cntrlEvent){
	case CTRL_C_EVENT:
		_tprintf(_T("Ctrl-C Leaving in <= 5 seconds.\n"));
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving hadler in 1 second or less\n"));
		return TRUE;
	case CTRL_CLOSE_EVENT:
		_tprintf(_T("Close event.Leaving in <= 5 second.\n"));
		exitFlag = TRUE;
		Sleep(4000);
		return TRUE;
	default:
		_tprintf(_T("Event : %d. Leaving in 5 seconds.\n"),cntrlEvent);
		exitFlag = TRUE;
		Sleep(4000);
		_tprintf(_T("Leaving handler in <=1 second.\n"));
		return TRUE;

	}
}