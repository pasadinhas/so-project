#include <kos_client.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include "shard.h"
#include "buffer.h"
#include "kos_request.h"
#include "kos_server.h"
#include "files.h"

Shard** shards;
Semaphores_t * semaphores;
void** buffer;

int kos_init(int num_server_threads, int buf_size, int num_shards) {
	
	// Args validation
	if(num_shards <= 0) return -1;
	if(num_server_threads <= 0) return -1;
	if(buf_size <= 0) return -1;
	if(num_server_threads != buf_size) {
		// we take the minimum value
		if (num_server_threads < buf_size) {
			buf_size = num_server_threads;
		} else {
			num_server_threads = buf_size;
		}
	}
	if(bufferCompleteInit(buf_size) != 0) return -1;
	if(newShardArray(num_shards, HT_SIZE) != 0) return -1;
	if(newServerThreadsArray(num_server_threads) != 0) return -1;
	//if(initFiles(num_shards) != 0) return -1;
	return 0;

}


char* kos_get(int clientid, int shardId, char* key) {
	char * result;
	int ticket = bufferGetTicket();
	bufferPutPosition(ticket, (void *) newKosRequestGET(shardId, key));
	//printf(">>> Client Request (Thread #%d) - GET REQUEST - ticket = %d \n", clientid, ticket);
	sem_post(&(semaphores[ticket].client));
	sem_wait(&(semaphores[ticket].server));
	result = (char*) bufferGetPosition(ticket);
	bufferPostTicket(ticket);
	return result;
}

char* kos_put(int clientid, int shardId, char* key, char* value) {
	char * result;
	int ticket = bufferGetTicket();
	bufferPutPosition(ticket, (void*)  newKosRequestPUT(shardId, key, value));
	//printf(">>> Client Request (Thread #%d) - PUT REQUEST - ticket = %d \n", clientid, ticket);
	sem_post(&(semaphores[ticket].client));
	sem_wait(&(semaphores[ticket].server));
	result = (char*) bufferGetPosition(ticket);
	bufferPostTicket(ticket);
	return result;
}

char* kos_remove(int clientid, int shardId, char* key) {
	char * result;
	int ticket = bufferGetTicket();
	bufferPutPosition(ticket, (void*)  newKosRequestREMOVE(shardId, key));
	//printf(">>> Client Request (Thread #%d) - REMOVE REQUEST - ticket = %d \n", clientid, ticket);
	sem_post(&(semaphores[ticket].client));
	sem_wait(&(semaphores[ticket].server));
	result = (char*) bufferGetPosition(ticket);
	bufferPostTicket(ticket);
	return result;
}

KV_t* kos_getAllKeys(int clientid, int shardId, int* dim) {
	KV_t * result;
	int ticket = bufferGetTicket();
	bufferPutPosition(ticket, (void*)  newKosRequestGETALL(shardId, dim));
	//printf(">>> Client Request (Thread #%d) - GET ALL REQUEST - ticket = %d \n", clientid, ticket);
	sem_post(&(semaphores[ticket].client));
	sem_wait(&(semaphores[ticket].server));
	result = (KV_t *) bufferGetPosition(ticket);
	bufferPostTicket(ticket);
	return result;
}


