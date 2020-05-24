#include <windows.h>
#include <tchar.h>

#include <stdio.h>
/*
https://docs.microsoft.com/en-us/windows/win32/fileio/appending-one-file-to-another-file
Appending one file to another 

*/
VOID _tmain(int argc, LPTSTR argv[])
{
    HANDLE hFile;
    HANDLE hAppend;
    DWORD  dwBytesRead, dwBytesWritten, dwPos;
    BYTE   buff[4096];

    if (argc < 3) {
        printf("usage : concatf read_file  append_to_file");
        return ;
    }
    // Open the existing file.

    hFile = CreateFile(argv[1], // open One.txt
        GENERIC_READ,             // open for reading
        0,                        // do not share
        NULL,                     // no security
        OPEN_EXISTING,            // existing file only
        FILE_ATTRIBUTE_NORMAL,    // normal file
        NULL);                    // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Could not open %s.") , argv[1]);
        return;
    }

    // Open the existing file, or if the file does not exist,
    // create a new file.

    hAppend = CreateFile(argv[2], // open Two.txt
        FILE_APPEND_DATA,         // open for writing
        FILE_SHARE_READ,          // allow multiple readers
        NULL,                     // no security
        OPEN_ALWAYS,              // open or create
        FILE_ATTRIBUTE_NORMAL,    // normal file
        NULL);                    // no attr. template

    if (hAppend == INVALID_HANDLE_VALUE)
    {
        _tprintf(_T("Could not open %s."), argv[2]);
        return;
    }

    // Append the first file to the end of the second file.
    // Lock the second file to prevent another process from
    // accessing it while writing to it. Unlock the
    // file when writing is complete.

    while (ReadFile(hFile, buff, sizeof(buff), &dwBytesRead, NULL)
        && dwBytesRead > 0)
    {
        dwPos = SetFilePointer(hAppend, 0, NULL, FILE_END);
        LockFile(hAppend, dwPos, 0, dwBytesRead, 0);
        WriteFile(hAppend, buff, dwBytesRead, &dwBytesWritten, NULL);
        UnlockFile(hAppend, dwPos, 0, dwBytesRead, 0);
    }

    // Close both files.

    CloseHandle(hFile);
    CloseHandle(hAppend);
}