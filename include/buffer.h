#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <semaphore.h>
#include <pthread.h>

int bufferCompleteInit(int size);
int bufferGetTicket();
void bufferPostTicket(int ticket);

/**
 *	The buffer itself
 */
 
extern void ** buffer;
extern int buffer_size;

int initBufferArray(int size);
void *bufferGetPosition(int i);
void bufferPutPosition(int i, void *ptr);

/**
 *	Semaphores to sync the client and the server 
 */
 
typedef struct semaphores_t {
    sem_t client, server;
} Semaphores_t;

extern Semaphores_t * semaphores;

int initSemaphoresArray(int n);

/**
 * 	Semaphore that controls the client's access to the buffer itself 
 */
 
extern sem_t accessBufferSem;
int initAccessBufferSem(int n);

/**
 * 	Stack to give an index to the client
 */
 
extern int *stack;
pthread_mutex_t stackMutex;
int	stackInit(int n);
void stackPush(int i);
int stackPop();

#endif
