#include <Windows.h>
#include <tchar.h>

void GetArgs(LPCTSTR command, int* pargc, LPTSTR argstr[])
{
	int i, icm = 0;
	DWORD ic = 0;

	for (i = 0; i < _tcslen(command); i++)
	{
		while (ic < _tcslen(command) && command[ic] != ' ' && command[ic] != '\t') {
			argstr[ic][icm] = command[ic];
			ic++;
			icm++;
		}
		argstr[i][icm] = '\0';
		while (ic < _tcslen(command)  && (command[ic] == ' ') || command[ic] == '\t')
			ic++;
		icm = 0;
	}
	if (pargc != NULL) *pargc = i;
	return;
}