/*
use completion port server
ʹ��IO��ɶ˿�;������һ��С�ķ������̳߳غ�һ������ص��ܵ������,ÿ�����������һ����ɼ�ֵ;
�ص��������ӵ���ɶ˿���,������ConnectNamedPipe.
�������̵߳ȴ���ͻ����Ӽ�����������������.
��⵽����,�����Ŀͻ�����,��õ�����,���ظ��ͻ�.
serverCP����Ʒ�ֹ�������߳���IO�ڼ�����������ڼ�����.
ÿ���ͻ��ܵ���Ҫ����һ��״̬;
ÿ���ܵ�һ��CP_KEY �ṹ��ά��;
connected
requestRead
coputed
responding
respondLast

*/
#include <Windows.h>
#include <tchar.h>

#include <process.h>

typedef struct{
}REQUEST;

typedef struct {
	HANDLE hComPort;
	DWORD threadNum;
}SERVER_THREAD_ARG;

enum CP_CLIENT_PIPE_STATE
{
	connected,requestRead,computed,responding,respondLast
};

typedef struct 
{
	HANDLE hComPort;
	HANDLE hNp;
	HANDLE hTempFile;
	FILE   *tFp;
	TCHAR tmpFileName[MAX_PATH];
	REQUEST request;
	DWORD nBytes;
	enum CP_CLIENT_PIPE_STATE npState;
	LPOVERLAPPED pOverLap;
}CP_KEY;

OVERLAPPED overLap;

volatile static int shutDown = 0;
static DWORD WINAPI Server(SERVER_THREAD_ARG);
static DWORD WINAPI ServerBroadcast(LPLONG);
static BOOL  WINAPI Handler(DWORD);
static DWORD WINAPI ComputeThread(LPVOID);
#define MAX_CLIENTS_CP 4
#define MAX_CLIENTS 4
#define MAX_SERVER_TH 4

static CP_KEY Key[MAX_CLIENTS_CP];

int _tmain(int argc, LPTSTR argv[])
{
	HANDLE hCompPort,hMonitor,hSrvrThread[MAX_CLIENTS];
	DWORD iNp,iTh;
	SECURITY_ATTRIBUTES temFileSA = {sizeof (SECURITY_ATTRIBUTES),NULL,TRUE};
	SERVER_THREAD_ARG ThArgs[MAX_SERVER_TH];
	OVERLAPPED ov = {0};
	
	//console ctrol handler to permit server shutdown 
	SetConsoleCtrlHandler(Handler,TRUE);
	//create a thread broadcast pipe name periodicatlly
	hMonitor = (HANDLE)_beginthreadex(NULL,0,ServerBroadcast,NULL,0,NULL);
	hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,MAX_SERVER_TH);
	//Create a ov named pipe for every potential client
	//add to the completion port
	//assume that the max num of clients far exceeds
	//the number of server thread
	for(iNp = 0; iNp< MAX_CLIENTS_CP; iNp++){
		memset(&Key[iNp],0,sizeof(CP_KEY));
		Key[iNp].hComPort = hCompPort;
		Key[iNp].hNp = CreateNamedPipe(SERVER_PIPE,PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,PIPE_READMODE_BYTE|PIPE_TYPE_MESSAGE,
			MAX_CLIENTS_CP,0,0,INFINITE,&temFileSA);
		GetTempFileName(_T("."),_T("CLP"),0,Key[iNp].tmpFileName);
		Key[iNp].pOverLap = &overLap;
		CreateIoCompletionPort(Key[iNp].hNp,hCompPort,(ULONG_PTR)&Key[iNp].hNp,0);
		ConnectNamedPipe(Key[iNp].hNp,&ov);
		Key[iNp].npState = connected;

	}

	for(iTh = 0 ; iTh<MAX_SERVER_TH; iTh++){
		ThArgs[iTh].hComPort = hCompPort;
		ThArgs[iTh].threadNum = iTh;
		hSrvrThread[iTh] = (HANDLE)_beginthreadex(NULL,0,Server,&ThArgs[iTh],0,NULL);
	}

	_tprintf(_T("ALL SERVER THREAD RUNNING.\n"));
	WaitForMultipleObjects(MAX_SERVER_TH,hSrvrThread,TRUE,INFINITE);
	WaitForSingleObject(hMonitor,INFINITE);
	_tprintf(_T("monitor and server threawds have shut down.\n"));
	CloseHandle(hMonitor);
	for(iTh= 0; iTh< MAX_SERVER_TH; iTh++){
		CloseHandle(hSrvrThread[iTh]);
	}
	CloseHandle(hCompPort);
	
	return 0;
}


typedef struct {
}RESPONSE;

static DWORD WINAPI Server(SERVER_THREAD_ARG pTharg)
{
	DWORD nXfer;
	BOOL disconnect = FALSE;
	CP_KEY *pKey;
	RESPONSE response;
	OVERLAPPED serverOV = {0},*pOV = NULL;

	return 0;
}

static DWORD WINAPI ComputeThread(PVOID pArg)
{
	return 0;
}

static DWORD WINAPI ServerBroadcast(LPLONG pNull)
{


	return 0;
}


BOOL WINAPI Handler(DWORD CtrlEvent)
{
	//same in ServerNP , but post a completion
}
