#include <Windows.h>

#include <tchar.h>
#define MAX_RQRS_LEN 10
#define	 MAX_MESSAGE_LEN 100
typedef struct MSG_STRUCT {
	char record[MAX_MESSAGE_LEN];
	LONG32 msg_len;
} MESSAGE;
typedef struct STATIC_BUF_T {
	char staticBuf[MAX_RQRS_LEN];
	LONG32 staticBufLen;
} STATIC_BUF;

static DWORD tlsIndex = TLS_OUT_OF_INDEXES;
static char staticBuf[MAX_RQRS_LEN];
static LONG32 staticBufLen;

static volatile long nPa = 0, nPd = 0, nTa = 0, nTd = 0;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	STATIC_BUF* pBuf;
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		tlsIndex = TlsAlloc();
		InterlockedIncrement(&nPa);

	case DLL_THREAD_ATTACH:
		InterlockedIncrement(&nTa);
		return TRUE;

	case DLL_PROCESS_DETACH:
		InterlockedIncrement(&nPd);
		InterlockedIncrement(&nTd);
		pBuf = TlsGetValue(tlsIndex);
		if (pBuf != NULL) {
			free(pBuf);
			pBuf = NULL;
		}
		TlsFree(tlsIndex);
		return TRUE;

	case DLL_THREAD_DETACH:
		InterlockedIncrement(&nTd);
		pBuf = TlsGetValue(tlsIndex);
		if (pBuf != NULL) {
			free(pBuf);
			pBuf = NULL;
		}
		return TRUE;
	default:
		return TRUE;
	}
}

__declspec(dllexport) BOOL ReceiveCSMessage(MESSAGE* pMsg, SOCKET sd)
{
	BOOL disconnect = FALSE;
	LONG32 nRemainRecv, nXfer, k;
	LPBYTE pBuffer, message;
	CHAR tempBuff[MAX_MESSAGE_LEN + 1];
	STATIC_BUF* pBuff;

	if (pMsg == NULL) return FALSE;
	pBuff = (STATIC_BUF*)TlsGetValue(tlsIndex);
	if(pBuff == NULL){
		pBuff = malloc(sizeof(STATIC_BUF));
		if (pBuff == NULL) return FALSE;
		TlsSetValue(tlsIndex, pBuff);
		pBuff->staticBufLen = 0;
	}

	message = pMsg->record;
	for (k = 0; k < pBuff->staticBufLen && pBuff->staticBuf[k] != '\0'; k++) {
		message[k] = pBuff->staticBuf[k];

	}
	
	if (k < pBuff->staticBufLen) {
		message[k] = '\0';
		pBuff->staticBufLen -= (k + 1);
		memcpy(pBuff->staticBuf, &(pBuff->staticBuf[k + 1]), pBuff->staticBufLen);
		return TRUE;
	}
	nRemainRecv = sizeof(temBuff) - sizeof(CHAR) - pBuff->staticBufLen;
	pBuff = message + pBuff->staticBufLen;

	pBuff->staticBufLen = 0;
	while (nRemainRecv > 0 && !disconnect) {
		nXfer = recv(sd, tempBuff, nRemainRecv, 0);
		if (nXfer <= 0) {
			disconnect = TRUE;
			continue;

		}
		for (k = 0; k < nXfer && tempBuff[k] != '\0'; k++) {
			*pBuffer = tempBuff[k];
			nRemainRecv -= nXfer;
			pBuffer++;
		}

		if (k < nXfer) {
			*pBuffer = '\0';
			nRemainRecv = 0;
			memcpy(pBuff->staticBuf, &tempBuff[k + 1], nXfer - k - 1);
			pBuff->staticBufLen = nXfer - k - 1;

		}
	}
	return !disconnect;
}

__declspec(dllexport) BOOL SendCSMessage(MESSAGE* pMsg, SOCKET sd)
{
	BOOL disconnect = FALSE;
	LONG32 nRemainSend, nXfer;
	LPBYTE pBuffer;

	if (pMsg == NULL)return FALSE;
	pBuffer = pMsg->record;

	if (pBuffer == NULL) return FALSE;
	nRemainSend = min(strlen(pBuffer) + 1, MAX_MESSAGE_LEN);
	while (nRemainSend > 0 && !disconnect) {
		nXfer = send(sd, pBuffer, nRemainSend, 0);
		if (nXfer < 0) {
			disconnect = TRUE;
		}
		nRemainSend -= nXfer;
		pBuffer += nXfer;

	}
	return !disconnect;
}