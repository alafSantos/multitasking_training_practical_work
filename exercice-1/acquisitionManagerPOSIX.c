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
#define SEM_LIB_INITIAL_VALUE SIZE_BUFFER
#define SEM_OCC_INITIAL_VALUE 0
#define SEM_LIB_NAME "/Socc"
#define SEM_OCC_NAME "/Slib"

static MSG_BLOCK Buffer[SIZE_BUFFER];

// Initialise les tableaux et leurs indices pour le Multi-Write Mono-Read
static int TabOcc[SIZE_BUFFER];
static int TabLib[SIZE_BUFFER];

// Pointeurs pour les tableaux TabOcc et TabLib pour le Multi-Write
// Il faudra les protéger et rendre leur actualisation atomique
static u_int8_t iocc = 0;
static u_int8_t ilib = 0;
//Pour le multi-read
static u_int8_t jocc = 0;
static u_int8_t jlib = 0;


// Mutex protégeant le multi-Write
static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER, mutex2 = PTHREAD_MUTEX_INITIALIZER;
// Mutex protégeant le multi-Read
static pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER, mutex4 = PTHREAD_MUTEX_INITIALIZER;
// Mutex protégeant produceCount
static pthread_mutex_t mprodcount = PTHREAD_MUTEX_INITIALIZER;

// Semaphores liés au remplissage du tableau
static sem_t *Socc;
static sem_t *Slib;
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
	// On supprime les sémaphores si ceux-ci existent
	sem_unlink(SEM_OCC_NAME);
	sem_unlink(SEM_LIB_NAME);
	// On créent les sémaphores qu'on désire
	Socc = sem_open(SEM_OCC_NAME, O_CREAT, 0644, SEM_OCC_INITIAL_VALUE);
	Slib = sem_open(SEM_LIB_NAME, O_CREAT, 0644, SEM_LIB_INITIAL_VALUE);
	// On vérifie si ils sont ouverts
	if ((Socc == SEM_FAILED) || (Slib == SEM_FAILED))
	{
		perror("[sem_open]");
		return ERROR_INIT;
	}
	// On initialise les tableaux pour le Multi-Write Mono-Read
	for (int i = 0; i < SIZE_BUFFER; i++)
	{
		TabOcc[i] = i;
		TabLib[i] = i;
	}

	// #####################################################################

	printf("[acquisitionManager]Semaphore created\n");
	return ERROR_SUCCESS;
}

static void incrementProducedCount(void)
{
	// TODO
	//  #####################################################################
	pthread_mutex_lock(&mprodcount);
	produceCount++;
	pthread_mutex_unlock(&mprodcount);
	//  #####################################################################
}

unsigned int getProducedCount(void)
{
	unsigned int p = 0;
	// TODO
	// #####################################################################
	pthread_mutex_lock(&mprodcount);
	p = produceCount;
	pthread_mutex_unlock(&mprodcount);
	// #####################################################################
	return p;
}

MSG_BLOCK getMessage(void)
{
	// TODO
	//  #####################################################################
	// Multi-read

	MSG_BLOCK msg;
	int tmp;

	sem_wait(Socc);

	// Recupération de l'adresse
	pthread_mutex_lock(&mutex3);
	tmp=TabOcc[jocc];
	jocc = ((jocc + 1)%SIZE_BUFFER); 
	pthread_mutex_unlock(&mutex3);


	// Récupération de la donnée
	msg = Buffer[tmp];

	
	// Actualisation du tableau libre
	pthread_mutex_lock(&mutex4);
	TabOcc[jlib] = tmp;
	jlib = ((jlib + 1) % SIZE_BUFFER);
	pthread_mutex_unlock(&mutex4);

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
//  extern pid_t gettid(void); // just to remove one warning
//  #####################################################################

void *produce(void *params)
{
	D(printf("[acquisitionManager]Producer created with id %d\n", gettid()));
	unsigned int i = 0;

	// Création des tampons
	MSG_BLOCK msg;
	int ind;

	while (i < PRODUCER_LOOP_LIMIT)
	{
		i++;
		sleep(PRODUCER_SLEEP_TIME + (rand() % 5));

		// TODO
		//  #####################################################################
		// Load the input from the entry
		getInput(i, &msg);

		// On fait le checksum avant la transmission
		if (messageCheck(&msg)) 
		{
			// Début du Multi-write
			sem_wait(Slib);

			pthread_mutex_lock(&mutex1);
			ind = TabLib[ilib];
			ilib = ((ilib + 1) % SIZE_BUFFER);
			pthread_mutex_unlock(&mutex1);

			Buffer[ind] = msg;
			incrementProducedCount();

			pthread_mutex_lock(&mutex2);
			TabOcc[iocc] = ind;
			iocc = ((iocc + 1) % SIZE_BUFFER);
			pthread_mutex_unlock(&mutex2);

			sem_post(Socc);
			// Fin du Multi-Write
		}
		//  #####################################################################
	}
	printf("[acquisitionManager] %d termination\n", gettid());
	// TODO
	//  #####################################################################
	pthread_exit(NULL);
	//  #####################################################################
}