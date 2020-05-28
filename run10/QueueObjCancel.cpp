/* 10 - 7 */


#include "SynchObj.h"

static BOOL shutDownGet = FALSE;
static BOOL shutDownPut = FALSE;

void WINAPI QueueShutDown(DWORD n)
{
	_tprintf(_T("in shutdown queue callback . %d\n"), n);
	if (n % 2 == 0) shutDownGet = TRUE;
	if ((n / 2) % 2 != 0) shutDownPut = TRUE;
	return;
}



