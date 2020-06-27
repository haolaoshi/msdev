/*
带状态结构的线程安全DLL
*/


#include <Windows.h>
#include <tchar.h>

#define MAX_RQRS_LEN 10
#define	 MAX_MESSAGE_LEN 100
typedef struct MSG_STRUCT {
	char record[MAX_MESSAGE_LEN];
	LONG32 msg_len;
} MESSAGE;


typedef struct SOCKET_HANDLE_T {
	SOCKET sk;
	char staticBuff[MAX_RQRS_LEN];
	LONG32 staticBuffLen;
} SOCKET_HANDLE,*PSOCKET_HANDLE;

__declspec(dllexport) PVOID CreateCSSocketHandle(SOCKET s)
{
	PVOID p;
	PSOCKET_HANDLE ps;
	p = malloc(sizeof(SOCKET_HANDLE));
	if (p == NULL) return NULL;
	ps = (SOCKET_HANDLE)p;
	ps->sk = s;
	ps->staticBuffLen = 0;
	return p;
}

__declspec(dllexport) BOOL CloseCSSocketHandle(PSOCKET_HANDLE psh)
{
	if (psh == NULL) return FALSE;
	free(psh);
	return TRUE;
}

__declspec(dllexport) BOOL ReceiveCSMessage(MESSAGE* pMsg, PSOCKET_HANDLE psh)
{
	BOOL disconnect = FALSE;
	LONG32 nRemainRecv = 0, nXfer, k;
	LPSTR pBuffer, message;
	CHAR termpBuff[MAX_RQRS_LEN + 1];
	SOCKET sd;

	if (psh == NULL) return FALSE;
	sd = psh->sk;

	message = pMsg->record;

	for (k = 0; k < psh->staticBuffLen && psh->staticBuff[k] != '\0'; k++)
	{
		message[k] = psh->staticBuff[k];
	}
	if (k < psh->staticBuffLen) {
		message[k] = '\0';

		psh->staticBuffLen -= (k + 1);
		memcpy(psh->staticBuff, &(psh->staticBuff[k + 1]), psh->staticBuffLen);
		return TRUE;
	}

	nRemainRecv = sizeof(termpBuff) - 1 - psh->staticBuffLen;
	pBuffer = message + psh->staticBuffLen;
	psh->staticBuffLen = 0;

	while (nRemainRecv > 0 && !disconnect) {
		nXfer = recv(sd, termpBuff, nRemainRecv, 0);
		if (nXfer <= 0) {
			disconnect = TRUE;
			continue;
		}

		nRemainRecv -= nXfer;
		for (k = 0; k < nXfer && termpBuff[k] != '\0'; k++)
		{
			*pBuffer = termpBuff[k];
			pBuffer++;
		}

		if (k >= nXfer) {
			nRemainRecv -= nXfer;

		}
		else {
			*pBuffer = '\0';
			nRemainRecv = 0;
			memcpy(psh->staticBuff, &termpBuff[k + 1]), nXfer - k - 1);
			psh->staticBuffLen = nXfer - k - 1;
		}

	}
	return !disconnect;
}

__declspec(dllexport) BOOL SendCSMessage(MESSAGE* pMsg, PSOCKET_HANDLE psh)
{
	BOOL disconnect = FALSE;
	LONG32 nRemainSend, nXfer;
	LPSTR pBuffer;
	SOCKET sd;

	if (psh == NULL || pMsg == NULL) return FALSE;
	sd = psh->sk;

	pBuffer = pMsg->record;
	nRemainSend = min(strlen(pBuffer)＋1, MAX_MESSAGE_LEN);
	while (nRemainSend > 0 && !disconnect) {
		nXfer = send(sd, pBuffer, nRemainSend, 0);
		if (nXfer <= 0) {
			disconnect = TRUE;
			continue;
		}
		nRemainSend -= nXfer;
		pBuffer += nXfer;

	}
	return !disconnect;
}