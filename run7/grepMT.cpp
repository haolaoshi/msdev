
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>


//grep thread's data structure
typedef struct {
	int argc;//4
	TCHAR targv[4][MAX_PATH];//8
}GREP_THREAD_ARG;//12

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

	/*BOSS Thread : Create separate "grep" thread for each file. */
	tHandle = (HANDLE*)malloc((argc - 2) * sizeof(HANDLE));
	gArg = (GREP_THREAD_ARG*)malloc((argc - 2) * sizeof(GREP_THREAD_ARG));//timep grepMT  1 2 3 
	for (iThd = 0; iThd < argc - 2; iThd++) {
		_tcscpy(gArg[iThd].targv[1], argv[1]); //Pattern 
		_tcscpy(gArg[iThd].targv[2], argv[iThd + 2]); // 
		GetTempFileName(_T("."), _T("Gre"), 0, gArg[iThd].targv[3]);//temp file name
		gArg[iThd].argc = 4;
		/*Create Work Thread to execute command line. */
		tHandle[iThd] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ThGrep, &gArg[iThd], 0, NULL);

	}
	/*Redirect std output for file listing process . */
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

void tchar2char(const TCHAR* tch, char* ch)
{
	int len;
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)tch, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, (LPCWCH)tch, -1, ch, len, NULL, NULL);
}
/* Source code for grep follows and is omitted from text. */
/* The form of the code is:
	static DWORD WINAPI ThGrep (GR_ARGS pArgs)
	{
	}
*/
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*	grep, written as a function to be executed on a thread. */
/*	Copyright 1995, Alan R. Feuer. */
/*	Modified version: The input and output file names
	are taken from the argument data structure for the file.
	This function uses the C library and therefore must
	be invoked by _beginthreadex. */

	/*	grep pattern file(s)
		Looks for pattern in files. A file is scanned line-by-line.

		These metacharacters are used:
			*	matches zero or more characters
			?	matches exactly one character
			[...]	specifies a character class
				matches any character in the class
			^	matches the beginning of the line
			$	matches the end of the line
		In addition, the standard C escape sequences are
		understood: \a, \b, \f, \t, \v */

		/*	Codes for pattern metacharacters. */

#define ASTRSK		1
#define QM		2
#define BEGCLASS	3
#define ENDCLASS	4
#define ANCHOR		5

FILE* openFile(char*, char*);
void prepSearchString(char*, char*);

BOOL patternMatch(char*, char*);

/* Other codes and definitions. */

#define EOS '\0'

/* Options for pattern match. */
static int patternSeen = FALSE;
static BOOL ignoreCase = FALSE;


static DWORD WINAPI ThGrep(PGR_ARGS pArgs)
{
	/* Modified version - the first argument is the
		pattern, the second is the input file and the
		third is the output file.
		argc is not used but is assumed to be 4. */

	char* file = (char*)malloc(MAX_PATH);
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
	char* m = (char*)malloc(4);
	strcpy(m, "wb");
	 
	tchar2char(argv[argc - 1], file);
	fpout = openFile(file, m);
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
			tchar2char(argv[i], file);
			char* rb = (char*)malloc(4);
			strcpy(rb, "rb");

			if (!patternSeen++) {
				char* sar = (char*)malloc(MAX_PATH);
				tchar2char(argv[i], sar);
				prepSearchString(sar, pattern);
			}				
			else if ((fp = openFile(file, rb))
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

static FILE*
openFile(char* file, char* mode)
{
	FILE* fp;

	/* printf ("Opening File: %s", file); */

	if ((fp = fopen(file, mode)) == NULL)
		perror(file);
	return (fp);
}


static void
prepSearchString(char* p, char* buf)

/* Copy prep'ed search string to buf. */
/*The register keyword specifies that the variable is to be stored in a machine register, if possible.*/
{
	register int c;
	register int i = 0;

	if (*p == '^') {
		buf[i++] = ANCHOR;
		++p;
	}

	for (;;) {
		switch (c = *p++) {
		case EOS: goto Exit;
		case '*': if (i >= 0 && buf[i - 1] != ASTRSK)
			c = ASTRSK; break;
		case '?': c = QM; break;
		case '[': c = BEGCLASS; break;
		case ']': c = ENDCLASS; break;

		case '\\':
			switch (c = *p++) {
			case EOS: goto Exit;
			case 'a': c = '\a'; break;
			case 'b': c = '\b'; break;
			case 'f': c = '\f'; break;
			case 't': c = '\t'; break;
			case 'v': c = '\v'; break;
			case '\\': c = '\\'; break;
			}
			break;
		}

		buf[i++] = (ignoreCase ? tolower(c) : c);
	}

Exit:
	buf[i] = EOS;
}

static BOOL
patternMatch(char* pattern, char* string)

/* Return TRUE if pattern matches string. */
{
	register char pc, sc;
	char* pat;
	BOOL anchored;

	if (anchored = (*pattern == ANCHOR))
		++pattern;

Top:			/* Once per char in string. */
	pat = pattern;

Again:
	pc = *pat;
	sc = *string;

	if (sc == '\n' || sc == EOS) {
		/* at end of line or end of text */
		if (pc == EOS) goto Success;
		else if (pc == ASTRSK) {
			/* patternMatch (pat + 1,base, index, end) */
			++pat;
			goto Again;
		}
		else return (FALSE);
	}
	else {
		if (pc == sc || pc == QM) {
			/* patternMatch (pat + 1,string + 1) */
			++pat;
			++string;
			goto Again;
		}
		else if (pc == EOS) goto Success;
		else if (pc == ASTRSK) {
			if (patternMatch(pat + 1, string)) goto Success;
			else {
				/* patternMatch (pat, string + 1) */
				++string;
				goto Again;
			}
		}
		else if (pc == BEGCLASS) { /* char class */
			BOOL clmatch = FALSE;
			while (*++pat != ENDCLASS) {
				if (!clmatch && *pat == sc) clmatch = TRUE;
			}
			if (clmatch) {
				++pat;
				++string;
				goto Again;
			}
		}
	}

	if (anchored) return (FALSE);

	++string;
	goto Top;

Success:
	return (TRUE);
}



