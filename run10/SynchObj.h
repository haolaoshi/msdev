#include <Windows.h>
#include <tchar.h>
/**
** 10-1 Threshold Barrier Definitions
**/
typedef struct THRESHOLD_BARRIER_TAG {
	HANDLE bGuarg;
	HANDLE bEvent;
	DWORD bCount;
	DWORD bThreshold;
} THRESHOLD_BARRIER,*THB_OBJECT;

DWORD CreateThresholdBarrier(THB_OBJECT*, DWORD);
DWORD WaitThresholdBarrier(THB_OBJECT);
DWORD CloseThresholdBarrier(THB_OBJECT);

#define SYNCH_OBJ_MODEM	1
#define SYNCH_OBJ_CREATE_FAILURE	2
#define SYNCH_OBJ_BUSY 3

/* 10-3 Queue Definitions */
typedef struct QUEUE_OBJECT_TAG {
	HANDLE qGuard;
	HANDLE qNe;
	HANDLE qNf;
	DWORD qSize;
	DWORD qFirst;
	DWORD qLast;
	char* msgArray;
} QUEUE_OBJECT;

DWORD QueueInitialize(QUEUE_OBJECT*, DWORD, DWORD);
DWORD QueueDestroy(QUEUE_OBJECT*);
DWORD QueueEmpty(QUEUE_OBJECT*);
DWORD QueueFull(QUEUE_OBJECT*);
DWORD QueueGet(QUEUE_OBJECT*, PVOID, DWORD, DWORD);
DWORD QueuePut(QUEUE_OBJECT*, PVOID, DWORD, DWORD);
DWORD QueueRemove(QUEUE_OBJECT*, PVOID, DWORD);
DWORD QueueInsert(QUEUE_OBJECT*, PVOID, DWORD);


#define CV_TIMEOUT 10
