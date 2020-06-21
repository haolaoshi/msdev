// crt_read.c
/* This program opens a file named crt_read.txt
* and tries to read 60,000 bytes from
* that file using _read. It then displays the
* actual number of bytes read.
*/

#include <fcntl.h>      /* Needed only for _O_RDWR definition */
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <share.h>
#include <string.h>

char buffer[60000];
/*
Begin_line and end_line are ready to intercept the start and end lines of the tmp file. 
These two parameters can also be proportional, for example. 0.3 and 0.7 means to extract 
the text content about 30% to 70% of the total number of lines.

*/
int main( int argc ,char** argv )
{
	int fh, bytesread;
	FILE* fo;
	unsigned int nbytes = 60000;


	if(argc != 4 ){

		int iangle = 100; while(iangle--) printf("<");
		printf("\n");
		printf("Usage :  extractf   xxxx.log   begin_line   end_line \n");
		printf("\tBegin_line and end_line are ready to intercept the start and end lines of the tmp file.\n");
		printf("\tThese two parameters can also be proportional, for example. 0.3 and 0.7 means to extract \n");
		printf("\t  the text content about 30% to 70% of the total number of lines.\n");
		int i = 100; while(i--) printf("<");
		printf("\n");

		exit(1);
	}
	/* Open file for input: */
	if ( _sopen_s( &fh, argv[1], _O_RDONLY, _SH_DENYNO, 0 ))
	{
		perror( "open failed on input file" );
		exit( 1 );
	}

	char tempFile[256];
	sprintf(tempFile,"%s.tmp",argv[1]);
	if ( (fo = fopen( tempFile,"w")) == NULL)
	{
		perror( "open failed on input file" );
		exit( 1 );
	}

	if( argc != 4 ){
		perror(" tree file  from  to ");
		exit(1);
	}
	double from = atof(argv[2]);
	double to = atof(argv[3]);
	                      
	FILE *fp = fopen(argv[1],"r");
	if(fp == NULL){
		perror("cannot open file \n");
		exit(1);
	}
	char c,m;
	int n = 0;
	while((c = fgetc(fp)) != EOF ){
		if (c == '\n') n++;
		m = c;
	}
	fclose(fp);
	if(m != '\n') n++;

	if(from <= 1 && to <= 1){
		from = from * n;
		to = to * n;

	}
	else if(from <=1 || to <= 1){
		perror("parameter not allowed!");
		exit(1);
	}

	int a = (int)from ;
	int b = (int)to;
	/* Read in input: */
	int line = 0,ch=0;
	while (( bytesread = _read( fh, buffer, nbytes )) > 0 ){   
		for(int k= 0 ; k < bytesread; k++){
			if(buffer[k] == '\n') line++;
		}
		if(line >= a && line <= b){
			fwrite(buffer,1,bytesread,fo);
		}
		if(line > b) break;
	}
	printf("extract file from line  %d  to line %d  (total %d )\n",a, b, n);

	_close( fh );
	fclose(fo);

}