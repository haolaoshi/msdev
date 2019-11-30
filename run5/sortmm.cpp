/* Chapter 5, sortMM command.
	Memory Mapped version - Minimal error reporting. */

/*  sortMM [options] [file]
	Sort one file. The extension to multiple files is straightforward but is omitted here.

	Options:
	-r	Sort in reverse order.
	-I	Use an existing index file to produce the sorted file.

	This limited implementation sorts on the first field only.
	The length of the Key field is determined by the length of the first
	white space terminated field of the first record (line). */

/* This program illustrates:
	1.	Mapping files to memory.
	2.	Use of memory-based string processing functions in a mapped file.
	3.	Use of a memory based function (qsort) with a memory-mapped file.
	4.	_based pointers. */

/* Technique:
	1.	Map the input file (the file to be sorted).
	2.	Using standard character processing, scan the input file,
		placing each key (which is fixed size) into the "index" file record
		(the input file with a .idx suffix).
		The index file is created using file I/O; later it will be mapped.
	3.	Also place the start position of the record in the index file record.
		This is a _based pointer to the mapped input file.
	4.	Map the index file.
	5.	Use the C library qsort to sort the index file
		(which has fixed length records consisting of key and _based pointer).
	6.	Scan the sorted index file, putting input file records in sorted order.
	7.	Skip steps 2, 3, and 5 if the -I option is specified.
		Notice that this requires _based pointers. */
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#define TSIZE	8
#define KEY_SIZE 8

DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
VOID ReportException(LPCTSTR userMessage,DWORD exceptionCode);
int KeyCompare (const void * pKey1, const void * pKey2)

/* Compare two records of generic characters.
	The key position and length are global variables. */
{
	return _tcsncmp ((const char *)pKey1, (const char *)pKey2, KEY_SIZE);
}

//int KeyCompare(LPCTSTR,LPCTSTR);
VOID CreateIndexFile(LARGE_INTEGER,LPCTSTR,LPTSTR);
DWORD_PTR kStart = 0 ,kSize=8;
BOOL reverse;


#define CR 0x0D
#define LF 0x0A


int _tmain(int argc,LPTSTR argv[])
{
	HANDLE hInFile,hInMap;
	HANDLE hXFile,hXMap;
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	BOOL idxExists,noPrint;
	DWORD indexSize,rXSize,iKey,nWrite;
	LARGE_INTEGER inputSize;
	LPTSTR pInFile = NULL;
	LPBYTE pXFile = NULL, pX;
	TCHAR	_based(pInFile) *pIn, indexFileName[MAX_PATH],ChNewLine = _T('\n');
	int FlIdx = Options(argc,argv,_T("rIn"),&reverse,&idxExists,&noPrint,NULL);

	/*Step 1:	Open and Map the Input File.*/
	hInFile = CreateFile(argv[FlIdx],GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	/*Create a file mapping obj, Use file size*/
	hInMap = CreateFileMapping(hInFile,NULL,PAGE_READWRITE,0,0,NULL);
	pInFile = (LPTSTR)MapViewOfFile(hInMap,FILE_MAP_ALL_ACCESS,0,0,0);
	if(!GetFileSizeEx(hInFile,&inputSize))
		ReportError(_T("Failed to get input file size."),5,TRUE);
	/*Step 2 and 3 : Create index file if necessary */
	_stprintf(indexFileName,_T("%s.idx"),argv[FlIdx]);
	if(!idxExists)
		CreateIndexFile(inputSize,indexFileName,pInFile);
	/*Step 4:	Map the index file*/
	hXFile = CreateFile(indexFileName,GENERIC_READ |GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
	/*Create a file mapping obj , use index file size*/
	hXMap = CreateFileMapping(hXFile,NULL,PAGE_READWRITE,0,0,NULL);
	pXFile = (LPBYTE)MapViewOfFile(hXMap,FILE_MAP_ALL_ACCESS,0,0,0);
	indexSize = GetFileSize(hXFile,NULL);/*idx file isn't huge*/
	/* Idividual index record size - key plus a pointer */
	rXSize = kSize * TSIZE + sizeof(LPTSTR);
	/*Step	5:	sort the index File Using qsort */
	if(!idxExists)
		/* KeyCompare use the global variables kSize and kStart */
			qsort(pXFile,indexSize/rXSize,rXSize,KeyCompare);
	/*Step 6:	Output the sorted input file*/
	/*Point to the first pointer in the index file */
	pX = pXFile + rXSize - sizeof(LPTSTR);
	if(!noPrint){
		for(iKey = 0; iKey < indexSize / rXSize;iKey++){
			WriteFile(hStdOut,&ChNewLine,TSIZE,&nWrite,NULL);
			/*the cast on pX is important , as its is a pointer to a character and we need the  or 8 based pointer*/
			pIn = (TCHAR __based(pInFile)*) *(DWORD *)pX;
			while((SIZE_T) pIn < inputSize.QuadPart && (*pIn != CR || *(pIn + 1) != LF)){
				WriteFile(hStdOut,pIn,TSIZE,&nWrite,NULL);
				pIn++;
			}
			pX += rXSize;/*Advance to the next index file pointer*/
		}
		UnmapViewOfFile(pInFile);
		CloseHandle(hInMap);CloseHandle(hInFile);
		UnmapViewOfFile(pXFile);
		CloseHandle(hXMap);CloseHandle(hXFile);
	}
	return 0;
}

VOID CreateIndexFile(LARGE_INTEGER inputSize,LPCTSTR indexFilename,LPTSTR pInFile)
{
	HANDLE hXFile;
	TCHAR _based(pInFile) *pInScan = 0;
	DWORD nWrite;
	/*Step	2:	Create an index file. do not map it yet as its len is unknown*/
	hXFile = CreateFile(indexFilename,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,0,NULL);
	/*Step	3:	scan the complete file , writing keys and record pointers to the key file*/
	while((DWORD_PTR) pInScan < inputSize.QuadPart){
		WriteFile(hXFile,pInScan + kStart , kSize * TSIZE,&nWrite,NULL);
		WriteFile(hXFile,&pInScan,sizeof(LPTSTR),&nWrite,NULL);
		while((DWORD)(DWORD_PTR)pInScan< inputSize.QuadPart - sizeof(TCHAR) && ((*pInScan != CR) || (*(pInScan + 1) != LF))){
			pInScan++;/*skip to end of line*/
		}
		pInScan +=2; /*SKIP past cr, lf.*/
	}
	CloseHandle(hXFile);
}