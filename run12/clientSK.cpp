#include <Windows.h>
#include <tchar.h>

static BOOL SendRequestMessage(REQUEST*, SOCKET);
static BOOL ReceiveResponseMessage(REPONSE*, SOCKET);

struct sockadd_in clientAddr;

int main(int argc, LPSTR argv[])
{
	SOCKET clientSock = INVALID_SOCKET;
	REQUEST request;
	RESPONESE response;
	WSADATA WSStartData;
	BOOL quit = FALSE;
	DWORD conVal;

	WSAStartup(MAKEWORD(2, 2), &WSStartData);
	clientSock = socket(AF_INET, SOCK_STREAM, 0);
	memset(&clientAddr, 0, sizeof(clientAddr));
	clientAddr.sin_family = AF_INET;
	if (argc >= 2)
		clientAddr.sin_addr.s_addr = inet_addr(argv[1]);
	else
	{
		clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}

	return 0;
}