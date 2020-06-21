#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <string>

DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);


BOOL TraverseDirectory(LPCTSTR, DWORD, LPBOOL);
DWORD FileType(LPWIN32_FIND_DATA);
BOOL ProcessItem(LPWIN32_FIND_DATA, DWORD, LPBOOL);
VOID showUsage();
	int startPos = 0,endPos = 0;



VOID showUsage(){
	_tprintf(_T("rename [-R]  path   startPos  endPos\n"));
	_tprintf(_T(" [-R] recursively rename\n"));
	_tprintf(_T(" extract name from start to pos <not include self>\n"));
   _tprintf(_T(" ½ØÍ·½ØÎ²º¯Êý\n"));
}
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
	if ((argc < fileIndex + 1) ){
		//ok = TraverseDirectory(_T("*"), MAX_OPTIONS, flags);
		showUsage();
		exit(1);
	}

	startPos = _ttoi(argv[fileIndex+1]);
	endPos = _ttoi(argv[fileIndex+2]);

		_tcscpy_s(pathName, argv[fileIndex]);
		_tcscpy_s(tempPath, argv[fileIndex]);

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
	 return ok;
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
		if(searchHandle != INVALID_HANDLE_VALUE)
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

	if (fType != TYPE_FILE) return FALSE;

	_tprintf(_T("\n"));

	if (bLong) {
		_tprintf(_T("%c"), fileTypeChar[fType - 1]);
		_tprintf(_T("%10d"), pFileData->nFileSizeLow);
		FileTimeToSystemTime(&(pFileData->ftLastWriteTime), &lastWrite);
		_tprintf(_T("%02d/%02d/%04d  %02d:%02d:%02d"), lastWrite.wMonth, lastWrite.wDay, lastWrite.wYear, lastWrite.wHour, lastWrite.wMinute, lastWrite.wSecond);
	}
	_tprintf(_T("  %s"), pFileData->cFileName);
	int lens = wcslen(pFileData->cFileName);
	
	TCHAR *currdir = new TCHAR[MAX_PATH]; 
	
	if(endPos < 0 || endPos >= lens){
		_tprintf(_T("invalid end pos to %d\n"),endPos);
		return FALSE;
	}
	TCHAR *pos = pFileData->cFileName+ lens;
	_tcsnccpy(currdir,pFileData->cFileName,lens-1);
	currdir[lens-1-endPos] = '\0';
 
	if(startPos >= (lens-endPos) || startPos >= lens){
		_tprintf(_T("invalid start pos to %d\n"),startPos);
		return FALSE;
	}
	 
	_tcscpy(currdir,currdir+startPos);
	 
  
	//currdir[6]='\0';
	_trename(pFileData->cFileName,currdir);
	delete currdir;
	currdir = NULL;
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