#include <Windows.h>
#include <tchar.h>

#define MAX_OVERLP	4
#define REC_SIZE 0x4000

VOID  ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
VOID  ReportException(LPCTSTR userMessage, DWORD exceptionCode);

int _tmain(int argc, LPTSTR argv[])
{
	HANDLE hInputFile, hOutputFile;
	DWORD shift, nIn[MAX_OVERLP], nOut[MAX_OVERLP], ic, i;
	OVERLAPPED overLapIn[MAX_OVERLP], overLapOut[MAX_OVERLP];
	HANDLE hEvents[2][MAX_OVERLP];
	CHAR rawRec[MAX_OVERLP][REC_SIZE], ccRec[MAX_OVERLP][REC_SIZE];
	LARGE_INTEGER curPosIn, curPosOut, fileSize;
	LONGLONG nRecords, iWaits;

	shift = _ttoi(argv[1]);

	hInputFile = CreateFile(argv[2], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	hOutputFile = CreateFile(argv[3], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);

	GetFileSizeEx(hInputFile, &fileSize);
	nRecords = (fileSize.QuadPart + REC_SIZE - 1) / REC_SIZE;

	curPosIn.QuadPart = 0;
	for (ic = 0; ic < MAX_OVERLP; ic++) {
		hEvents[0][ic] = overLapIn[ic].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		hEvents[1][ic] = overLapOut[ic].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		overLapIn[ic].Offset = curPosIn.LowPart;
		overLapIn[ic].OffsetHigh = curPosIn.HighPart;

		if (curPosIn.QuadPart < fileSize.QuadPart)
			ReadFile(hInputFile, rawRec[ic], REC_SIZE, &nIn[ic], &overLapIn[ic]);

		curPosIn.QuadPart += (LONGLONG)REC_SIZE;

	}
	iWaits = 0;
	while (iWaits < 2 * nRecords)
	{
		ic = WaitForMultipleObjects(2 * MAX_OVERLP, hEvents[0], FALSE, INFINITE)  -WAIT_OBJECT_0;
		iWaits++;
		if (ic < MAX_OVERLP) {
			GetOverlappedResult(hInputFile, &overLapIn[ic], &nIn[ic], FALSE);
			ResetEvent(hEvents[0][ic]);
			curPosIn.LowPart = overLapIn[ic].Offset;
			curPosIn.HighPart = overLapIn[ic].OffsetHigh;
			curPosOut.QuadPart = curPosIn.QuadPart;
			overLapOut[ic].Offset = curPosIn.LowPart;
			overLapOut[ic].OffsetHigh = curPosOut.HighPart;
			for (i = 0; i < nIn[ic]; i++)
				ccRec[ic][i] = (rawRec[ic][i] + shift) % 256;
			WriteFile(hOutputFile, ccRec[ic], nIn[ic], &nOut[ic], &overLapOut[ic]);
			curPosIn.QuadPart += REC_SIZE * (LONGLONG)(MAX_OVERLP);
			overLapIn[ic].Offset = curPosIn.LowPart;
			overLapIn[ic].OffsetHigh = curPosIn.HighPart;
		}else if(ic < 2 * MAX_OVERLP){
			ic -= MAX_OVERLP;
			GetOverlappedResult(hOutputFile, &overLapOut[ic], &nOut[ic], FALSE);
			ResetEvent(hEvents[1][ic]);
			curPosIn.LowPart = overLapIn[ic].Offset;
			curPosIn.HighPart = overLapIn[ic].OffsetHigh;
			if (curPosIn.QuadPart < fileSize.QuadPart)
				ReadFile(hInputFile, rawRec[ic], REC_SIZE, &nIn[ic], &overLapIn[ic]);


		}
		else {
			ReportError(_T("Multiple wait error"), 0, TRUE);
		}


	}

	for (ic = 0; ic < MAX_OVERLP; ic++) {
		CloseHandle(hEvents[0][ic]);
		CloseHandle(hEvents[1][ic]);
	}
	CloseHandle(hInputFile);
	CloseHandle(hOutputFile);

	return 0;
}
