#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include "acquisitionManager.h"
#include "msg.h"
#include "iSensor.h"
#include "multitaskingAccumulator.h"
#include "iAcquisitionManager.h"
#include "debug.h"

// producer count storage
volatile unsigned int produceCount = 0;

pthread_t producers[4];

static void *produce(void *params);

/**
 * Semaphores and Mutex
 */
// TODO
//  #####################################################################
#define N 8 // buffer size
static MSG_BLOCK Buffer[N];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER, mutex2 = PTHREAD_MUTEX_INITIALIZER;
sem_t *Socc, *Slib;
// #####################################################################

/*
 * Creates the synchronization elements.
 * @return ERROR_SUCCESS if the init is ok, ERROR_INIT otherwise
 */
static unsigned int createSynchronizationObjects(void);

/*
 * Increments the produce count.
 */
static void incrementProducedCount(void);

static unsigned int createSynchronizationObjects(void)
{

	// TODO
	//  #####################################################################
	Socc = sem_open("Socc", O_CREAT, 0644, 0);
	Slib = sem_open("Socc", O_CREAT, 0644, N);
	// #####################################################################

	printf("[acquisitionManager]Semaphore created\n");
	return ERROR_SUCCESS;
}

static void incrementProducedCount(void)
{
	// TODO
	//  #####################################################################
	produceCount++;
	//  #####################################################################
}

unsigned int getProducedCount(void)
{
	unsigned int p = 0;
	// TODO
	// #####################################################################
	p = produceCount;
	// #####################################################################
	return p;
}

MSG_BLOCK getMessage(void)
{
	// TODO
	//  #####################################################################
	// Mono-read
	MSG_BLOCK msg;
	static u_int8_t iocc;
	sem_wait(Socc);
	msg = Buffer[iocc];
	iocc = (iocc + 1) % N;
	sem_post(Slib);
	return msg;
	// #####################################################################
}

// TODO create accessors to limit semaphore and mutex usage outside of this C module.

unsigned int acquisitionManagerInit(void)
{
	unsigned int i;
	printf("[acquisitionManager]Synchronization initialization in progress...\n");
	fflush(stdout);
	if (createSynchronizationObjects() == ERROR_INIT)
		return ERROR_INIT;

	printf("[acquisitionManager]Synchronization initialization done.\n");

	for (i = 0; i < PRODUCER_COUNT; i++)
	{
		// TODO
		//  #####################################################################
		pthread_create(&producers[i], NULL, produce, NULL);
		//  #####################################################################
	}

	return ERROR_SUCCESS;
}

void acquisitionManagerJoin(void)
{
	unsigned int i;
	for (i = 0; i < PRODUCER_COUNT; i++)
	{
		// TODO
		//  #####################################################################
		pthread_join(producers[i], NULL);
		//  #####################################################################
	}

	// TODO
	//  #####################################################################
	sem_destroy(Slib);
	sem_destroy(Socc);
	//  #####################################################################
	printf("[acquisitionManager]Semaphore cleaned\n");
}

//  #####################################################################
extern pid_t gettid(void); // just to remove one warning
//  #####################################################################

void *produce(void *params)
{
	D(printf("[acquisitionManager]Producer created with id %d\n", gettid()));
	unsigned int i = 0;
	while (i < PRODUCER_LOOP_LIMIT)
	{
		i++;
		sleep(PRODUCER_SLEEP_TIME + (rand() % 5));
		// TODO
		//  #####################################################################
		// Multi-write (not yet)
		static u_int8_t ilib;
		sem_wait(Slib);
		MSG_BLOCK msg;
		getInput(7, &msg);
		Buffer[ilib] = msg;
		ilib = (ilib + 1) % N;
		incrementProducedCount();
		sem_post(Socc);

		//  #####################################################################
	}
	printf("[acquisitionManager] %d termination\n", gettid());
	// TODO
	//  #####################################################################
	pthread_exit(NULL);
	//  #####################################################################
}