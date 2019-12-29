/*
grep pattern files;
Looks for pattern in files. a file is scanned loine by line;

These metacharacters are used ;
	*	matches zero or more characters
	?	matches exactly one characters
	[...]	specifies a character class
	^	matches any character in the class
	^	matches the begining of the liine
	$	matches the end of the line

	In addition , the standard c escape sequences are understood;
	\a,\b,\f,\v

	
	
*/
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#define	STAR				1
#define	QM					2
#define BEGCLASS			3
#define ENDCLASS			4
#define ANCHOR				5

#define EOS				'\0'

BOOL	ignoreCase = FALSE;

FILE* openFile(char* file, char* mode)
{
	FILE* fp;
	if ((fp = fopen(file, mode)) == NULL)
		perror(file);
	return fp;
}

void prepareSearchString(char* p, char* buf) {
	register int c;
	register int i = 0;
	if (*p == '^') {
		buf[i++] = ANCHOR;
		++p;
	}
	for (;;) {
		switch (c = *p++) {
		case EOS: goto Exit;
		case '*': if (i >= 0 && buf[i - 1] != STAR) c = STAR; break;
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

BOOL patternMatch(char* pattern, char* string) {
	register char pc, sc;
	char* pat;
	BOOL anchored;
	if (anchored = (*pattern == ANCHOR))
		++pattern;

Top:
	pat = pattern;

Again:
	pc = *pat;
	sc = *string;

	if (sc == '\n' || sc == EOS) {
		if (pc == EOS) {
			goto Success;
		}
		else if (pc == STAR) {
			++pat;
			goto Again;
		}
		else {
			return (FALSE);
		}
	}
	else {
		if (pc == sc || pc == QM) {
			++pat;
			++string;
			goto Again;
		}
		else if (pc == EOS) {
			goto Success;
		}
		else if(pc == STAR){
			if (patternMatch(pat + 1, string)) goto Success;
			else {
				++string;
				goto Again;
			}
		}else if(pc == BEGCLASS){
			BOOL clmatch = FALSE;
			while (*++pat != ENDCLASS)
			{
				if (!clmatch && *pat == sc) clmatch = TRUE;
			}
			if(clmatch){
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
void main(int argc, char** argv)
{
	int i, patternSeen = FALSE, showName= FALSE, result = 1;
	BOOL ignoreCase = FALSE;
	char pattern[256];
	char string[2048];
	FILE* fp;

	char* mod = (char*)malloc(4);
	strcpy(mod, "r");

	fp = openFile(argv[2], mod);
	if (fp == NULL) {
		perror("File Open Failed.!");
		fputs("", stdout);
		fputc('\0', stdout);
		exit(1);
	}

	if (argc < 1) {
		puts("Usage : grep pattern file(s)");
		exit(2);
	}

	for (i = 1; i < argc + 1; i++) {
		if (i < argc && argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'y':
				ignoreCase = TRUE;
				break;
			}
		}else{
			if (!patternSeen++)
				prepareSearchString(argv[i], pattern);
			else if (/*(fp = openFile(file=argv[i],"rb")) != NULL */ TRUE) {
				if (!showName && i < argc - 1) ++showName;
				while (fgets(string, sizeof(string), fp) != NULL && !feof(fp)) {
					if (ignoreCase) _strlwr(string);//将字符串中的S参数转换为小写形式
					if (patternMatch(pattern, string)) {
						result = 0;
						if (showName) {
							fputs(string, stdout);
						}
						else {
							fputs(string, stdout);
						}
					}
				}
			}
		
		}
	}
	fputc('\n', stdout);
	exit(result);
}