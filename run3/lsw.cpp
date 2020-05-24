#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>

DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);


BOOL TraverseDirectory(LPCTSTR, DWORD, LPBOOL);
DWORD FileType(LPWIN32_FIND_DATA);
BOOL ProcessItem(LPWIN32_FIND_DATA, DWORD, LPBOOL);

#define MAX_OPTIONS 3
#define TYPE_FILE	1
#define TYPE_DIR	2
#define TYPE_DOT	3
int _tmain(int argc, LPTSTR argv[])
{

	BOOL flags[MAX_OPTIONS], ok = TRUE;
	TCHAR pathName[MAX_PATH + 1], currPath[MAX_PATH + 1], tempPath[MAX_PATH+1];
	LPTSTR  pSplash, pFileName;
	int i, fileIndex;
	fileIndex = Options(argc, argv, _T("Rl"), &flags[0], &flags[1], NULL);
	GetCurrentDirectory(MAX_PATH, currPath);
	if (argc < fileIndex + 1)
		ok = TraverseDirectory(_T("*"), MAX_OPTIONS, flags);
	else for (i = fileIndex; i < argc; i++) {
		_tcscpy_s(pathName, argv[i]);
		_tcscpy_s(tempPath, argv[i]);

		pSplash = _tcsrchr(tempPath, '\\');//_tstrrchr
		if (pSplash != NULL) {
			*pSplash = '\0';
			_tcscat_s(tempPath, _T("\\"));
			SetCurrentDirectory(tempPath);
			pSplash = _tcsrchr(pathName, '\\');//_tstrrchr
			pFileName = pSplash + 1;
		}
		else {
			pFileName = pathName;
		}
		ok = TraverseDirectory(pFileName, MAX_OPTIONS, flags) && ok;
		SetCurrentDirectory(currPath);
	}
}
static BOOL TraverseDirectory(LPCTSTR pathName, DWORD numFlags, LPBOOL flags)
{
	HANDLE searchHandle;
	WIN32_FIND_DATA findData;
	BOOL recursive = flags[0];
	DWORD fType, iPass;
	TCHAR currPath[MAX_PATH + 1];

	GetCurrentDirectory(MAX_PATH, currPath);
	for (iPass = 1; iPass <= 2; iPass++) {
		searchHandle = FindFirstFile(pathName, &findData);
		do {
			fType = FileType(&findData);
			if (iPass == 1)
				ProcessItem(&findData, MAX_OPTIONS, flags);
			if (fType == TYPE_DIR && iPass == 2 && recursive) {
				_tprintf(_T("\n%s\\%s:"), currPath, findData.cFileName);
				SetCurrentDirectory(findData.cFileName);
				TraverseDirectory(_T("*"), numFlags, flags);
				SetCurrentDirectory(_T(".."));
			}
		} while (FindNextFile(searchHandle, &findData));
		FindClose(searchHandle);
	}
	return TRUE;
}
static BOOL ProcessItem(LPWIN32_FIND_DATA pFileData, DWORD numFlags, LPBOOL flags)
{
	const TCHAR fileTypeChar[] = { ' ','d' };
	DWORD fType = FileType(pFileData);
	BOOL bLong = flags[1];
	SYSTEMTIME lastWrite;

	if (fType != TYPE_FILE && fType != TYPE_DIR) return FALSE;

	_tprintf(_T("\n"));

	if (bLong) {
		_tprintf(_T("%c"), fileTypeChar[fType - 1]);
		_tprintf(_T("%10d"), pFileData->nFileSizeLow);
		FileTimeToSystemTime(&(pFileData->ftLastWriteTime), &lastWrite);
		_tprintf(_T("%02d/%02d/%04d  %02d:%02d:%02d"), lastWrite.wMonth, lastWrite.wDay, lastWrite.wYear, lastWrite.wHour, lastWrite.wMinute, lastWrite.wSecond);
	}
	_tprintf(_T("  %s"), pFileData->cFileName);
	return TRUE;
}
static DWORD FileType(LPWIN32_FIND_DATA pFileData)
{
	BOOL isDir;
	DWORD fType;
	fType = TYPE_FILE;
	isDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (isDir) {
		if (lstrcmp(pFileData->cFileName, _T(".")) == 0 || lstrcmp(pFileData->cFileName, _T("..")) == 0)
			fType = TYPE_DOT;
		else
			fType = TYPE_DIR;
	}
	return fType;
}