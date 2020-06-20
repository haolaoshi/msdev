#include <Windows.h>
#include <tchar.h>
/*
Example a socket message receive function 
Example a socket based client 


*/
static BOOL SendRequestMessage(REQUEST*, SOCKET);
static BOOL ReceiveResponseMessage(REPONSE*, SOCKET);
#define MAX_MSG_LEN 255
#define MAX_RQRS_LEN 255
struct sockadd_in clientAddr;

typedef struct {
	LONG32 msgLeng;
	BYTE record[MAX_MSG_LEN];
} MESSAGE;

DWORD ReceiveMessage(MESSAGE* pMsg, SOCKET sd)
{

	DWORD disconnect = 0;
	LONG32 nRemainRecv, nXfer;
	LPBYTE pBuffer;
	nRemainRecv = 4;

	pBuffer = (LPBYTE)pMsg;
	while (nRemainRecv > 0 && !disconnect) {
		nXfer = recv(sd, pBuffer, nRemainRecv, 0);
		disconnect = (nXfer == 0);
		nRemainRecv -= nXfer;
		pBuffer += nXfer;
	}
	nRemainRecv = pMsg->msgLeng;
	nRemainRecv = min(nRemainRecv, MAX_RQRS_LEN);
	while (nRemainRecv > 0 && !disconnect)
	{
		nXfer = recv(sd, pBuffer, nRemainRecv, 0);
		disconnect = (nXfer == 0);
		nRemainRecv -= nXfer;
		pBuffer += nXfer;
	}
	return disconnect;
}


struct sockaddr_in clientsAddr;
#define SERVER_PORT 8911

int main(int argc, LPSTR argv[])
{
	SOCKET clientSock = INVALID_SOCKET;
	REQUEST request;
	RESPONESE response;
	WSADATA WSStartData;
	BOOL quit = FALSE;
	DWORD conVal;

	WSAStartup(MAKEWORD(2, 2), &WSStartData);//(1)
	clientSock = socket(AF_INET, SOCK_STREAM, 0);//(2)
	memset(&clientAddr, 0, sizeof(clientAddr));
	clientsAddr.sin_family = AF_INET;
	{
		clientsAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}
	clientsAddr.sin_port = htons(SERVER_PORT);

	conVal = connect(clientSock, (struct sockaddr*)&clientsAddr, sizeof(clientsAddr));//(3)
	while(!quit) {
		_tprintf(_T("%s"), _T("\nEnter Command: "));
		fgets(request.record, MAX_RQRS_LEN, stdin);
		request.record[strlen(request.record) - 1] = '\0';
		if (strcmp(request.record, "$Quit") == 0) quit = TRUE;
		SendRequestMessage(&request, clientSock);
		if (!quit) ReceiveResponseMessage(&response, clientSock);

	}
	shutdown(clientSock, SD_BOTH);
	closesocket(clientSock);
	WSACleanup();
	_tprintf(_T("\n****Leaving client\n"));

	return 0;
}