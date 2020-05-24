#include <Windows.h>
#include <tchar.h>
#include <process.h>


#define DELAY_COUNT 20
#define CACHE_LINE_SIZE 64
#define delay_cpu(n)  {\
	int i=0, j=0;\
	/* Do some wasteful computations that will not be optimized to nothing */\
	while (i < n) {\
		j = (int)(i*i + (float)(2*j)/(float)(i+1));\
		i++;\
	}\
}


VOID CALLBACK  Worker(PTP_CALLBACK_INSTANCE, PVOID, PTP_WORK);
int workerDelay = DELAY_COUNT;
 
__declspec(align(CACHE_LINE_SIZE))
typedef struct _THARG {
	SRWLOCK SRWL;
	int threadNumber;
	unsigned int tasksToComplete;
	unsigned int tasksComplete;
} THARG;

int _tmain(DWORD argc, LPTSTR argv[])
{
	INT nThread, iThread;
	HANDLE* pWorkObjects;
	SRWLOCK srwl;
	unsigned int tasksPerThread, totalTasksComplete;
	THARG** pThreadArgsArray, * pThreadArg;
	TP_CALLBACK_ENVIRON cbe;//call back envroment
	BOOL traceFlag = FALSE;

	nThread = _ttoi(argv[1]);
	tasksPerThread = _ttoi(argv[2]);

	InitializeSRWLock(&srwl);
	pWorkObjects = (HANDLE * )malloc(nThread * sizeof(PTP_WORK));
	pThreadArgsArray = (THARG * *)malloc(nThread * sizeof(THARG*));

	InitializeThreadpoolEnvironment(&cbe);
	for (iThread = 0; iThread < nThread; iThread++) {
		pThreadArg = (pThreadArgsArray[iThread] = (THARG *)_aligned_malloc(sizeof(THARG), CACHE_LINE_SIZE));
		pThreadArg->threadNumber = iThread;
		pThreadArg->SRWL = srwl;
		pThreadArg->tasksToComplete = tasksPerThread;
		pThreadArg->tasksComplete = 0;
		pWorkObjects[iThread] = CreateThreadpoolWork(Worker, pThreadArg, &cbe);
		SubmitThreadpoolWork((PTP_WORK)pWorkObjects[iThread]);
	}

	for (iThread = 0; iThread < nThread; iThread++) {
		WaitForThreadpoolWorkCallbacks((PTP_WORK)pWorkObjects[iThread],FALSE);
		CloseThreadpoolWork((PTP_WORK)pWorkObjects[iThread]);
	}
	free(pWorkObjects);

	_tprintf(_T("Worker threads have terminated."));

	totalTasksComplete = 0;
	for (iThread = 0; iThread < nThread; iThread++) {
		pThreadArg = pThreadArgsArray[iThread];
		_tprintf(_T("Takss compelte by thred %5d: %6d\n"), iThread, pThreadArg->tasksComplete);
		totalTasksComplete += pThreadArg->tasksComplete;
		_aligned_free(pThreadArg);
	}
	free(pThreadArgsArray);

	_tprintf(_T("totla work performed. %d\n"), totalTasksComplete);
	return 0;
}

VOID CALLBACK Worker(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PTP_WORK Work)
{
	THARG* threadArgs = (THARG*)Context;
	while (threadArgs->tasksComplete < threadArgs->tasksToComplete) {
		delay_cpu(workerDelay);
		AcquireSRWLockExclusive(&(threadArgs->SRWL));
		(threadArgs->tasksComplete)++;
		ReleaseSRWLockExclusive(&(threadArgs->SRWL));
	}
	return;
}