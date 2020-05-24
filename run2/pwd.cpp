#include<Windows.h>
#include<tchar.h>

#define DIRNAME_LEN (MAX_PATH +2)

BOOL PrintMsg(HANDLE hOut, LPCTSTR pMsg);
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);
int _tmain(int argc, LPTSTR argv[])
{
	TCHAR pwdBuffer[DIRNAME_LEN];
	DWORD lenCurDir;
	lenCurDir = GetCurrentDirectory(DIRNAME_LEN, pwdBuffer);
	if (lenCurDir == 0)
		ReportError(_T("Failed Get Pathname."), 1, TRUE);
	if (lenCurDir > DIRNAME_LEN)
		ReportError(_T("Pathname too long."), 2, FALSE);
	PrintMsg(GetStdHandle(STD_OUTPUT_HANDLE), pwdBuffer);
	return 0;
}