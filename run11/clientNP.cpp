/*
Named pipe connection oriented client


C/S SYSTEM.CLIENT


*/
#include <Windows.h>
#include <tchar.h>

#include "ClientServer.h"

int _tmain(int argc,LPTSTR argv[])
{
	HANDLE hnamedPipe = INVALID_HANDLE_VALUE;
	TCHAR quitMsg[] = _T("$Quit");
	TCHAR serverPipeName[MAX_PATH];
	REQUEST request;
	REPONSE respone ;
	DWORD nRead,nWrite,npMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	LocateServer(serverPipeName,MAX_PATH);

	while(INVALID_HANDLE_VALUE == hnamedPipe){
		WaitNamedPipe(serverPipeName,NMPWAIT_WAIT_FOREVER);
		hnamedPipe = CreateFile(serverPipeName,GENERIC_READ | GENERIC_WRITE,0,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	}

	SetNamedPipeHandleState(hnamedPipe,&npMode,NULL,NULL);
	request.command = 0;
	request.rqLen = RQ_SIZE;

	while(ConsolePrompt(promptMsg,request.record,MAX_RQRS_LEN,TRUE) && (_tcscmp(request.record,quitMsg) != 0)){
		WriteFile(hnamedPipe,&request,RQ_SIZE,&nWrite,NULL);
		while(ReadFile(hnamedPipe,&response,RS_SIZE,&nRead,NULL))
		{
			if(response.rsLen <= 1) break;
			_tprintf(_T("%s"),response.record);
		}
	}
	_tprintf(_T("Quit command received, Disconnect !"));
	CloseHandle(hnamedPipe);
	return 0;

}