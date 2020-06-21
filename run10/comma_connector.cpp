#include <Windows.h>
#include <tchar.h>



int _tmain(int argc,LPCTSTR argv[])
{
	if(argc != 2){

		_tprintf(_T("Error !"));
		return 1;
	}
 
#define BUF_SIZE	256


	HANDLE hIn, hOut;
	DWORD nIn, nOut, iCopy,iSave;
	CHAR aBuffer[BUF_SIZE], ccBuffer[BUF_SIZE];
	BOOL writeOK = TRUE;
	 
	 ZeroMemory(&ccBuffer,BUF_SIZE);

	hIn = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) return FALSE;

	hOut = CreateFile(_T("foutfile.txt"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) return FALSE;
	iSave = 0;

	while (ReadFile(hIn, aBuffer, BUF_SIZE, &nIn, NULL) && nIn > 0) {
		for (iCopy = 0; iCopy < nIn; iCopy++){
			ccBuffer[iSave] = aBuffer[iCopy];
			if((aBuffer[iCopy]) == '\r'){
				ccBuffer[iSave] = ',';
				iCopy++;  
			}
			iSave++;
		} 
		ccBuffer[iSave] = '\0';
		WriteFile(hOut,ccBuffer,iSave,&nOut,NULL);
	}
	WriteFile(hOut,'\0',1,&nOut,NULL);

	CloseHandle(hIn);
	CloseHandle(hOut);

	return 0;
}