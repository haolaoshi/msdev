#include <Windows.h>
#include <tchar.h>

DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);


int _tmain(int argc, LPTSTR argv[])
{
	FILETIME newFileTime;
	LPFILETIME pAccessTime = NULL, pModifyTime = NULL;
	HANDLE hFile;
	BOOL setAccessTime, setModTime, NotCreateNew, maFlag;
	DWORD createFlag;
	int i, fileIndex;

	fileIndex = Options(argc, argv, _T("amc"), &setAccessTime, &setModTime, &NotCreateNew, NULL);
	maFlag = setAccessTime || setModTime;
	createFlag = NotCreateNew ? OPEN_EXISTING : OPEN_ALWAYS;
	
	for (i = fileIndex; i < argc; i++) {
		hFile = CreateFile(argv[i], GENERIC_READ | GENERIC_WRITE, 0, NULL, createFlag, FILE_ATTRIBUTE_NORMAL, NULL);
		GetSystemTimeAsFileTime(&newFileTime);
		if (setAccessTime || !maFlag) pAccessTime = &newFileTime;
		if (setModTime || !maFlag) pModifyTime = &newFileTime;
		SetFileTime(hFile, NULL, pAccessTime, pModifyTime);
		CloseHandle(hFile);
	}
	return 0;
}