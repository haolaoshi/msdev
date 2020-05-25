#include "SynchObj.h"
#include "messages.h""
#include <time.h>

#define DELAY_COUNT 1000
#define MAX_THREADS 1024

#define TBLOCK_SIZE 5
#define TBLOCK_TIMEOUT 50
#define P2T_QLEN 10
#define T2R_QLEN 4
#define R2C_QLEN 4

DWORD WINAPI Producer(PVOID);
DWORD WINAPI Consumer(PVOID);
DWORD WINAPI Transmitter(PVOID);
DWORD WINAPI Receiver(PVOID);

typedef struct THARG_TAG {
	DWORD threadNumber;
	DWORD workGoal;
	DWORD workDone;
}THARG;

typedef struct T2R_MSG_OBJ_ARG {
	DWORD numMessages;
	MSG_BLOCK messages[TBLOCK_SIZE];
}T2R_MSG_OBJ;

typedef struct TR_ARG_TAG {
	DWORD nProducers;
}TR_ARG;


QUEUE_OBJECT p2tq, t2rq, * r2cqArray;
static volatile DWORD ShutDown = 0;
static DWORD EventTimeOut = 50;
DWORD trace = 0;

DWORD main(DWORD argc, char* argv[]) {
	DWORD tStatus, nThread, iThread, goal;
	HANDLE* produceThreadArray, * consumerThreadArray, transmitterThread, receiverThread;
	THARG* producerArg, * consumerArg;
	TR_ARG transmitterArg, receiverArg;

	srand((int)time(NULL));
	nThread = atoi(argv[1]);
	receiverArg.nProducers = transmitterArg.nProducers = nThread;
	goal = atoi(argv[2]);
	if (argc >= 4) trace = atoi(argv[3]);


	produceThreadArray = malloc(nThread * sizeof(HANDLE));
	producerArg = calloc(nThread, sizeof(THARG));
	consumerThreadArray = malloc(nThread * sizeof(HANDLE));
	consumerArg = calloc(nThread, sizeof(THARG));

	QueueInitialize(&p2tq, sizeof(MSG_BLOCK), P2T_QLEN);
	QueueInitialize(&t2rq, sizeof(T2R_MSG_OBJ), T2R_QLEN);

	r2cqArray = calloc(nThread, sizeof(QUEUE_OBJECT));

	for (iThread = 0; iThread < nThread; iThread++) {
		QueueInitialize(&r2cqArray[iThread], sizeof(MSG_BLOCK), R2C_QLEN);
		consumerArg[iThread].threadNumber = iThread;
		consumerArg[iThread].workGoal = goal;
		consumerArg[iThread].workDone = 0;
		produceThreadArray[iThread] = (HANDLE)_beginthreadex(NULL, 0, Consumer, &consumerArg[iThread], 0, NULL);
		producerArg[iThread].threadNumber = iThread;
		producerArg[iThread].workDone = 0;
		producerArg[iThread].workGoal = goal;
		produceThreadArray[iThread] = (HANDLE)__beginthreadex(NULL, 0, Producer, &producerArg[iThread], 0, NULL);

	}
	transmitterThread = (HANDLE)_beginthreadex(NULL, 0, Transmitter, &transmitterArg, 0, NULL);
	receiverThread = (HANDLE)_beginthreadex(NULL, 0, Receiver, &receiverArg, 0, NULL);

	_tprintf(_T("BOOS : ALL thread are running."));

	for (iThread 0; iThread < nThread; iThread++) {
		WaitForSingleObject(produceThreadArray[iThread], INFINITE);
		_tprintf(_T("BOOS producer  % produced . %d work units\n"), iThread, producerArg[iThread].workDone);

	}

	_tprintf(_T("BOOS : ALL PRODUCERS have completed their workd.\n"));

	for (iThread = 0; iThread < nThread; iThread++)
	{
		WaitForSingleObject(consumerThreadArray[iThread], INFINITE);
		_tprintf(_T("boos ; consumer %d consumerd %d work units\n"), iThread, consumerArg[iThread].workDone);
	}
	_tprintf(_T("boos : all consumers have completed their work\n"));
	ShutDown = 1;
	WaitForSingleObject(transmitterThread, INFINITE);
	WaitForSingleObject(receiverThread, INFINITE);
	QueueDestroy(&p2tq);
	QueueDestroy(&t2rq);

	for (iThread = 0; iThread < nTrhead;iThread++) {
		QueueDestroy(&r2cqArray[iThread]);
		CloseHandle(consumerThreadArray[iThread]);
		CloseHandle(produceThreadArray[iThread]);
	}

	free(r2cqArray);
	free(produceThreadArray);
	free(consumerThreadArray);
	free(producerArg);
	free(consumerArg);
	CloseHandle(transmitterThread);
	CloseHandle(receiverThread);
	_tprintf(_T("system has finished. shutting down\n"));
	return 0;
}


DWORD WINAPI Producer(PVOID arg)
{
	THARG* parg;
	DWORD iThread;
	MSG_BLOCK msg;

	parg = (THARG*)arg;
	iThread = parg->threadNumber;
	while (parg->workDone < parg->workGoal) {
		delay_cpu(DELAY_COUNT * rand() / RAND_MAX);
		MessageFill(&msg, iThread, iThread, parg->workDone);
		QueuePut(&p2tq, &msg, sizeof(msg), INFINITE);
		parg->workDone++;
	}

	MessageFill(&msg, iThread, iThread, -1);
	QueuePut(&p2tq, &msg, sizeof(msg), INFINITE);
	return 0;
}

DWORD WINAPI Transmitter(PVOID arg) {

}

DWORD WINAPI Receiver(PVOID arg)
{

}

DWORD WINAPI Consumer(PVOID arg)
{

}