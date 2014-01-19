#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include "shard.h"
#include "buffer.h"
#include "kos_request.h"
#include "delay.h"

Shard** shards;
Semaphores_t * semaphores;
void** buffer;

void *server_thread(void *arg) {
    
    int id = *((int *) arg);    
    kos_request_t *request;
    
    while(1) {
        
        sem_wait(&(semaphores[id].client));
	    //printf(">>> Server Thread (#%d) running. \n", id);
        request = (kos_request_t *) bufferGetPosition(id);
        //delay();
        if(request) {
            //pthread_mutex_lock(&(shards[request->shardID]->mutex));           
            switch(request->action) {
                case GET:
                    bufferPutPosition(id, (void *) ShardSearch(shards[request->shardID], request->key));
                    break;
                case PUT:
                    bufferPutPosition(id, (void *) ShardInsert(shards[request->shardID], request->key, request->value));
                    break;
                case REMOVE:
                    bufferPutPosition(id, (void *) ShardDelete(shards[request->shardID], request->key));
                    break;
                case GETALL:
                    bufferPutPosition(id, (void *) ShardGetAll(shards[request->shardID], request->dim_ptr));
                    break;
                default:
                    bufferPutPosition(id, NULL);
                    break; 
            }
            //pthread_mutex_unlock(&(shards[request->shardID]->mutex));
            
        } else bufferPutPosition(id, NULL);

        sem_post(&(semaphores[id].server));
        
    }
}

int newServerThreadsArray(int n) {
    pthread_t * server_threads;
    int * ids = (int *) malloc(sizeof(int) * n);
    int i, s;
    server_threads = malloc(sizeof(pthread_t) * n);
    
    if (!server_threads)
        return -1;
    
    for (i = 0; i < n; i++) {
        ids[i] = i;
        s = pthread_create(&server_threads[i], NULL, &server_thread, &(ids[i]));
        if (s) {
            printf("Server pthread_create failed with code %d!\n",s);
            return -1;
        }
    }
    return 0;
}
