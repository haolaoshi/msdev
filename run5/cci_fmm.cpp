#include <Windows.h>
#include <tchar.h>

VOID ReportException(LPCTSTR userMessage,DWORD exceptionCode);
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);

BOOL cci_f(LPCTSTR fIn,LPCTSTR fOut,DWORD shift)
	/*Caesar cipher function.
	*fIN	:	source file pathname
	*fOut	:	Destination file pathname
	*shift	:	Numeric shift value*/
{
	BOOL complete = FALSE;
	__try{
		HANDLE hIn,hOut;
		HANDLE hInMap,hOutMap;
		LPTSTR pIn,pInFile,pOut,pOutFile;
		LARGE_INTEGER fileSize;

		hIn = CreateFile(fIn,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		GetFileSizeEx(hIn,&fileSize);
		if(fileSize.HighPart > 0 && sizeof(SIZE_T) == 4)
			ReportException(_T("File is too large for Win32."),4);
		hInMap = CreateFileMapping(hIn,NULL,PAGE_READONLY,0,0,NULL);
		pInFile = (LPTSTR)MapViewOfFile(hInMap,FILE_MAP_READ,0,0,0);
		hOut = CreateFile(fOut,GENERIC_READ |GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		hOutMap = CreateFileMapping(hOut,NULL,PAGE_READWRITE,fileSize.HighPart,fileSize.LowPart,NULL);
		pOutFile =  (LPTSTR)MapViewOfFile(hOutMap,FILE_MAP_WRITE,0,0,(SIZE_T)fileSize.QuadPart);
		__try{
			pIn = pInFile;
			pOut = pOutFile;
			while(pIn < pInFile + fileSize.QuadPart){
				*pOut = (*pIn + shift)%256;
				pIn++;pOut++;
			}
			complete = TRUE;
		}
		__except(GetExceptionCode() == EXCEPTION_IN_PAGE_ERROR ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			complete = FALSE;
			ReportError(_T("Fatal Error accessing mapped file."),9,TRUE);
		}
		UnmapViewOfFile(pOutFile);UnmapViewOfFile(pInFile);
		CloseHandle(hOutMap);CloseHandle(hInMap);
		CloseHandle(hIn);CloseHandle(hOut);
		return complete;
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		if(!complete)
			DeleteFile(fOut);
		return FALSE;
	}

}

int _tmain(int argc, LPTSTR argv[])
{
	if (argc != 4)
		ReportError(_T("Usage : cci shift file1 file2"), 1, FALSE);
	if (!cci_f(argv[2], argv[3], atoi(argv[1])))
		ReportError(_T("Encryption Failed."), 4, TRUE);
	return 0;
}

