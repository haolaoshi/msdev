/* Chapter 5, sortBT command.   Binary Tree version. */

/* sort files
	Sort one or more files.
	This limited implementation sorts on the first field only.
	The key field length is assumed to be fixed length (8-characters).
	The data fields are varying length character strings. */

/* This program illustrates:
	1.	Multiple independent heaps; one for the sort tree nodes,
		the other for the records.
	2.	Using HeapDestroy to free an entire data structure in a single operation.
	3.	Structured exception handling to catch memory allocation errors. */

/* Technique:
	1.	Scan the input file, placing each key (which is fixed size)
		into the binary search tree and each record onto the data heap.
	2. 	Traverse the search tree and output the records in order.
	3. 	Destroy the heap and repeat for the next file.
*/

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
/*
	sortbt command.
	binary tree version.
*/
#define KEY_SIZE	8
VOID ReportError(LPCTSTR userMessage, DWORD exitCode, BOOL printfErrorMessage);


DWORD Options(int argc, LPTSTR argv[], LPCTSTR OptStr, ...);

typedef struct _TREENODE
{
	struct _TREENODE *Left,*Right;
	TCHAR key[KEY_SIZE];
	LPTSTR pData;
}TREENODE,*LPTNODE,**LPPTNODE;

#define NODE_SIZE	sizeof(TREENODE)
#define NODE_HEAP_ISIZE	0x8000
#define DATA_HEAP_ISIZE	0x8000
#define MAX_DATA_LEN	0x1000
#define TKEY_SIZE	KEY_SIZE * sizeof(TCHAR)
#define STATUS_FILE_ERROR	0xE0000001  /*Customer exception*/

#define EMPTY _T ("")
#define YES _T ("y")
#define NO _T ("n")
#define CR 0x0D
#define LF 0x0A
#define TSIZE sizeof (TCHAR)


LPTNODE FillTree(HANDLE , HANDLE,HANDLE);
BOOL Scan(LPTNODE);
int KeyCompare(LPCTSTR,LPCTSTR),iFile;
BOOL InsertTree(LPPTNODE,LPTNODE);

int _tmain(int argc,LPTSTR argv[])
{
	HANDLE hIn = INVALID_HANDLE_VALUE,hNode = NULL, hData = NULL;
	LPTNODE pRoot;
	BOOL NoPrint;
	CHAR errorMessage[256];
	int iFirstFile = Options(argc,argv,_T("n"),&NoPrint,NULL);
	/*Process al files on the command line.*/
	for(iFile = iFirstFile; iFile < argc; iFile++) __try{
		hIn = CreateFile(argv[iFile],GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
		if(hIn == INVALID_HANDLE_VALUE)
			RaiseException(STATUS_FILE_ERROR,0,0,NULL);
		__try{
			/*allocate two graowable heap.*/
			hNode = HeapCreate(HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,
				NODE_HEAP_ISIZE,0);
			hData = HeapCreate(HEAP_GENERATE_EXCEPTIONS|HEAP_NO_SERIALIZE,
				DATA_HEAP_ISIZE,0);
			/*PROCESS input file, creating the tree*/
			pRoot = FillTree(hIn,hNode,hData);
			if(!NoPrint){
				_tprintf(_T("Sorted file : %s \n"),argv[iFile]);
				Scan(pRoot);
			}
		}
		__finally{/*Heaps and file handles closed*/
			if(hNode != NULL) HeapDestroy(hNode);
			if(hData != NULL) HeapDestroy(hData);
			hNode = NULL;hData = NULL;
			if(hIn != INVALID_HANDLE_VALUE) CloseHandle(hIn);
			hIn = INVALID_HANDLE_VALUE;
		}
	}
	__except((GetExceptionCode() == STATUS_FILE_ERROR|| GetExceptionCode() == STATUS_NO_MEMORY) 
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH){
		_stprintf(errorMessage,_T("\n%s %s"),
			_T("sortbt error on file: "),argv[iFile]);
		ReportError(errorMessage,0,TRUE);
	}
	return 0;
}
/*fill the tree ,use the calling programs's exception handler*/
LPTNODE FillTree(HANDLE hIn,HANDLE hNode,HANDLE hData)
{
	LPTNODE pRoot = NULL,pNode;
	DWORD nRead,i;
	BOOL atCR;
	TCHAR dataHold[MAX_DATA_LEN];
	LPTSTR pString;

	while(TRUE){
		/*allocate and initiate a new tree node*/
		pNode = (LPTNODE)HeapAlloc(hNode , HEAP_ZERO_MEMORY,NODE_SIZE);
		if(!ReadFile(hIn,pNode->key,TKEY_SIZE,&nRead,NULL)||nRead != TKEY_SIZE)
			return pRoot;
		atCR = FALSE;/*READ DATA util end of line*/
		for(i = 0; i < MAX_DATA_LEN; i++){
			ReadFile(hIn,&dataHold[i],HTSIZE,&nRead,NULL);
			if(atCR && dataHold[i] == LF) break;
			atCR = (dataHold[i] == CR);
		}
		dataHold[i-1] = '\0';
		/*cOMBIN KEY AND DATA --insert in tree*/
		pString = (LPTSTR)HeapAlloc(hData,HEAP_ZERO_MEMORY,(SIZE_T)(KEY_SIZE + _tcslen(dataHold) + 1) * TSIZE);
		memcpy(pString,pNode->key,TKEY_SIZE);
		pString[KEY_SIZE] = '\0';
		_tcscat(pString,dataHold);
		pNode->pData = pString;
		InsertTree(&pRoot,pNode);
	}
	return NULL;
}

BOOL InsertTree(LPPTNODE ppRoot,LPTNODE pNode)
{
	/*Add a single node,with data, to the tree*/
	if(*ppRoot == NULL){
		*ppRoot = pNode;
		return TRUE;
	}
	if(KeyCompare(pNode->key,(*ppRoot)->key) < 0)
		InsertTree(&((*ppRoot)->Left),pNode);
	else
		InsertTree(&((*ppRoot)->Right),pNode);
}

static int KeyCompare(LPCTSTR pKey1,LPCTSTR pKey2)
	/*compare two records of generic characters.*/
{
	return _tcsncmp(pKey1,pKey2,KEY_SIZE);
}
static BOOL Scan(LPTNODE pNode)
	/*Recursively scan and print the contents of a binary tree*/
{
	if(pNode == NULL) return TRUE;
	Scan(pNode->Left);
	_tprintf(_T("%s\n"),pNode->pData);
	Scan(pNode->Right);
	return TRUE;
}