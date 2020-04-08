#include <Windows.h>
#include <tchar.h>

#include <stdio.h>
//program 1-3
//a system function style copy cmd
int _tmain(int argc, LPTSTR argv[])
{
	if (argc != 3) {

		printf("Usage : cpf file1 file2 \n");
		return 1;
	}

	if (!CopyFile(argv[1], argv[2], FALSE)) {
		printf("Copy File Error: %x\n", GetLastError());
		return 2;
	}

	return 0;
}