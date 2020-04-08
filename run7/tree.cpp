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

int main( int argc ,char** argv )
{
   int fh, bytesread;
   FILE* fo;
   unsigned int nbytes = 60000;
   char* filename = "rec.log";
   if(argc > 1){
	   strcpy(filename,argv[1]);
	   printf("read file %s \n",filename);
   }
   /* Open file for input: */
   if ( _sopen_s( &fh, filename, _O_RDONLY, _SH_DENYNO, 0 ))
   {
      perror( "open failed on input file" );
      exit( 1 );
   }
   if ( (fo = fopen(  "fileout.log","w")) == NULL)
   {
      perror( "open failed on input file" );
      exit( 1 );
   }
   /* Read in input: */
   int line = 0,ch=0;
   while (( bytesread = _read( fh, buffer, nbytes )) > 0 ){   
	   for(int k= 0 ; k < bytesread; k++){
		   if(buffer[k] == '\n') line++;
	   }
	   if(line >= 2383378 && line <= 2921371){
		   fwrite(buffer,1,bytesread,fo);
	   }
   }
   printf("total lines %d \n",line);

   _close( fh );
  fclose(fo);
}