#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>
typedef struct {
	int argc;
	TCHAR targv[4][MAX_PATH];
}GREP_THREAD_ARG;

typedef GREP_THREAD_ARG* PGR_ARGS;
static DWORD WINAPI ThGrep(PGR_ARGS pArgs);
#define MAX_COMMAND_LINE 100
int _tmain(int argc, LPTSTR argv[])
{
	GREP_THREAD_ARG* gArg;
	HANDLE* tHandle;
	DWORD threadIndex, exitCode;
	TCHAR commandLine[MAX_COMMAND_LINE];
	int iThd, threadCount;
	STARTUPINFO startup;
	PROCESS_INFORMATION processinfo;
	GetStartupInfo(&startup);
	tHandle = malloc((argc - 2) * sizeof(HANDLE));
	gArg = malloc((argc - 2) * sizeof(GREP_THREAD_ARG));
	for (iThd = 0; iThd < argc - 2; iThd++) {
		_tcscpy(gArg[iThd].targv[1], argv[1]);
		_tcscpy(gArg[iThd].targv[2], argv[iThd + 2]);
		GetTempFileName(_T("."), _T("Gre"), 0, gArg[iThd].targv[3]);
		gArg[iThd].argc = 4;
		tHandle[iThd] = (HANDLE)_beginthreadex(NULL, 0, ThGrep, &gArg[iThd], 0, NULL);

	}
	startup.dwFlags = STARTF_USESTDHANDLES;
	startup.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	threadCount = argc - 2;
	while (threadCount > 0) {
		threadIndex = WaitForMultipleObjects(threadCount, tHandle, FALSE, INFINITE);
		iThd = (int)threadIndex - (int)WAIT_OBJECT_0;
		GetExitCodeThread(tHandle[iThd], &exitCode);
		CloseHandle(tHandle[iThd]);
		if (exitCode == 0) {
			if (argc > 3) {
				_tprintf(_T("\n**search results - file: %s \n"), gArg[iThd].targv[2]);
				fflush(stdout);
			}
			_stprintf(commandLine, _T("%s%s"), _T("cat "), gArg[iThd].targv[3]);
			CreateProcess(NULL, commandLine, NULL, NULL, TRUE, 0, NULL, NULL, &startup, &processinfo);
			WaitForSingleObject(processinfo.hProcess, INFINITE);
			CloseHandle(processinfo.hProcess);
			CloseHandle(processinfo.hThread);

		}
		DeleteFile(gArg[iThd].targv[3]);
		tHandle[iThd] = tHandle[threadCount - 1];
		_tcscpy(gArg[iThd].targv[3], gArg[threadCount - 1].targv[3]);
		_tcscpy(gArg[iThd].targv[2], gArg[threadCount - 1].targv[2]);
		threadCount--;
	}
	return 0;
}
static FILE*
openFile(char* file, char* mode)
{
	FILE* fp;

	/* printf ("Opening File: %s", file); */

	if ((fp = fopen(file, mode)) == NULL)
		perror(file);
	return (fp);
}

static DWORD WINAPI ThGrep(PGR_ARGS pArgs)
{
	/* Modified version - the first argument is the
		pattern, the second is the input file and the
		third is the output file.
		argc is not used but is assumed to be 4. */

	char* file;
	int i, patternSeen = FALSE, showName = FALSE, argc, result = 1;
	char pattern[256];
	char string[2048];
	TCHAR argv[4][MAX_COMMAND_LINE];
	FILE* fp, * fpout;

	argc = pArgs->argc;
	_tcscpy(argv[1], pArgs->targv[1]);
	_tcscpy(argv[2], pArgs->targv[2]);
	_tcscpy(argv[3], pArgs->targv[3]);
	if (argc < 3) {
		puts("Usage: grep output_file pattern file(s)");
		return 1;
	}

	/* Open the output file. */

	fpout = openFile(file = argv[argc - 1], "wb");
	if (fpout == NULL) {
		printf("Failure to open output file.");
		return 1;
	}

	for (i = 1; i < argc - 1; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'y':
				ignoreCase = TRUE;
				break;
			}
		}
		else {
			if (!patternSeen++)
				prepSearchString(argv[i], pattern);
			else if ((fp = openFile(file = argv[i], "rb"))
				!= NULL) {
				if (!showName && i < argc - 2) ++showName;
				while (fgets(string, sizeof(string), fp)
					!= NULL && !feof(fp)) {
					if (ignoreCase) _strlwr(string);
					if (patternMatch(pattern, string)) {
						result = 0;
						if (showName) {
							fputs(file, fpout);
							fputs(string, fpout);
						}
						else fputs(string, fpout);
					}
				}
				fclose(fp);
				fclose(fpout);
			}
		}
	}
	return result;
}