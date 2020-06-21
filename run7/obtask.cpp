/*
grep pattern files;
Looks for pattern in files. a file is scanned loine by line;
**** 过滤ob取到的任务数及时间
1 模式匹配selectOutboundList和活动号,记住threadid  
2 匹配threadid 和length()= ,  判断 length > 0 时输出当前行
3 length 为取到的任务数；
	
obtask   [活动号]  ob*.log 
改进方向：
	目前1次只能搜索1个log文件，还不能模糊匹配多文件。

*/
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
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

	if (argc < 3) {
		puts("Usage : obtask pattern file(s)");
		exit(2);
	}

	int i, patternSeen = FALSE, showName= FALSE, taskSepcified = FALSE,result = 1;
	BOOL ignoreCase = FALSE;
	BOOL needLenSize = FALSE;
	char pattern[256];
	char taskID[56];
	char jobs[13] = "r.length()";
	char threadID[16];
	ZeroMemory(threadID,16);
	char string[2048],request[2048];

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
		//	else if(!taskSepcified++)
		//		prepareSearchString(argv[i], taskID);
			else if (/*(fp = openFile(file=argv[i],"rb")) != NULL */ TRUE) {
				if (!showName && i < argc - 1) ++showName;
				while (fgets(string, sizeof(string), fp) != NULL && !feof(fp)) {
					if (ignoreCase) _strlwr(string);//将字符串中的S参数转换为小写形式
					//匹配到 selectOutboundList 且r.length()>0 
					if(needLenSize && patternMatch(jobs,string) && patternMatch(threadID,string)){
						 	fputs(request,stdout);
							fputs(string, stdout);
						ZeroMemory(threadID,16);
						needLenSize = FALSE;
					}
					if (patternMatch(pattern, string) ) {//匹配到 
						result = 0;
						needLenSize = TRUE;
						//fputs(string, stdout); 
						strncpy(threadID,string+48,4);
						strcpy(request,string);
					}
				}
			}
		
		}
	}
	fputc('\n', stdout);
	exit(result);
}