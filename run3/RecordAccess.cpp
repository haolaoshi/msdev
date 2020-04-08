#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>

#define STRING_SIZE 256


typedef struct _RECORD
{
	DWORD	referenceCount;
	SYSTEMTIME recordCreationTime;
	SYSTEMTIME recordReferenceTime;
	SYSTEMTIME recordUpdateTime;
	TCHAR dataString[STRING_SIZE];
}RECORD;
typedef struct _HEADER {
	DWORD numberRecords;
	DWORD numNonEmptyRecords;
}HEADER;

int _tmain(int argc, char* argv[])
{
	HANDLE hFile;
	LARGE_INTEGER curPtr;
	DWORD openOption, nxfer, recno;

	RECORD record;
	TCHAR astr[STRING_SIZE], command, extra;
	OVERLAPPED ov = { 0,0,0,0,NULL }, ovzero = { 0,0,0,0,NULL };
	HEADER header = { 0,0 };
	SYSTEMTIME currentTime;
	BOOLEAN headerChange, recordChange;

	openOption = ((argc > 2 && atoi(argv[2]) <= 0) || argc <= 2) ? OPEN_EXISTING : CREATE_ALWAYS;


	  
	_tprintf(_T("File name %s \n"), (LPCTSTR)argv[1]);

	hFile = CreateFile((LPCTSTR)argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL, openOption, FILE_FLAG_RANDOM_ACCESS, NULL);
	if (argc >= 3 && atoi(argv[2]) > 0) {
		header.numberRecords = atoi(argv[2]);
		WriteFile(hFile, &header, sizeof(header), &nxfer, &ovzero);
		curPtr.QuadPart = (LONGLONG)sizeof(RECORD) * atoi(argv[2]) + sizeof(HEADER);
		SetFilePointerEx(hFile, curPtr, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
		return 0;
	}

	ReadFile(hFile, &header, sizeof(HEADER), &nxfer, &ovzero);
	while (TRUE) {
		headerChange = FALSE; recordChange = FALSE;
		_tprintf(_T("Enter r /w/d/q Rec#\n"));
		_tscanf(_T("%c%u%c"), &command, &recno, &extra);
		if (command == 'q') break;
		if (recno >= header.numberRecords) {
			_tprintf(_T("Record number is too large ( %d  -- %d )"), recno , header.numberRecords);
			continue;
		}
		curPtr.QuadPart = (LONGLONG)recno * sizeof(RECORD) + sizeof(HEADER);
		ov.Offset = curPtr.LowPart;
		ov.OffsetHigh = curPtr.HighPart;
		ReadFile(hFile, &record, sizeof(RECORD), &nxfer, &ov);
		GetSystemTime(&currentTime);
		record.recordReferenceTime = currentTime;

		if (command == 'r' || command == 'd') {
			if (record.referenceCount == 0) {
				_tprintf(_T("Record number %d is empty.\n"), recno);
				continue;
			}
			else {
				_tprintf(_T("Record Number %d . Reference count : %d \n"), recno, record.referenceCount);
				_tprintf(_T("Data: %s \n"), record.dataString);

			}
			if (command == 'd') {
				record.referenceCount = 0;
				header.numNonEmptyRecords--;
				headerChange = TRUE;
				recordChange = TRUE;
			}
		}
		else if (command == 'w') {
			_tprintf(_T("Enter new data string for the record .\n"));
			_fgetts(astr, sizeof(astr), stdin);
			astr[_tcslen(astr) - 1] = _T('\0');
			if (record.referenceCount == 0) {
				record.recordCreationTime = currentTime;
				header.numNonEmptyRecords++;
				headerChange = TRUE;
			}
			record.recordUpdateTime = currentTime;
			record.referenceCount++;
		//	errno_t strncpy_s(char* restrict dest, rsize_t destsz,const char* restrict src, rsize_t count);

			strncpy_s((char* )record.dataString,  STRING_SIZE - 1,(char* )astr, STRING_SIZE - 1);
			recordChange = TRUE;
		}
		else {
			_tprintf(_T("command must be r,w,or d . \n"));
		}
		if (recordChange)
			WriteFile(hFile, &record, sizeof(RECORD), &nxfer, &ov);
		if (headerChange)
			WriteFile(hFile, &header, sizeof(header), &nxfer, &ovzero);

	}
	_tprintf(_T("Computed number or  non-empty record is : %d \n"), header.numNonEmptyRecords);
	ReadFile(hFile, &header, sizeof(HEADER), &nxfer, &ovzero);
	_tprintf(_T("File %s has %d non-epty records \n Capacitp : %d \n"), argv[1], header.numNonEmptyRecords, header.numberRecords);
	CloseHandle(hFile);
	return 0;
}