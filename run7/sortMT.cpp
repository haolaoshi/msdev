#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>


DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);


#define DATALEN		56	
#define KEYLEN		8
typedef struct _RECORD {
	CHAR	key[KEYLEN];
	TCHAR	data[DATALEN];
}RECORD;//  120 ?

#define RECSIZE		sizeof(RECORD)
typedef RECORD* LPRECORD;

typedef struct _THREADARG {
	DWORD iTh;  //Seq
	LPRECORD	lowRecord;//Left Child
	LPRECORD	highRecord;//Right Child
}THREADARG,*PTHREADARG;

static int KeyCompare(LPCTSTR, LPCTSTR);

int KeyCompare(LPCTSTR pKey1, LPCTSTR pKey2)

/* Compare two records of generic characters.
The key position and length are global variables. */
{
	return _tcsncmp( pKey1,  pKey2, KEYLEN);
}
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
	LPTSTR stringEnd;

	iNP = Options(argc, argv, _T("n"), &noPrint, NULL);
	iFF = iNP + 1;
	numFiles = _ttoi(argv[iNP]);
	if (numFiles == 0) {

		_tprintf(_T("No numFiles "));
		return 1;
	}
	//打开或者创建文件并返回句柄
	hFile = CreateFile(argv[iFF], GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		_tprintf(_T("File Open Failed."));
		return 3;
	}
	//For technical reasons
	SetFilePointer(hFile, 2, 0, FILE_END);
	SetEndOfFile(hFile);

	mHandle = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	GetFileSizeEx(hFile, &fileSize);
	nRec = fileSize.QuadPart / RECSIZE; //Total Number of Records
	nRecTh = nRec / numFiles; //per thread 
	threadArg = (PTHREADARG)malloc(numFiles * sizeof(THREADARG));
	pThreadHandle = (HANDLE*)malloc(numFiles * sizeof(HANDLE));

	pRecords = (LPRECORD)MapViewOfFile(mHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	CloseHandle(mHandle);

	lowRecordNum = 0;
	for (iTh = 0; iTh < numFiles; iTh++) {
		threadArg[iTh].iTh = iTh;
		threadArg[iTh].lowRecord = pRecords + lowRecordNum;
		threadArg[iTh].highRecord = pRecords + (lowRecordNum + nRecTh);
		lowRecordNum += nRecTh;
		pThreadHandle[iTh] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)SortThread, &threadArg[iTh], CREATE_SUSPENDED, NULL);
	}

	for (iTh = 0; iTh < numFiles; iTh++) {
		ResumeThread(pThreadHandle[iTh]);
	}

	WaitForSingleObject(pThreadHandle[0],INFINITE);
	for (iTh = 0; iTh < numFiles; iTh++) {
		CloseHandle(pThreadHandle[iTh]);
	}

	stringEnd = (LPTSTR)pRecords + nRec * RECSIZE;
	*stringEnd = _T('\0');
	if (!noPrint) {
		_tprintf(_T("%s"), (LPCTSTR)pRecords);
	}
	UnmapViewOfFile(pRecords);
	SetFilePointer(hFile, -2, 0, FILE_END);
	SetEndOfFile(hFile);

	CloseHandle(hFile);
	free(threadArg);
	free(pThreadHandle);
	return 0;
}

static VOID MergeArrays(LPRECORD, DWORD);

DWORD WINAPI SortThread(PTHREADARG pThArg)
{
	DWORD groupSize = 2, myNumber, twoToi = 1;
	DWORD_PTR  numbersInGroup;
	LPRECORD first;

	myNumber = pThArg->iTh;
	first = pThArg->lowRecord;
	numbersInGroup = pThArg->highRecord - first;

	qsort(first, numbersInGroup, RECSIZE, (_CoreCrtNonSecureSearchSortCompareFunction)KeyCompare);
	while ((myNumber % groupSize) == 0 && numbersInGroup < nRec) {
		WaitForSingleObject(pThreadHandle[myNumber + twoToi], INFINITE);
		MergeArrays(first, numbersInGroup);
		numbersInGroup *= 2;
		groupSize *= 2;
		twoToi *= 2;
	}
	return 0;

}



static VOID MergeArrays(LPRECORD p1, DWORD nRecs)
{
	DWORD iRec = 0, i_1 = 0, i_2 = 0;
	LPRECORD pDest, pHold, pDestHold, p2 = p1 + nRecs;
	pDest = pDestHold = (LPRECORD)malloc(2 * nRecs * RECSIZE + 100);
	pHold = p1;
	while (i_1 < nRecs && i_2 < nRecs) {
		if (KeyCompare((LPCTSTR)p1, (LPCTSTR)p2) <= 0) {
			memcpy(pDest, p1, RECSIZE);
			i_1++;
			p1++;
			pDest++;
		}
		else {
			memcpy(pDest, p2, RECSIZE);
			i_2++;
			p2++;
			pDest++;
		}
	}
	if (i_1 >= nRecs) {
		memcpy(pDest, p2, RECSIZE * (nRecs - i_2));

	}memcpy(pDest, p1, RECSIZE* (nRecs - i_1));

	memcpy(pHold, pDestHold, 2 * nRecs * RECSIZE);
	free(pDestHold);
	return;
}