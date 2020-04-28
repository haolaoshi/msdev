#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>
#include <process.h>


#define DATA_SIZE 256

typedef struct MSG_BLOCK_TAG {
	CRITICAL_SECTION mGuard;
	DWORD fReady, fStop;
	volatile DWORD nCons, mSequence;
	DWORD nLost;
	time_t mTimestamp;
	DWORD mChecksum; 
	DWORD mData[DATA_SIZE];

}MSG_BLOCK;

MSG_BLOCK mBlock = { 0, 0, 0, 0 ,0 };
DWORD WINAPI Produce(void*);
DWORD WINAPI Consume(void*);
void MessageFill(MSG_BLOCK*);
void MessageDisplay(MSG_BLOCK*);

DWORD _tmain(DWORD argc, LPTSTR argv[])
{
	DWORD status;
	HANDLE hProduce, hConsume;
	InitializeCriticalSection(&mBlock.mGuard);

	hProduce = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)Produce, NULL, 0, NULL);
	hConsume = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)Consume, NULL, 0, NULL);

	status = WaitForSingleObject(hConsume,INFINITE);
	status = WaitForSingleObject(hProduce, INFINITE);

	DeleteCriticalSection(&mBlock.mGuard);
	_tprintf(_T("Producer and Consumer threads have terminated\n"));
	_tprintf(_T("Message producesd : %d , Consumed : %d , Lost : %d .\n"),
		mBlock.mSequence, mBlock.nCons, mBlock.mSequence - mBlock.nCons);

	return 0;
}

DWORD WINAPI Produce(void* p)
{
	srand((DWORD)time(NULL));
	while (!mBlock.fStop) {
		Sleep(rand() / 100);
		EnterCriticalSection(&mBlock.mGuard);
		__try {
			if (!mBlock.fStop) {
				mBlock.fReady = 0;
				MessageFill(&mBlock);
				mBlock.fReady = 1;
				InterlockedIncrement(&mBlock.mSequence);
			}
		}
		__finally {
			LeaveCriticalSection(&mBlock.mGuard);
		}
	}
	/* 
	
	DWORD* mess = (DWORD*)p;
	printf("线程%d ，生产者\n", GetCurrentThreadId);
	TCHAR threadId[20];
	//_stprintf_s(commandLine,_T("grep \"%s\"  \"%s\""),argv[1],argv[iProc + 2]);
	_stprintf_s(threadId,20, L"线程%d ,生产者", GetCurrentThreadId());
	MessageBox(0, (LPCWSTR)mess, (LPCWSTR)threadId, 0);
	*/

	return 0;

}
DWORD WINAPI Consume(void* p)
{
	CHAR command, extra;
	while (!mBlock.fStop) {
		_tprintf(_T("\n **Enter 'c' for Consume; 's' to stop: "));
		_tscanf(_T("%c%c"), &command, &extra);
		if (command == 's') {
			mBlock.fStop = 1;
		}
		else if (command == 'c')
		{
			EnterCriticalSection(&mBlock.mGuard);
			__try {
				if (mBlock.fReady == 0)
					_tprintf(_T("No new messages . Try again later \n"));
				else {
					MessageDisplay(&mBlock);
					mBlock.nLost = mBlock.mSequence - mBlock.nCons;
					mBlock.fReady = 0;
					InterlockedIncrement(&mBlock.nCons);
				}
			}
			__finally {
				LeaveCriticalSection(&mBlock.mGuard);
			}
		}
		else {
			_tprintf(_T("illegal command . Try again\n"));
		}
	}
	/*
	DWORD* mess = (DWORD*)p;
	printf_s("线程 %d,消费者\n",GetCurrentThreadId());
	TCHAR threadid[20];
	_stprintf_s(threadid, L"线程%d ,消费者", GetCurrentThreadId());
	MessageBox(0, (LPCWSTR)mess, (LPCWSTR)threadid, 0);
	*/

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