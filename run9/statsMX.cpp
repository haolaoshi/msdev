#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <process.h>


#define DELAY_COUNT		20
#define CACHE_LINE_SIZE	64

DWORD WINAPI Worker(void*);

__declspec(align(CACHE_LINE_SIZE))
typedef struct _THARG {
	HANDLE hMutex;
	int threadNumber;
	unsigned int tasksToComplete;
	unsigned int tasksComplete;
} THARG;

int _tmain(DWORD argc, LPTSTR argv[])
{
	INT nThread, iThread;
	HANDLE* hWorkers, hMutex;
	unsigned int tasksPerThread, totalTasksComplete;
	THARG** pThreadArgsArray, * pThreadArg;

	nThread = _ttoi(argv[1]);
	tasksPerThread = _ttoi(argv[2]);

	hMutex = CreateMutex(NULL, FALSE, NULL);
	hWorkers = (HANDLE*)malloc(nThread * sizeof(HANDLE));
	pThreadArgsArray = (THARG**)malloc(nThread * sizeof(THARG*));

	for (iThread = 0; iThread < nThread; iThread++) {
		pThreadArg = (pThreadArgsArray[iThread] = (THARG*)_aligned_malloc(sizeof(THARG), CACHE_LINE_SIZE));
		pThreadArg->threadNumber = iThread;
		pThreadArg->tasksToComplete = tasksPerThread;
		pThreadArg->tasksComplete = 0;
		pThreadArg->hMutex = hMutex;
		hWorkers[iThread] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)Worker, pThreadArg, 0, NULL);

	}

	for (iThread = 0; iThread < nThread; iThread++)
		WaitForSingleObject(hWorkers[iThread], INFINITE);

	CloseHandle(hMutex);

	free(hWorkers);
	_tprintf(_T("Worker threads have teminators.\n"));

	totalTasksComplete = 0;

	for (iThread = 0; iThread < nThread; iThread++) {
		pThreadArg = pThreadArgsArray[iThread];
		_tprintf(_T("Tasks complete  by thead %5d : %5d \n"), iThread, pThreadArg->tasksComplete);
		totalTasksComplete += pThreadArg->tasksComplete;
		_aligned_free(pThreadArg);
	}
	_tprintf(_T("total work perfored: %d.\n"), totalTasksComplete);
	free(pThreadArgsArray);
	return 0;
}

DWORD WINAPI Worker(void* arg)
{
	THARG* threadArgs;
	int iThread;
	threadArgs = (THARG*)arg;
	iThread = threadArgs->threadNumber;

	while (threadArgs->tasksComplete < threadArgs->tasksToComplete) {
		delay_cpu(DELAY_COUNT);
		WaitForSingleObject(threadArgs->hMutex, INFINITE);
		(threadArgs->tasksComplete)++;
		ReleaseMutex(threadArgs->hMutex);

	}
	return 0;
}