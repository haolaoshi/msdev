#include<Windows.h>
#include <stdio.h>

#define BUF_SIZE	256
//program 1-1
//a c-style copy cmd
int main(int argc, char* argv[])
{
	FILE* inFile, * outFile;
	char rec[BUF_SIZE];
	size_t bytesIn, bytesOut;
	if (argc != 3) {
		printf("Usage: cp file1 file2\n");
		return 1;
	}

	errno_t err  = fopen_s(&inFile,argv[1], "rb");
	if (err != 0) {
		perror(argv[1]);
		return 2;
	}

	err = fopen_s(&outFile,argv[2], "wb");
	if (err != 0) {
		perror(argv[2]);
		return 3;
	}
	while ((bytesIn = fread(rec, 1, BUF_SIZE, inFile)) > 0) {
		bytesOut = fwrite(rec, 1, bytesIn, outFile);
		if (bytesOut != bytesIn) {
			perror("Fatal write error");
			return 4;
		}
	}

	fclose(outFile);
	fclose(inFile);
	return 0;

}