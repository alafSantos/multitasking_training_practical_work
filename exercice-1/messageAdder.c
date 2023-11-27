#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
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

//  #####################################################################
static sem_t *Socc;
static sem_t *Slib;
//  #####################################################################

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
	sem_wait(Socc);
	p = out;
	sem_post(Slib);
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

	// On supprime les sémaphores si ceux-ci existent
	sem_unlink("SoccMA");
	sem_unlink("SlibMA");
	// On créent les sémaphores qu'on désire
	Socc = sem_open("SoccMA", O_CREAT, 0644, 0);
	Slib = sem_open("SlibMA", O_CREAT, 0644, 1);
	// On vérifie si ils sont ouverts
	if ((Socc == SEM_FAILED) || (Slib == SEM_FAILED))
	{
		perror("[sem_open]");
		return;
	}

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
	MSG_BLOCK tmp1, tmp2;
	// #####################################################################
	D(printf("[messageAdder]Thread created for sum with id %d\n", gettid()));
	unsigned int i = 0;
	while (i < ADDER_LOOP_LIMIT)
	{

		// TODO
		// #####################################################################
		tmp1 = getMessage();
		incrementConsumeCount();
		i++;
		for (int k = 0; k < 3; k++)
		{
			sleep(ADDER_SLEEP_TIME);
			tmp2 = getMessage();
			messageAdd(&tmp1, &tmp2);
			incrementConsumeCount();
			i++;
		}

		sem_wait(Slib);
		out = tmp1;
		sem_post(Socc);
		// #####################################################################
	}
	printf("[messageAdder] %d termination\n", gettid());
	// TODO
	//  #####################################################################
	pthread_exit(NULL);
	// #####################################################################
}
