#include <windows.h>
#include <tchar.h>
#include <process.h>


#include <fstream>



typedef struct{
	HANDLE hNamedPipe;
	DWORD threadNumber;
	TCHAR tempFileName[MAX_PATH];
}THREAD_ARG;

typedef THREAD_ARG *LPTHREAD_ARG;
#define MAX_CLIENTS  200

volatile static int shutdown = 0;
static DWORD WINAPI Server(LPTHREAD_ARG);
static DWORD WINAPI Connect(LPTHREAD_ARG);
static DWORD WINAPI ServerBroadcast(LPLONG);
static BOOL WINAPI Handler(DWORD);
static THREAD_ARG threadArgs[MAX_CLIENTS];
BOOL shutdownFlag = FALSE;

int _tmain(int argc, LPTSTR argv[])
{
	HANDLE hNp,hMonitor,hSrvrThread[MAX_CLIENTS];
	DWORD iNp,monitorId,threadId;
	LPSECURITY_ATTRIBUTES pNPSA = NULL;
	SetConsoleCtrlHandler (Handler ,TRUE);

	hMonitor = (HANDLE)_beginthreadex(NULL,0,ServerBroadcast,NULL,0,&monitorId);
	for(iNp = 0; iNp < MAX_CLIENTS; iNp++){
		hNp = CreateNamedPipe(SERVER_PIPE,PIPE_ACCESS_DUPLEX,
			PIPE_READMODE_MESSAGE|PIPE_TYPE_MESSAGE|PIPE_WAIT,
			MAX_CLIENTS,0,0,INFINITE,pNPSA);
		threadArgs[iNp].hNamedPipe = hNp;
		threadArgs[iNp].threadNumber = iNp;
		GetTempFileName(_T("."),_T("CLP"),0,threadArgs[iNp],0,&threadId);

	}
	WaitForMultipleObjects(MAX_CLIENTS,hSrvrThread,TRUE,INFINITE);
	_tprintf(_T("All server thread workers have shut down .\n"));
	WaitForSingleObject(hMonitor,INFINITE);
	_tprintf(_T("Monitor Thread have shut down.\n"));

	CloseHandle(hMonitor);
	for(iNp = 0; iNp < MAX_CLIENTS;iNp++){
		CloseHandle(hSrvrThread[iNp]);
		DeleteFile(threadArgs[iNp].tempFileName);
	}
	_tprintf(_T("Server process will exit.\n"));
	return 0;
}

static DWORD WINAPI Server(LPTHREAD_ARG pTharg)
{
	
	HANDLE hNamedPipe,hTemFile = INVALID_HANDLE_VALUE,
	hConTh = NULL,hClient;

	DWORD nXfer,conThStatus,clientProcessId;
	STARTUPINFO startInfoCh;
	SECURITY_ATTRIBUTES temSA = 
	{sizeof(SECURITY_ATTRIBUTES),NULL,TRUE};
	PROCESS_INFORMATION procInfo;
	FILE *fp;
	REQUEST request;
	RESPONSE response;

	GetStartupInfo(&startInfoCh);
	hNamedPipe = pTharg->hNamedPipe;
	hTemFile = CreateFile(pTharg->tempFileName,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,&temSA,
		CREATE_ALWAYS,FILE_ATTRIBUTE_TEMPORARY,NULL);
	while(!shutdown){

		hConTh = (HANDLE)_beginthreadex(NULL,0,Connect,pTharg,0,NULL);
		while(!shutdown && WaitForSingleObject(hConTh,CS_TIMEOUT) == WAIT_TIMEOUT){
			//EMPTY LOOP BODY
		}
		if(shutdown) _tprintf(_T("thread %d received shutdown\n"),pTharg->threadNumber);
		if(shutdown) continue;
		CloseHandle(hConTh);
		hConTh = NULL;

		GetNamedPipeClientProcessId(pTharg->hNamedPipe, &clientProcessId);
		_tprintf(_T("connect to client process id : %d\n"),clientProcessId);

		while(!shutdown && ReadFile(hNamedPipe,&request,RQ_SIZE,&nXfer,NULL)){
			_tprintf(_T("command from client thread : %d. %s\n"),clientProcessId,request.record);
			shutdownFlag = shutdownFlag || (_tcscmp(request.record , shutRequest) == 0 );
			if(shutdownFlag ) continue;

			startInfoCh.hStdOutput = hTemFile;
			startInfoCh.hStdError = hTemFile;
			startInfoCh.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
			startInfoCh.dwFlags = STARTF_USESTDHANDLES;

			CreateProcess(NULL,request.record,NULL,NULL,TRUE,0,NULL,NULL,&startInfoCh,&procInfo);
			CloseHandle(procInfo.hThread);
			WaitForSingleObject(procInfo.hProcess,INFINITE);
			CloseHandle(procInfo.hProcess);

			fp = _tfopen(pTharg->tempFileName,_T("r"));
			while(_fgetts(response.record,MAX_RQRS_LEN,fp) != NULL){

			}

			_tcscpy(response.record,_T(""));
			response.rsLen = 0;
			WriteFile(hNamedPipe,&response,sizeof(response.rsLen),&nXfer,NULL);
			FlushFileBuffers(hNamedPipe);
			fclose(fp);
			SetFilePointer(hTemFile,0,NULL,FILE_BEGIN);
			SetEndOfFile(hTemFile);

		}
		FlushFileBuffers(hNamedPipe);
		DisconnectNamedPipe(hNamedPipe);
	}

	if(hConTh != NULL){
		GetExitCodeThread(hConTh,&conThStatus);
		if(conThStatus == STILL_ACTIVE){
			hClient = CreateFile(SERVER_PIPE,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			if(hClient != INVALID_HANDLE_VALUE) CloseHandle(hClient);
			WaitForSingleObject(hConTh,INFINITE);
		}

	}

	_tprintf(_T("thread %d shutting down.\n"),pTharg->threadNumber);
	CloseHandle(hTemFile);
	hTemFile = INVALID_HANDLE_VALUE;
	DeleteFile(pTharg->tempFileName);
	_tprintf(_T("exiting server thread number %d \n"),pTharg->threadNumber);
	return 0;
}

static DWORD WINAPI Connect(LPTHREAD_ARG pTharg)
{
	ConnectNamedPipe(pTharg->hNamedPipe,NULL);
	return 0;
}

BOOL WINAPI Handler(DWORD CtrlEvent)
{

	shutdownFlag = TRUE;
	return TRUE;
}