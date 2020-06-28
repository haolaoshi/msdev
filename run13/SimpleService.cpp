#include <Windows.h>
#include <tchar.h>
#include <time.h>
#include <stdio.h>


#define UPDATE_TIME 1000

VOID LogEvent(LPCTSTR,WORD),LogClose();
BOOL LogInit(LPTSTR);
void WINAPI ServiceMain(DWORD argc,LPTSTR argv[]);
VOID WINAPI ServiceCtrlHandler(DWORD);
void UpdateStatus(int,int);
int ServiceSpecific(int,LPTSTR*);

static BOOL shutDown = FALSE,pauseFlag = FALSE;
static SERVICE_STATUS hServStatus;
static SERVICE_STATUS_HANDLE hStat;
static LPTSTR serviceName = _T("Simple Service");
static LPTSTR logFileName = _T(".\\LogFiles\\SimpleServiceLog.txt");
static BOOL consoleApp = FALSE, isService;
DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);
//DWORD Options(int,LPTSTR*,LPCTSTR,BOOL*,int);

VOID _tmain(int argc,LPTSTR argv[])
{

	SERVICE_TABLE_ENTRY DispatchTable[] = 
	{
		{serviceName,	ServiceMain},
		{NULL,			NULL	  }
	};

	Options(argc,argv,_T("c"),&consoleApp,NULL);

	isService = !consoleApp;

	if(!LogInit(logFileName)) return ;

	if(isService){
		LogEvent(_T("Starting Dispatcher"),EVENTLOG_SUCCESS);
		StartServiceCtrlDispatcher(DispatchTable);
	}else{
		LogEvent(_T("Starting Applicationg "),EVENTLOG_SUCCESS);
		ServiceSpecific(argc,argv);
	}

	LogClose();
	return ;
}

void WINAPI ServiceMain(DWORD argc,LPTSTR argv[])
{
	LogEvent(_T("Entering ServiceMain"),EVENTLOG_SUCCESS);
	hServStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	hServStatus.dwCurrentState = SERVICE_START_PENDING;
	hServStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
	hServStatus.dwWin32ExitCode = NO_ERROR;
	hServStatus.dwServiceSpecificExitCode = 0;
	hServStatus.dwCheckPoint = 0;
	hServStatus.dwWaitHint = 2 * UPDATE_TIME;

	hStat = RegisterServiceCtrlHandler(serviceName,ServiceCtrlHandler);

	if(hStat == 0){
		LogEvent(_T("cannot register handler "),EVENTLOG_ERROR_TYPE);
		hServStatus.dwCurrentState = SERVICE_STOPPED;
		hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		hServStatus.dwServiceSpecificExitCode = 1;
		UpdateStatus(SERVICE_STOPPED,-1);
		return;
	}

	LogEvent(_T("Control handler registered"),EVENTLOG_SUCCESS);
	SetServiceStatus(hStat,&hServStatus);
	LogEvent(_T("status SERVICE_START_PENDING"),EVENTLOG_SUCCESS);
	ServiceSpecific(argc,argv);
	LogEvent(_T("Service threads shut down "),EVENTLOG_SUCCESS);
	LogEvent(_T("Set SERVICE_STOPPED status"),EVENTLOG_SUCCESS);
	UpdateStatus(SERVICE_STOPPED,0);
	LogEvent(_T("status set to service stopped "),EVENTLOG_SUCCESS);
	return ;
}

int ServiceSpecific(int argc,LPTSTR argv[])
{

	UpdateStatus(-1,-1);
	UpdateStatus(SERVICE_RUNNING,-1);
	LogEvent(_T("Status update. service running"),EVENTLOG_SUCCESS);

	LogEvent(_T("starting main service loop"),EVENTLOG_SUCCESS);
	while(!shutDown){
		Sleep(UPDATE_TIME);
		UpdateStatus(-1,-1);
		LogEvent(_T("Status update.No change"),EVENTLOG_SUCCESS);
	}
	LogEvent(_T("Server process has shut down"),EVENTLOG_SUCCESS);
	return 0;
}

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch(dwControl){
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		shutDown = TRUE;
		UpdateStatus(SERVICE_STOP_PENDING,-1);
		break;
	case SERVICE_CONTROL_PAUSE:
		pauseFlag = TRUE;
		break;
	case SERVICE_CONTROL_CONTINUE:
		pauseFlag = FALSE;
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		if(dwControl > 127 && dwControl < 256) 
			break;
	}
	UpdateStatus(-1,-1);
	return ;
}

void UpdateStatus(int NewStatus,int check)
{
	if(check < 0) hServStatus.dwCheckPoint++;
	else hServStatus.dwCheckPoint = check;

	if(NewStatus >= 0) hServStatus.dwCurrentState = NewStatus;
	if(isService){
		if(!SetServiceStatus(hStat,&hServStatus)){
			LogEvent(_T("Cannot set status"),EVENTLOG_ERROR_TYPE);
			hServStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
			hServStatus.dwCurrentState = SERVICE_STOPPED;
			hServStatus.dwServiceSpecificExitCode = 2;
			UpdateStatus(SERVICE_STOPPED,-1);
			return ;
		}else{
			LogEvent(_T("Service Stauts updated."),EVENTLOG_SUCCESS);
		}
	}else{
		LogEvent(_T("standalone status updaed."),EVENTLOG_SUCCESS);
	}
	return ;
}

static FILE * logFp = NULL;

VOID LogEvent(LPCTSTR UserMessage,WORD type)
{
	TCHAR cTimeString[30] = _T("");
	time_t currentTime = time(NULL);
	_tcsncat(cTimeString,_tctime(&currentTime),30);
	cTimeString[_tcslen(cTimeString) - 2 ] = _T('\0');
	_ftprintf(logFp,_T("%s. "),cTimeString);
	if(type == EVENTLOG_SUCCESS || type == EVENTLOG_INFORMATION_TYPE )
		_ftprintf(logFp,_T("%s"),_T("Information. "));
	else if (type == EVENTLOG_ERROR_TYPE)
		_ftprintf(logFp,_T("%s"),_T("Error.       "));
	else if (type == EVENTLOG_WARNING_TYPE )
		_ftprintf(logFp,_T("%s"),_T("Warning.	  "));
	else _ftprintf(logFp,_T("%s"),_T("Unk`nown.	  "));

	_ftprintf(logFp,_T("%s\n"),UserMessage);
	fflush(logFp);
	return ;
}


BOOL LogInit(LPTSTR name)
{
	logFp = _tfopen(name,_T("a+"));
	if(logFp != NULL ) LogEvent(_T("Initialized Logging"),EVENTLOG_SUCCESS);
	return (logFp != NULL);
}

VOID LogClose()
{
	LogEvent(_T("Cloing Log"),EVENTLOG_SUCCESS);
	return ;
}
