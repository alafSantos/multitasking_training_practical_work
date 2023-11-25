#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include "displayManager.h"
#include "iDisplay.h"
#include "iAcquisitionManager.h"
#include "iMessageAdder.h"
#include "msg.h"
#include "multitaskingAccumulator.h"
#include "debug.h"

//  #####################################################################
extern pid_t gettid(void); // just to remove one warning
//  #####################################################################

// DisplayManager thread.
pthread_t displayThread;

/**
 * Display manager entry point.
 * */
static void *display(void *parameters);

void displayManagerInit(void)
{
	// TODO
	//  #####################################################################
	pthread_create(&displayThread, NULL, display, NULL);
	// #####################################################################
}

void displayManagerJoin(void)
{
	// TODO
	//  #####################################################################
	pthread_join(displayThread, NULL);
	// #####################################################################
}

static void *display(void *parameters)
{
	D(printf("[displayManager]Thread created for display with id %d\n", gettid()));
	unsigned int diffCount = 0;
	while (diffCount < DISPLAY_LOOP_LIMIT)
	{
		sleep(DISPLAY_SLEEP_TIME);
		// TODO
		//  #####################################################################
		MSG_BLOCK tmp = getCurrentSum();
		if (messageCheck(&tmp))
		{
			messageDisplay(&tmp);
		}

		__uint8_t msgLeft = getProducedCount() - getConsumedCount();
		if (msgLeft == 0)
			diffCount = DISPLAY_LOOP_LIMIT; // il va mieux de verifier ça
		print(getProducedCount(), getConsumedCount());
		//  #####################################################################
	}
	printf("[displayManager] %d termination\n", gettid());
	// TODO
	//  #####################################################################
	pthread_exit(NULL);
	// #####################################################################
}