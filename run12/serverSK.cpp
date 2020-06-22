#include <Windows.h>
#include <tchar.h>

#define MAX_CLIENTS  100

struct sockaddr_in servsAddr;
struct sockaddr_in connectsAddr;
WSADATA WSStartData;

enum SERVER_THREAD_STATE{SERVER_SLOT_FREE,SERVER_THREAD_STOPPED,SERVER_THREAD_RUNNING,SERVER_SLOT_INVALID};
typedef struct SERVER_ARG_TAG{
	CRITICAL_SECTION threadCs;
	SOCKET sock;
	DWORD number;
	enum SERVER_THREAD_STATE thState;
	HANDLE hSvrThread;
	HINSTANCE hDll;/*Shared library handle*/
}SERVER_ARG;

static BOOL ReceiveRequestMessage(REQUEST* pRequest, SOCKET);
static BOOL SendResponseMessage(RESPONSE* pResponse, SOCKET);
static DWORD WINAPI Server(PVOID);
static DWORD WINAPI AcceptThread(PVOID);
static BOOL WINAPI Handler(DWORD);

volatile static int shutFlag = 0;
static SOCKET SrvSock = INVALID_SOCKET, connectSock = INVALID_SOCKET;

int main(int argc, LPCTSTR argv)
{
	DWORD iThread, tStatus;
	SERVER_ARG sArgs[MAX_CLIENTS];
	HANDLE hAcceptThread = NULL;
	HINSTANCE hDll = NULL;

	SetConsoleCtrlHandler(Handler, TRUE);
	WSAStartup(MAKEWORD(2, 2), &WSStartData);

	if (argc > 1) hDll = LoadLibrary(argv[1]);
	for (iThread = 0; iThread < MAX_CLIENTS; iThread++)
	{
		InitializeCriticalSection(&sArgs[iThread].threadCs);
		sArgs[iThread].number = iThread;
		sArgs[iThread].thState = SERVER_SLOT_FREE;
		sArgs[iThread].sock = 0;
		sArgs[iThread].hDll = hDll;
		sArgs[iThread].hSvrThread = NULL;

	}

	SrvSock = socket(PF_INET, SOCK_STREAM, 0);
	servsAddr.sin_family = AF_INET;
	servsAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servsAddr.sin_port = htons(SERVER_PORT);
	bind(SrvSock, (struct sockaddr*)&servsAddr, sizeof(servsAddr));
	listen(SrvSock, MAX_CLIENTS);

	while (!shutFlag) {
		iThread = 0;
		while (!shutFlag) {
			EnterCriticalSection(&sArgs[iThread].threadCs);
			__try {
				&sArgs[iThread].thState == SERVER_SLOT_FREE || shutFlag) break;
				if (sArgs[iThread].thState == SERVER_THREAD_STOPPED) {
					WaitForSingleObject(sArgs[iThread].hSvrThread, INFINITE);
					CloseHandle(sArgs[iThread].hSvrThread);
					sArgs[iThread].hSvrThread = NULL;
					sArgs[iThread].thState = SERVER_SLOT_FREE;
				}
				if (sArgs[iThread].thState == SERVER_SLOT_FREE || shutFlag) break;
			}
			__finally {
				LeaveCriticalSection(&sArgs[iThread].threadCs);
			}

			iThread = (iThread++) & MAX_CLIENTS;
			if (iThread == 0) Sleep(50);

		}

		if (shutFlag) break;

		hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, &sArgs[iThread], 0, NULL);
		while (!shutFlag) {
			tStatus = WaitForSingleObject(hAcceptThread, CS_TIMEOUT);
			if (tStatus == WAIT_OBJECT_0) {
				if (!shutFlag) {
					CloseHandle(hAcceptThread);
					hAcceptThread = NULL;
				}
				break;
			}
		}
	}

	_tprintf(_T("shutdown in process . wait for server threds\n"));

	while (TRUE) {
		int nRunningThreads = 0;
		for (iThread = 0; iThread < MAX_CLIENTS; iThread++) {
			EnterCriticalSection(&sArgs[iThread].threadCs);
			__try {
				if (sArgs[iThread].thState == SERVER_THREAD_RUNNING || sArgs[iThread].thState == SERVER_THREAD_STOPPED) {
					if (WaitForSingleObject(sArgs[iThread].hSvrThread, 10000) == WAIT_OBJECT_0) {
						CloseHandle(sArgs[iThread].hSvrThread);
						sArgs[iThread].hSvrThread = NULL;
						sArgs[iThread].thState = SERVER_SLOT_INVALID;

					}
					else {
						if (WaitForSingleObject(sArgs[iThread].hSvrThread, 10000) == WAIT_TIMEOUT)
							nRunningThreads++;
						else
							_tprintf(_T("Error for Waiting ;slot %d \n"), iThread);
					}
				}
			}
			__finally {
				LeaveCriticalSection(&sArgs[iThread].threadCs);
			}
		}
		if (nRunningThreads == 0) break;
	}
	if (hDll != NULL) FreeLibrary(hDll);
	shutdown(SrvSock, SD_BOTH);
	closesocket(SrvSock);
	WSACleanup();
	if (hAcceptThread != NULL)
		WaitForSingleObject(hAcceptThread, INFINITE);

	return 0;
}

static DWORD WINAPI AcceptThread(PVOID pArg)
{

}