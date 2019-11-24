#include <Windows.h>
#include <tchar.h>
 

#define BUF_SIZE	256

static VOID CatFile(HANDLE, HANDLE);
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);
LPSTR  a = new char[4];//�ַ�����ÿ���ַ�1�ֽ�
LPCSTR b = new char[10];//�ַ�����ÿ���ַ�1�ֽ�,C��ʾ����
LPTSTR c = new char[20];//ͨ������
LPCTSTR d= new char[16];//ͨ�����ͣ�C��ʾ����


int _tmain(int argc, LPTSTR argv[])
{
	HANDLE hInFile, hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	BOOL dashS;
	int iArg, iFirstFile;

	iFirstFile = Options(argc, argv, _T("s"), &dashS, NULL);
	if (iFirstFile == argc) {
		CatFile(hStdIn, hStdOut);
		return 0;
	}

	for (iArg = iFirstFile; iArg < argc; iArg++)
	{
		hInFile = CreateFile(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hInFile == INVALID_HANDLE_VALUE) {
			if (!dashS) ReportError(_T("Error File doesnot exist."),0,FALSE);

		}
		else {
			CatFile(hInFile, hStdOut);
			if (GetLastError() != 0 && !dashS) {
				ReportError(_T("Cat Error "), 0, FALSE);
			}
			CloseHandle(hInFile);
		}
	}


	return 0;
}

static VOID CatFile(HANDLE hInFile, HANDLE hOutFile) {
	DWORD nIn, nOut;
	BYTE buffer[BUF_SIZE];
	while (ReadFile(hInFile, buffer, BUF_SIZE, &nIn, NULL) && (nIn != 0) && WriteFile(hOutFile, buffer, nIn, &nOut, NULL));
	return;
}