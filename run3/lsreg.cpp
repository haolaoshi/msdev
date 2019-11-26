#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#define TSIZE 1


DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);


BOOL TraverseRegistry(HKEY, LPTSTR, LPTSTR, LPBOOL);
BOOL DisplayPair(LPTSTR, DWORD, LPBYTE, DWORD, LPBOOL);
BOOL DisplaySubKey(LPTSTR, LPTSTR, PFILETIME, LPBOOL);

int _tmain(int argc, LPTSTR argv[])
{
	BOOL flags[2], ok = TRUE;
	TCHAR keyName[MAX_PATH + 1];
	LPTSTR pScan;
	DWORD i,keyIndex;
	HKEY hKey,hNextKey;

	if(argc <= 1){
		_tprintf(_T("sorri , there is no KEY specified.\n"));
		return TRUE;
	}
	LPTSTR PreDefKeyNames[] = {
		_T("HKEY_LOCAL_MACHINE"),_T("HKEY_CURRENT_USER"),
		_T("HKEY_CLASSES_ROOT"),_T("HKEY_CURRENT_CONFIG"),NULL};
	HKEY PreDefKeys[]={
		HKEY_LOCAL_MACHINE,HKEY_CURRENT_USER,
		HKEY_CLASSES_ROOT,HKEY_CURRENT_CONFIG};
	keyIndex = Options(argc,argv,_T("Rl"),&flags[0],&flags[1],NULL);
	pScan = argv[keyIndex];
	for(i = 0; *pScan != _T('\\') && *pScan != _T('\0')
		&& i < MAX_PATH ; pScan++, i++)
		keyName[i] = *pScan;
	keyName[i] = _T('\0');
	if(*pScan == _T('\\')) pScan++;

	for(i =0 ; PreDefKeyNames[i] != NULL && _tcscmp(PreDefKeyNames[i],keyName) != 0; i++);
	hKey = PreDefKeys[i];
	RegOpenKeyEx(hKey,pScan,0,KEY_READ,&hNextKey);
	hKey = hNextKey;

	ok = TraverseRegistry(hKey,argv[keyIndex],NULL,flags);
	RegCloseKey(hKey);
	return ok ? 0 : 1;
}

BOOL TraverseRegistry(HKEY hKey,LPTSTR fullKeyName,LPTSTR subKey,LPBOOL flags)
{
	HKEY hSubKey;
	BOOL recursive = flags[0];
	LONG result;
	DWORD valueType , index;
	DWORD numSubKeys,maxSubKeyLen,numValues,maxValueNameLen,maxValueLen;
	DWORD subKeynameLen,valueNameLen,valueLen;
	FILETIME lastWriteTime;
	LPTSTR subKeyName,valueName;
	LPBYTE value;
	TCHAR fullSubKeyName[MAX_PATH + 1];

	RegOpenKeyEx(hKey,subKey,0,KEY_READ,&hSubKey);
	RegQueryInfoKey(hSubKey,NULL,NULL,NULL,&numSubKeys,&maxSubKeyLen,NULL,&numValues,&maxValueNameLen,&maxValueLen,NULL,&lastWriteTime);
	subKeyName = (LPTSTR)malloc(TSIZE * (maxSubKeyLen + 1));
	valueName = (LPTSTR)malloc(TSIZE * (maxValueNameLen + 1));
	value =(LPBYTE) malloc(maxValueLen);
	for(index = 0; index < numValues; index++){
		valueNameLen = maxValueNameLen+1;
		valueLen = maxValueLen + 1;
		result = RegEnumValue(hSubKey,index,valueName,&maxValueNameLen,NULL,&valueType,value,&valueLen);
		if(result == ERROR_SUCCESS && GetLastError() == 0)
			DisplayPair(valueName,valueType,value,valueLen,flags);
	}
	for(index = 0; index < numSubKeys; index++){
		subKeynameLen = maxSubKeyLen + 1;
		result = RegEnumKeyEx(hSubKey,index,subKeyName,&subKeynameLen,NULL,NULL,NULL,&lastWriteTime);
		if(GetLastError() == 0){
			DisplaySubKey(fullKeyName,subKeyName,&lastWriteTime,flags);
			if(recursive){
				_stprintf(fullSubKeyName,_T("%s\\%s"),fullKeyName,subKeyName);
				TraverseRegistry(hSubKey,fullSubKeyName,subKeyName,flags);
			}
		}
	}
	_tprintf(_T("\n"));
	free(subKeyName);free(valueName);free(value);
	RegCloseKey(hSubKey);
	return TRUE;
}

BOOL DisplayPair(LPTSTR valueName,DWORD valueType,LPBYTE value,DWORD valueLen,LPBOOL flags)
{
	LPBYTE pV=value;
	DWORD i;
	_tprintf(_T("\n%s= "),valueName);
	switch(valueType){
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_BINARY:
		for(i = 0 ; i < valueLen; i++,pV++)
			_tprintf(_T(" %x"),*pV);
		break;
	case REG_DWORD:
		_tprintf(_T("%x"),(DWORD)*value);
		break;
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
	case REG_SZ:
		_tprintf(_T("%s"),(LPTSTR)value);
		break;
	default:
		_tprintf(_T("???"));
	}
	return TRUE;
}

BOOL DisplaySubKey(LPTSTR keyName,LPTSTR subKeyName,PFILETIME pLastWrite,LPBOOL flags)
{
	BOOL longList = flags[1];
	SYSTEMTIME sysLastWrite;
	_tprintf(_T("\n%s"),keyName);

	if(_tcslen(subKeyName) > 0) 
		_tprintf(_T("\\%s"),subKeyName);
	if(longList){
		FileTimeToSystemTime(pLastWrite,&sysLastWrite);
		_tprintf(_T("%02d/%02d%04d %02d:%02d:%02d"),
			sysLastWrite.wMonth,sysLastWrite.wDay,
			sysLastWrite.wYear,sysLastWrite.wHour,
			sysLastWrite.wMinute,sysLastWrite.wSecond);
	}
	return TRUE;

}