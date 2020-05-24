/* Chapter 5. sortFL.  File sorting with memory mapping.
	sortFL file */

/* This program illustrates:
	1.	Using a memory-based algorithm (qsort) with a memory mapped file.
	2.	Outputting the mapped file as a single string.
	3.	try-except block to clean up (delete a temp file, in this case)
		either on normal completion or in case of an exception. */

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>


#define DATALEN 56
#define KEY_SIZE 8
#define TSIZE	1

typedef struct _RECORD{
	TCHAR key[KEY_SIZE];
	TCHAR data[DATALEN];
}RECORD;

#define RECSIZE sizeof (RECORD)
typedef RECORD * LPRECORD;
//int KeyCompare (LPCTSTR, LPCTSTR);


DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);
VOID ReportException(LPCTSTR userMessage,DWORD exceptionCode);
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);


/*  CODE FROM HERE TO END IS NOT INCLUDED IN TEXT. */
// int (__cdecl * _PtFuncCompare)(const void *, const void *));
int KeyCompare (const void * pKey1, const void * pKey2)

/* Compare two records of generic characters.
	The key position and length are global variables. */
{
	return _tcsncmp (pKey1, pKey2, KEY_SIZE);
}

int _tmain(int argc ,LPTSTR argv[])
{
	/* The file is the first argument. Sorting is done in place. */
	/* Sorting is done by file memory mapping. */
	HANDLE hFile = INVALID_HANDLE_VALUE, hMap = NULL;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	LPVOID pFile = NULL;
	DWORD FsLow,Result=2;
	LARGE_INTEGER fileSize;
	TCHAR tempFile[MAX_PATH];
	LPTSTR pTFile;

	BOOL NoPrint;
	int iFirstFile;
	iFirstFile = Options(argc,argv,_T("n"),&NoPrint,NULL);
	if(argc <= iFirstFile)
		ReportError(_T("Usage : sortfl [options] files"),1,FALSE);
	//__try{

		_stprintf(tempFile,_T("%s.tmp"),argv[1]);
		CopyFile(argv[1],tempFile,TRUE);
		Result = 1;

		hFile = CreateFile(tempFile,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if(hFile==INVALID_HANDLE_VALUE)
			ReportException(_T("File open Failed"),2);
		FsLow = GetFileSize(hFile,NULL);
		if(FsLow == 0){
			CloseHandle(hFile);
			return 0;
		}
		GetFileSizeEx(hFile,&fileSize);
		fileSize.QuadPart += 2;
		hMap = CreateFileMapping(hFile,NULL,PAGE_READWRITE,fileSize.HighPart,fileSize.LowPart,NULL);
		pFile = MapViewOfFile(hMap,FILE_MAP_ALL_ACCESS,0,0,0);
				/* Now sort the file.
			Perform the sort with the C library - in mapped memory. */

	qsort (pFile, FsLow / RECSIZE, RECSIZE, KeyCompare);
		//qsort(pFile,FsLow/RECSIZE,RECSIZE,KeyCompare);
		pTFile = (LPTSTR)pFile;
		pTFile[fileSize.QuadPart/TSIZE] = '\0';
		_tprintf(_T("%s"),pFile);
	//}
	UnmapViewOfFile(pFile);
	CloseHandle(hMap);
	CloseHandle(hFile);
	DeleteFile(tempFile);
	return 0;
}

