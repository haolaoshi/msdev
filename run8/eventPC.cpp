#include <Windows.h>
#include <tchar.h>
#include <process.h>
#include <time.h>
#include <stdio.h>


#define DATA_SIZE 256

typedef struct MSG_BLOCK_TAG {
	HANDLE mGuard;
	HANDLE mReady;
	DWORD  fReady, fStop;
	volatile DWORD nCons, mSequence;
	DWORD nLost;
	time_t mTimestamp;
	DWORD mChecksum; 
	DWORD mData[DATA_SIZE];
} MSG_BLOCK;

MSG_BLOCK mBlock = { 0, 0, 0, 0 ,0 };
DWORD WINAPI Produce(void*);
DWORD WINAPI Consume(void*);
void MessageFill(MSG_BLOCK*);
void MessageDisplay(MSG_BLOCK*);


DWORD _tmain(DWORD argc, LPTSTR argv[])
{
	HANDLE hProcude, hConsume;
	mBlock.mGuard = CreateMutex(NULL, FALSE, NULL);
	mBlock.mReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	hProcude = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)Produce, NULL, 0, NULL);
	hConsume = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)Consume, NULL, 0, NULL);
	WaitForSingleObject(hConsume, INFINITE);
	WaitForSingleObject(hProcude, INFINITE);

	CloseHandle(mBlock.mGuard);
	CloseHandle(mBlock.mReady);
	_tprintf(_T("Producer and Consumer threads have terminated.\n"));
	_tprintf(_T("Message Produces %d , Consumed %d , Known Lost %d\n"), mBlock.mSequence, mBlock.nCons, mBlock.mSequence - mBlock.nCons);
	return 0;
}

DWORD WINAPI Produce(void* arg)
{
	srand((DWORD)time(NULL));
	while (!mBlock.fStop) {
		Sleep(rand() / 5);
		WaitForSingleObject(mBlock.mGuard, INFINITE);
		__try {
			if (!mBlock.fStop) {
				mBlock.fReady = 0;
				MessageFill(&mBlock);
				mBlock.fReady = 1;
				InterlockedIncrement(&mBlock.mSequence);
				SetEvent(mBlock.mReady);
			}
		}
		__finally {
			ReleaseMutex(mBlock.mGuard);
		}
	}
	return 0;
}


DWORD WINAPI Consume(void* arg)
{
	DWORD ShutDown = 0;
	CHAR command[10];
	while (!ShutDown) {
		_tprintf(_T("\n**Enter 'c' for Consume ; 's' to stop : "));
		_tscanf(_T("%s"), command);
		if (command[0] == 's') {
			WaitForSingleObject(mBlock.mGuard, INFINITE);
			ShutDown = mBlock.fStop - 1;
			ReleaseMutex(mBlock.mGuard);
		}
		else if (command[0] == 'c') {
			WaitForSingleObject(mBlock.mReady, INFINITE);
			WaitForSingleObject(mBlock.mGuard, INFINITE);
			__try {
				if (!mBlock.fReady) __leave;
				MessageDisplay(&mBlock);
				InterlockedIncrement(&mBlock.nCons);
				mBlock.nLost = mBlock.mSequence - mBlock.nCons;
				mBlock.fReady = 0;
			}
			__finally {
				ReleaseMutex(mBlock.mGuard);
			}
		}
		else {
			_tprintf(_T("Illegal command . Try agaon\n"));
		}
	}
	return 0;
}
void MessageFill(MSG_BLOCK* msgBlock)
{
	DWORD i;
	msgBlock->mChecksum = 0;
	for (i = 0; i < DATA_SIZE; i++) {
		msgBlock->mData[i] = rand();
		msgBlock->mChecksum ^= msgBlock->mData[i];
	}
	msgBlock->mTimestamp = time(NULL);
	return;
}

void MessageDisplay(MSG_BLOCK* msgBlock)
{
	DWORD i, tcheck = 0;
	for (i = 0; i < DATA_SIZE; i++)
		tcheck ^= msgBlock->mData[i];
	_tprintf(_T("\nMessage number %d generated at : %s "), msgBlock->mSequence, _tctime(&(msgBlock->mTimestamp)));
	_tprintf(_T("First and laster entries: T%x %x \n"), msgBlock->mData[0], msgBlock->mData[DATA_SIZE - 1]);
	if (tcheck == msgBlock->mChecksum)
		_tprintf(_T("GOOD -> checksum was valid.\n"));
	else
		_tprintf(_T("BAD -> checksum filed . message corrupted\n"));

	return;
}