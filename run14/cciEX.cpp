#include <Windows.h>
#include <tchar.h>

#define MAX_OVERLP	4
#define REC_SIZE 0x8000  /*size is not as important as cciOV*/
#define debug  0

VOID  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
VOID  ReportException(LPCTSTR userMessage, DWORD exceptionCode);

static VOID WINAPI ReadDone(DWORD,DWORD,LPOVERLAPPED);
static VOID WINAPI WriteDone(DWORD,DWORD,LPOVERLAPPED);
/*the first OV is for readin,the second OV is for write.
each outstanding operation has a single OV*/
OVERLAPPED overLapIn[MAX_OVERLP], overLapOut[MAX_OVERLP];

CHAR rawRec[MAX_OVERLP][REC_SIZE], ccRec[MAX_OVERLP][REC_SIZE];
HANDLE hInputFile, hOutputFile;
LONGLONG nRecords, nDone;

DWORD shift ; //, nIn[MAX_OVERLP], nOut[MAX_OVERLP], ic, i;
LARGE_INTEGER  fileSize;

int _tmain(int argc, LPTSTR argv[])
{
	DWORD ic;
	LARGE_INTEGER curPosIn;

	shift = _ttoi(argv[1]);

	hInputFile = CreateFile(argv[2], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	hOutputFile = CreateFile(argv[3], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);

	GetFileSizeEx(hInputFile, &fileSize);
	nRecords = (fileSize.QuadPart + REC_SIZE - 1) / REC_SIZE;

	curPosIn.QuadPart = 0;


	for (ic = 0; ic < MAX_OVERLP; ic++) {
		overLapIn[ic].hEvent = (HANDLE)ic;
		overLapOut[ic].hEvent = (HANDLE)ic;

		overLapIn[ic].Offset = curPosIn.LowPart;
		overLapIn[ic].OffsetHigh = curPosIn.HighPart;

		if (curPosIn.QuadPart < fileSize.QuadPart){
			if(debug) _tprintf(_T("\n$Main fileSize : %d "),ic);
			ReadFileEx(hInputFile, rawRec[ic], REC_SIZE, &overLapIn[ic],ReadDone);
		}
		curPosIn.QuadPart += (LONGLONG)REC_SIZE;
	}
	/****************************************************/
	/*ALL rread operation are running , Enter a alterable wait state, 
	continue util all records have been processed*/

	nDone = 0;
	while(nDone < 2 * nRecords){
		SleepEx(0,TRUE);
		if(debug) _tprintf(_T("\n-------MainLoop Seeep -%d"),nDone);
	}
	CloseHandle(hInputFile);
	CloseHandle(hOutputFile);

	//system("pause");

	return 0;
}

static VOID WINAPI ReadDone(DWORD Code,DWORD nBytes,LPOVERLAPPED pOv){
	LARGE_INTEGER curPosIn, curPosOut ;
	DWORD i,ic;/*represent value from event field*/
	nDone++;
	if(debug) _tprintf(_T("\n$ReadDone : %d "),nBytes);

	ic = PtrToInt(pOv->hEvent);

	curPosIn.LowPart = overLapIn[ic].Offset;
	curPosIn.HighPart = overLapIn[ic].OffsetHigh;

	curPosOut.QuadPart = (curPosIn.QuadPart / REC_SIZE) *REC_SIZE;
	overLapOut[ic].Offset = curPosOut.LowPart;
	overLapOut[ic].OffsetHigh = curPosOut.HighPart;

	for (i = 0; i < nBytes; i++)
		ccRec[ic][i] = (rawRec[ic][i] + shift) % 256;

	WriteFileEx(hOutputFile, ccRec[ic], nBytes, &overLapOut[ic],WriteDone);

	curPosIn.QuadPart += REC_SIZE * (LONGLONG)(MAX_OVERLP);
	overLapIn[ic].Offset = curPosIn.LowPart;
	overLapIn[ic].OffsetHigh = curPosIn.HighPart;

	return;
}

static VOID WINAPI WriteDone(DWORD Code,DWORD nBytes,LPOVERLAPPED pOv)
{

	LARGE_INTEGER curPosIn;
	DWORD ic;
	nDone++;
	if(debug)	_tprintf(_T("\n$WriteDone : %d "),nBytes);

	ic = PtrToInt(pOv->hEvent);



	curPosIn.LowPart = overLapIn[ic].Offset;
	curPosIn.HighPart = overLapIn[ic].OffsetHigh;
	if (curPosIn.QuadPart < fileSize.QuadPart)
		ReadFileEx(hInputFile, rawRec[ic], REC_SIZE,   &overLapIn[ic],ReadDone);



	return ;
}
