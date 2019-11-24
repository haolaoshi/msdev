#include <Windows.h>
#include <tchar.h>
#include<io.h>

BOOL cci_f(LPCTSTR, LPCTSTR, DWORD);
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);
//program 2.3
int _tmain(int argc, LPTSTR argv[])
{

	if (argc != 4)
		ReportError(_T("Usage : cci shift file1 file2"), 1, FALSE);
	if (!cci_f(argv[2], argv[3], atoi(argv[1])))
		ReportError(_T("Encryption Failed."), 4, TRUE);
	return 0;
}

//program 2.4
#define BUF_SIZE	256
BOOL cci_f(LPCTSTR fIn, LPCTSTR fOut, DWORD shif)
{
	HANDLE hIn, hOut;
	DWORD nIn, nOut, iCopy;
	CHAR aBuffer[BUF_SIZE], ccBuffer[BUF_SIZE];
	BOOL writeOK = TRUE;

	hIn = CreateFile(fIn, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hIn == INVALID_HANDLE_VALUE) return FALSE;

	hOut = CreateFile(fOut, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hOut == INVALID_HANDLE_VALUE) return FALSE;

	while (ReadFile(hIn, aBuffer, BUF_SIZE, &nIn, NULL) && nIn > 0 && writeOK) {
		for (iCopy = 0; iCopy < nIn; iCopy++)
			ccBuffer[iCopy] = (aBuffer[iCopy] + shif) % 256;
		writeOK = WriteFile(hOut, ccBuffer, nIn, &nOut, NULL);
	}

	CloseHandle(hIn);
	CloseHandle(hOut);
	return writeOK;
}