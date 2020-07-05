#include <Windows.h>
#include <tchar.h>
#include <stdio.h>


static int Create(int,LPTSTR*,LPTSTR);
static int Delete(int,LPTSTR*,LPTSTR);
static int Start(int,LPTSTR*,LPTSTR);
static int Control(int,LPTSTR*,LPTSTR);
 VOID  GetArgs(LPCTSTR command, int* pargc, LPTSTR argstr[]);

static SC_HANDLE hScm;
static BOOL debug;

#define MAX_ARG 4
#define MAX_COMMAND_LINE 100

int _tmain(int argc,LPTSTR* argv)
{
	BOOL exitFlag = FALSE;
	TCHAR command[MAX_COMMAND_LINE + 1] ,*pc;
	DWORD i;
	int locArgc;

	TCHAR argstr[MAX_ARG][MAX_COMMAND_LINE];
	LPTSTR pArgs[MAX_ARG];

	debug = argc > 1;

	for(i = 0 ; i < MAX_ARG; i++) pArgs[i] = argstr[i];//local argv as point to string
	hScm = OpenSCManager(NULL,SERVICES_ACTIVE_DATABASE,SC_MANAGER_ALL_ACCESS);

	_tprintf(_T("\nNEW WINDOWS MANAGERMENT"));

	while(!exitFlag){
		_tprintf(_T("\nSM$"));
		_fgetts(command,MAX_COMMAND_LINE,stdin);
		pc = _tcschr(command,_T('\n'));
		*pc = '\0';
		if(debug) _tprintf(_T("%s\n"),command);
		GetArgs(command,&locArgc,pArgs);
		CharLower(argstr[0]);
		if(debug) _tprintf(_T("\n%s  %s  %s %s"),argstr[0],argstr[1],argstr[2],argstr[3]);

		if(_tcscmp(argstr[0],_T("create")) == 0){
			Create(locArgc,pArgs,command);
		}else if(_tcscmp(argstr[0],_T("delete")) == 0){
			Delete(locArgc,pArgs,command);
		}else if(_tcscmp(argstr[0],_T("start")) == 0){
			Start(locArgc,pArgs,command);
		}else if(_tcscmp(argstr[0],_T("control")) == 0){
			Control(locArgc,pArgs,command);
		}else if(_tcscmp(argstr[0],_T("quit")) == 0){
			exitFlag = TRUE;
		}else _tprintf(_T("\nCommand not recognized."));
	}
	CloseServiceHandle(hScm);
	return 0;
}

int Create(int argc,LPTSTR argv[],LPTSTR command)
{
	SC_HANDLE hSc;
	TCHAR executable[MAX_PATH+1];
	TCHAR qutoExecutable[MAX_PATH+ 3] = _T("\"");

	GetFullPathName(argv[3],MAX_PATH+1,executable,NULL);
	_tcscat(qutoExecutable,_T("\""));
	if(debug) _tprintf(_T("\nService Full Path Name: %s "),executable);

	hSc = CreateService(hScm,argv[1],argv[2],
		SERVICE_ALL_ACCESS,SERVICE_WIN32_OWN_PROCESS,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,
		qutoExecutable,
		NULL,NULL,NULL,NULL,NULL);
	return 0;
}

int Delete(int argc, LPTSTR argv[],LPTSTR command)
{
	SC_HANDLE hSc;
	if(debug) _tprintf(_T("\nAbout to Delete service : %s "),argv[1]);
	hSc = OpenService(hScm,argv[1],DELETE);
	DeleteService(hSc);
	CloseServiceHandle(hSc);
	return 0;
}


int Start(int argc,LPTSTR argv[] ,LPTSTR)
{
	SC_HANDLE hSc;
	TCHAR workingDir[MAX_PATH+ 1];
	LPCTSTR argvStart[] = {argv[1],workingDir};
	GetCurrentDirectory(MAX_PATH+ 1,workingDir);
	hSc = OpenService(hScm,argv[1],SERVICE_ALL_ACCESS);
	StartService(hSc,2,argvStart);
	CloseServiceHandle(hSc);
	return 0;
}

static LPCTSTR commandList[] = {_T("stop"),_T("pause"),_T("resume"),_T("interrogate"),_T("user")};
static DWORD controlsAccepted[] = {SERVICE_CONTROL_STOP,SERVICE_CONTROL_PAUSE,SERVICE_CONTROL_CONTINUE,SERVICE_CONTROL_INTERROGATE,128};

int Control(int argc,LPTSTR argv[],LPTSTR)
{
	SC_HANDLE hSc;
	SERVICE_STATUS sStatus;
	DWORD i,dwControl;
	BOOL found = FALSE;

	if(debug) _tprintf(_T("\nControl service : %s "),argv[1]);
	for (i = 0 ; i < sizeof(controlsAccepted)/sizeof(DWORD)&& !found; i++)
		found = (_tcscmp(commandList[i],argv[2]) == 0);
	if(!found){
		_tprintf(_T("\nIllegal control command : %s "),argv[1]);
		return 1;
	}

	dwControl = controlsAccepted[i-1];
	if(dwControl == 128) dwControl = _ttol(argv[3]);
	if(debug) _tprintf(_T("\ndwControl = %d"),dwControl);
	hSc = OpenService(hScm,argv[1],SERVICE_INTERROGATE|SERVICE_PAUSE_CONTINUE|
		SERVICE_STOP| SERVICE_USER_DEFINED_CONTROL|
		SERVICE_QUERY_STATUS);
	ControlService(hSc,dwControl,&sStatus);

	if(dwControl == SERVICE_CONTROL_INTERROGATE){
		QueryServiceStatus(hSc,&sStatus);
		_tprintf(_T("\nStatus from QueryServiceStatus"));
		_tprintf(_T("\nService Status"));
		_tprintf(_T("\nServiceType : %d"),sStatus.dwServiceType);
		_tprintf(_T("\ndwCurrentState : %d"),sStatus.dwCurrentState);
		_tprintf(_T("\ndwControlsAccepted : %d"),sStatus.dwControlsAccepted);
		_tprintf(_T("\ndwWin32ExitCode : %d"),sStatus.dwWin32ExitCode);
		_tprintf(_T("\ndwCheckPoint : %d"),sStatus.dwCheckPoint);
		_tprintf(_T("\ndwWaitHint : %d"),sStatus.dwWaitHint);
		_tprintf(_T("\ndwServiceSpecificExitCode : %d"),sStatus.dwServiceSpecificExitCode);
	}
	if(hSc != NULL) CloseServiceHandle(hSc);
	return 0;
}