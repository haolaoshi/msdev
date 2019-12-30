#include <Windows.h>

#define DATALEN		56	
#define KEYLEN		8
typedef struct _RECORD {
	CHAR	key[KEYLEN];
	TCHAR	data[DATALEN];
}RECORD;

#define RECSIZE		sizeof(RECORD)
typedef RECORD* LPRECORD;

typedef struct _THREADARG {
	DWORD iTh;
	LPRECORD	lowRecord;
	LPRECORD	highRecord;
}THREADARG,*PTHREADARG;

static int KeyCompare(LPCTSTR, LPCTSTR);
static DWORD WINAPI SortThread(PTHREADARG pTharg);
static DWORD nRec;
static HANDLE* pThreadHandle;

int _tmain(int argc, LPTSTR argv[])
{
	HANDLE hFile, mHandle;
	LPRECORD pRecords = NULL;
	DWORD lowRecordNum, nRecTh, numFiles, iTh;
	LARGE_INTEGER fileSize;
	BOOL noPrint;
	int iFF, iNP;
	PTHREADARG threadArg;
	LPTSTR stirngEnd;
	iNP = Options(argc, argv, _T("n"), &noPrint, NULL);
	iFF = iNP + 1;

}