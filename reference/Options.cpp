#include<windows.h>
#include <tchar.h>
#include<stdarg.h>

#ifdef _UNICODE 
#define _memtchr wmemch
#else
#define _memtchr memchr
#endif

DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...)
{
	va_list pFlagList;
	LPBOOL pFlag;
	int iFlag = 0, iArg;

	va_start(pFlagList, OptStr);
	while ((pFlag = va_arg(pFlagList, LPBOOL)) != NULL && iFlag < (int)_tcslen(OptStr)) {
		*pFlag = FALSE;
		for (iArg = 1; !(*pFlag) && iArg < argc && argv[iArg][0] == _T('-'); iArg++)
			*pFlag = _memtchr(argv[iArg], OptStr[iFlag], _tcslen(argv[iArg])) != NULL;
		iFlag++;
	}
	va_end(pFlagList);
	for (iArg = 1; iArg < argc && argv[iArg][0] == _T('-'); iArg++);

	return iArg;
}