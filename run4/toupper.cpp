#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

VOID ReportException(LPCTSTR userMessage,DWORD exceptionCode);

int _tmain(int argc,LPTSTR argv[])
{
	HANDLE hIn = INVALID_HANDLE_VALUE,hOut=INVALID_HANDLE_VALUE;
	DWORD nXfer , iFile,j;
	CHAR outFileName[256]="",*pBuffer = NULL;
	OVERLAPPED ov={0,0,0,0,NULL};
	LARGE_INTEGER fSize;

	for(iFile=1;iFile<argc;iFile++) __try{

		if(_tcslen(argv[iFile])>250)
			ReportException(_T("The File Name is too long."),1);
		_stprintf(outFileName,"UC_%s",argv[iFile]);
		__try{
			hIn = CreateFile(argv[iFile],GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
			if(hIn == INVALID_HANDLE_VALUE)
				ReportException(argv[iFile],1);
			if(!GetFileSizeEx(hIn,&fSize) || fSize.HighPart > 0)
				ReportException(_T("This File is too large."),1);
			hOut = CreateFile(outFileName,GENERIC_READ|GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
			if(hOut == INVALID_HANDLE_VALUE)
				ReportException(outFileName,1);
			pBuffer = (CHAR*)malloc(fSize.LowPart);
			if(pBuffer == NULL)
				ReportException(_T("Memory alloc failed."),1);
			if(!ReadFile(hIn,pBuffer,fSize.LowPart,&nXfer,NULL)
				||nXfer != fSize.LowPart)
				ReportException(_T("Read File Error"),1);
			for(j=0 ; j < fSize.LowPart;j++){
				if(isalpha(pBuffer[j])) pBuffer[j] = toupper(pBuffer[j]);
			}
			if(!WriteFile(hOut,pBuffer,fSize.LowPart,&nXfer,NULL)||nXfer != fSize.LowPart)
				ReportException(_T("Write File Rrror ."),1);
		}
		__finally{
			if(pBuffer != NULL) free(pBuffer);pBuffer = NULL;
			if(hIn != INVALID_HANDLE_VALUE) {
				CloseHandle(hIn);
				hIn = INVALID_HANDLE_VALUE;
			}
			if(hOut != INVALID_HANDLE_VALUE){
				CloseHandle(hOut);
				hOut = INVALID_HANDLE_VALUE;
			}
			_tcscpy(outFileName,_T(""));
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		_tprintf(_T("Error processing file %s\n"),argv[iFile]);
		DeleteFile(outFileName);
	}
	_tprintf(_T("All file converted , Exception note above.\n"));
	 return 0;
}