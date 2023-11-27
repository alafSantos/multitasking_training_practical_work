#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include "messageAdder.h"
#include "msg.h"
#include "iMessageAdder.h"
#include "multitaskingAccumulator.h"
#include "iAcquisitionManager.h"
#include "debug.h"

//  #####################################################################
//  extern pid_t gettid(void); // just to remove one warning
//  #####################################################################

// consumer thread
pthread_t consumer;
// Message computed
volatile MSG_BLOCK out;
// Consumer count storage
volatile unsigned int consumeCount = 0;

// Mutex protégeant produceCount
pthread_mutex_t mconscount = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t msum = PTHREAD_MUTEX_INITIALIZER;

/**
 * Increments the consume count.
 */
static void incrementConsumeCount(void);

/**
 * Consumer entry point.
 */
static void *sum(void *parameters);

MSG_BLOCK getCurrentSum()
{
	// TODO
	//  #####################################################################
	MSG_BLOCK p;
	pthread_mutex_lock(&msum);
	p = out;
	pthread_mutex_unlock(&msum);
	return p;
	// #####################################################################
}

unsigned int getConsumedCount()
{
	// TODO
	//  #####################################################################
	unsigned int p;
	pthread_mutex_lock(&mconscount);
	p = consumeCount;
	pthread_mutex_unlock(&mconscount);
	return p;
	// #####################################################################
}

void messageAdderInit(void)
{
	out.checksum = 0;
	for (size_t i = 0; i < DATA_SIZE; i++)
	{
		out.mData[i] = 0;
	}
	// TODO
	//  #####################################################################
	pthread_create(&consumer, NULL, sum, NULL);
	// #####################################################################
}

void messageAdderJoin(void)
{
	// TODO
	//  #####################################################################
	pthread_join(consumer, NULL);
	// #####################################################################
}

//  #####################################################################
static void incrementConsumeCount(void)
{
	pthread_mutex_lock(&mconscount);
	consumeCount++;
	pthread_mutex_unlock(&mconscount);
}
//  #####################################################################

static void *sum(void *parameters)
{
	// #####################################################################
	// Init hors de la boucle pour éviter d'avoir à recréer
	MSG_BLOCK tmp;
	// #####################################################################
	D(printf("[messageAdder]Thread created for sum with id %d\n", gettid()));
	unsigned int i = 0;
	while (i < ADDER_LOOP_LIMIT)
	{
		i++;
		sleep(ADDER_SLEEP_TIME);
		// TODO
		// #####################################################################
		tmp = getMessage();
		pthread_mutex_lock(&msum);
		messageAdd(&out, &tmp);
		pthread_mutex_unlock(&msum);
		incrementConsumeCount();
		// #####################################################################
	}
	printf("[messageAdder] %d termination\n", gettid());
	// TODO
	//  #####################################################################
	pthread_exit(NULL);
	// #####################################################################
}
