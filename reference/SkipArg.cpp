#include <Windows.h>


LPTSTR SkipArg(LPCTSTR targv)
{
	LPTSTR p;
	p = (LPTSTR)targv;

	while (*p != '\0' && *p != ' ' && *p != '\t') p++;
	while (*p != '\0' && (*p == ' ' || *p == '\t')) p++;

	return p;
}