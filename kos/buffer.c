#include <stdlib.h>
#include <pthread.h>
#include "buffer.h"

int bufferCompleteInit(int size) {
    if(initBufferArray(size) != 0) return -1;
    if(initSemaphoresArray(size) != 0) return -1;
    if(initAccessBufferSem(size) != 0) return -1;
    if(stackInit(size) != 0) return -1;
    return 0;
}

/**
 *  Buffer implementation
 */

void ** buffer;

int initBufferArray(int size) {
    buffer = (void **) malloc(sizeof(void *) * size);
    if(!buffer) return -1;
    return 0;
}

void *bufferGetPosition(int i) {
    return buffer[i];
}

void bufferPutPosition(int i, void *ptr) {
    buffer[i] = ptr;
}

/**
 *  Semaphores to sync clients and servers implementation
 */

Semaphores_t * semaphores;

int initSemaphoresArray(int n) {
    int i;
    semaphores = (Semaphores_t *) malloc(sizeof(Semaphores_t) * n);
    if (!semaphores)
        return -1;
    for (i = 0; i < n; i++) {
        sem_init(&(semaphores[i].client), 0, 0);
        sem_init(&(semaphores[i].server), 0, 0);
    }
    return 0;
}

/**
 * Access Buffer Semaphore implementation
 */
 
sem_t accessBufferSem;

int initAccessBufferSem(int n) {
    return sem_init(&accessBufferSem, 0, n);
}

/**
 *  Stack implementation
 */

pthread_mutex_t stackMutex;
int* stack;
static int stackIndex;

int stackInit(int n) {
	int i;
	stackIndex = 0;
	if (!(stack = (int *) malloc(sizeof(int) * n))) return -1;
	for (i=0; i<n; i++)
	    stackPush(i);
	return 0;
}

int stackPop() {
	int aux;
	pthread_mutex_lock(&stackMutex);
	//printf(">>> DEBUG\n StackPop ~> aux = stack[stackIndex] = stack[%d] = %d \n" , stackIndex, stack[stackIndex]);
	aux = stack[--stackIndex];
    //printf(">>> Stack Pop: index is now %d and stack returned %d \n", stackIndex, aux);
    //printf("WAITING ... zzzzzzzzzzzZZZZZZZZZZZZZZZZZZZZZzzzzzzzzzzZZZZZZZZZZZZZZZZZZZ\n");
	//getchar();
	pthread_mutex_unlock(&stackMutex);
	return aux;
}

void stackPush(int i) {
	pthread_mutex_lock(&stackMutex);
	stack[stackIndex++] = i;
	//printf(">>> Stack Push: index is now %d and inserted stack[%d] = %d \n", stackIndex, stackIndex -1, i);
    //printf("WAITING ... zzzzzzzzzzzZZZZZZZZZZZZZZZZZZZZZzzzzzzzzzzZZZZZZZZZZZZZZZZZZZ\n");
	//getchar();
	pthread_mutex_unlock(&stackMutex);
	//printf(">>> DEBUG\n STACK ~> Inserting ticket %d in stack\n", i);
}

/**
 *  Buffer tickets system
 */

int bufferGetTicket() {
    //printf(">>> Getting ticket. Stack Index = %d", stackIndex);
    sem_wait(&accessBufferSem);
    return stackPop();
}

void bufferPostTicket(int ticket) {
    //printf(">>> Getting ticket. Stack Index = %d", stackIndex);
    stackPush(ticket);
    sem_post(&accessBufferSem);
}

