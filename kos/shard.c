#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "kos_client.h"
#include "shard.h"
#include "buffer.h"
#include "files.h"
#include "delay.h"

/** *****************
 *  HashTable Functions 
 ** ****************/
 
int hash(char* key) {
	int i = 0;
	if (key == NULL) return -1;
	while (*key != '\0') {
		i += (int) *key;
		key++;
	}
	return i % HT_SIZE;
}

SynchronizedList** newHashTable(int hashSize) {
	int i;
	SynchronizedList** hashTable = (SynchronizedList**) malloc(hashSize * sizeof(SynchronizedList*));
	for (i = 0; i < hashSize; i++) 
		hashTable[i] = newSynchronizedList();
	return hashTable;
}

/** *****************
 *  Synchronized List Functions 
 ** ****************/
 
SynchronizedList* newSynchronizedList() {
	SynchronizedList* sl = (SynchronizedList *) malloc(sizeof(SynchronizedList));
	if(!sl) return NULL;
	sl->list = NULL;
	sem_init(&(sl->readers), 0, 0);
	sem_init(&(sl->writers), 0, 0);
	sl->nReaders = 0;
	sl->waitingReaders = 0;
	sl->waitingWriters = 0;
	sl->writing = FALSE;
	return sl;
}

void beginReading(SynchronizedList *sl){
	pthread_mutex_lock(&(sl->mutex));
	if (sl->writing || sl->waitingWriters > 0) {
		(sl->waitingReaders)++;
		pthread_mutex_unlock(&(sl->mutex));
		sem_wait(&(sl->readers));
		pthread_mutex_lock(&(sl->mutex));
		if (sl->waitingReaders > 0) {
			(sl->nReaders)++ ;
			(sl->waitingReaders)-- ;
			sem_post(&(sl->readers));
		}
	}
	else
		sl->nReaders++;
	pthread_mutex_unlock(&(sl->mutex));
}

void endReading(SynchronizedList *sl) {
	pthread_mutex_lock(&(sl->mutex));
	(sl->nReaders)-- ;	
	if (sl->nReaders == 0 && sl->waitingWriters > 0) {
		sem_post(&(sl->writers));
		sl->writing = TRUE;
		(sl->waitingWriters)-- ;
	}
	pthread_mutex_unlock(&(sl->mutex));
}

void beginWriting(SynchronizedList *sl){
	pthread_mutex_lock(&(sl->mutex));
	if (sl->writing || sl->nReaders > 0 || sl->waitingReaders > 0) {
		(sl->waitingWriters)++;
		pthread_mutex_unlock(&(sl->mutex));
		sem_wait(&(sl->writers));
		pthread_mutex_lock(&(sl->mutex));
	}
	sl->writing = TRUE;
	pthread_mutex_unlock(&(sl->mutex));
}

void endWriting(SynchronizedList *sl){
	pthread_mutex_lock(&(sl->mutex));
	sl->writing = FALSE;
	if(sl->waitingReaders > 0) {
		sem_post(&(sl->readers));
		(sl->nReaders)++ ;
		(sl->waitingReaders)-- ;
	}
	else if (sl->waitingWriters > 0) {
		sem_post(&(sl->writers));
		sl->writing = TRUE;
		(sl->waitingWriters)-- ;
	}
	pthread_mutex_unlock(&(sl->mutex));
}

/** *****************
 *  List Functions 
 ** ****************/
 
KV_t* NewKV(char* key, char* value){
	KV_t* new = (KV_t*) malloc(sizeof(KV_t));
	strncpy(new->key, key, KV_SIZE);
	strncpy(new->value, value, KV_SIZE);
	return new;
}

List NewListNode(KV_t* item) {
	List x = (List) malloc(sizeof(struct node));
	x->item = item;
	x->next = NULL;
	x->KVID = -1;
	return x;
}

List insertList(List head, int shardID, KV_t* item, char** result, int* KVID) {
	List current, previous;
	List newNode = NewListNode(item);
	for (current = head, previous = NULL; current != NULL; previous = current, current = current->next) {
		if (strcmp(current->item->key, item->key) == 0) {
			// the element already exists
			*result = current->item->value;
			*KVID = current->KVID;
			if (current == head) {
				head = newNode;
			} else {
				previous->next = newNode;
			}
			newNode->KVID = current->KVID;
			newNode->next = current->next;
			//free(current);
			return head;
		}
	}
	// if we dont found the element, we insert in the begin
	*result = NULL;
	*KVID = fileNextKVID(shardID);
	newNode->KVID = (*KVID);
	newNode->next = head;
	return newNode;
}

List insertListFromFile(List head, KV_t* item, int KVID) {
	List newNode = NewListNode(item);
	newNode->KVID = KVID;
	newNode->next = head;
	return newNode;
}

KV_t* searchList(List head, char* key) {
	List current;
	KV_t* item;
	for (current = head; current != NULL; current = current->next) {
		item = current->item;
		if (strcmp(item->key, key) == 0)
			return item;
	}
	
	return NULL;
}

List removeItemList(List head, char* key, char** result, int* KVID) {
	List current, prev;
	KV_t* item;
	for (current = head, prev = NULL; current != NULL; prev = current, current = current->next) {
		item = current->item;
		if (strcmp(item->key, key) == 0) {
			*result = item->value;
			*KVID = current->KVID;
			if (current == head) {
				head = current->next;
			}
			else {
				prev->next = current->next;
			}
			//free(current);
			return head;
		}
	}
	*result = NULL;
	*KVID = -1;
	return head;
}

/** *****************
 *  Shards Functions 
 ** ****************/

Shard** shards;
Semaphores_t * semaphores;
void** buffer;

int newShardArray(int n, int hashSize){
	int i;
	
	shards = (Shard**) malloc(sizeof(Shard*) * n);
	if(!shards) return -1;
	for(i = 0; i < n; i++) {
		shards[i] = newShard(hashSize, i);
	}
	initFiles(n);
	
	return 0;
	
}

Shard* newShard(int hashSize, int shardID){
	Shard* shard = (Shard*) malloc(sizeof(Shard));
	shard->hashTable = newHashTable(hashSize);
	shard->nElems = 0;
	shard->shardID = shardID;
	return shard;
	
}

char* ShardSearch(Shard* shard, char* key) {
	int i = hash(key);
	beginReading(shard->hashTable[i]);
	//delay();
	KV_t* elem = searchList(shard->hashTable[i]->list, key);
	endReading(shard->hashTable[i]);
	return (elem) ? elem->value : NULL;
}

char* ShardInsert(Shard* shard, char* key, char* value) {
	int i = hash(key);
	int KVID;
	char* result;
	KV_t *newKV = NewKV(key, value);
	beginWriting(shard->hashTable[i]);
	//delay();
	shard->hashTable[i]->list = insertList(shard->hashTable[i]->list, shard->shardID, newKV, &result, &KVID);
	if (result){
	 	fileUpdateValue(shard->shardID, KVID, value);
	 	//printf(">>> FILE UPDATE VALUE: <%s,%s>\n", key, value);
	} else {
		shard->nElems++;
		fileWriteKV(shard->shardID, KVID, key, value);
	}
	endWriting(shard->hashTable[i]);
	return result;
}

void ShardInsertFromFile(Shard* shard, char* key, char* value, int KVID) {
	int i = hash(key);
	KV_t *newKV = NewKV(key, value);
	shard->hashTable[i]->list = insertListFromFile(shard->hashTable[i]->list, newKV, KVID);
	shard->nElems++;
}

char* ShardDelete(Shard* shard, char* key) {
	int i = hash(key);
	int KVID;
	char* result;
	beginWriting(shard->hashTable[i]);
	//delay();
	shard->hashTable[i]->list = removeItemList(shard->hashTable[i]->list, key, &result, &KVID);
	if (result) {
		shard->nElems--;
		fileDeleteKV(shard->shardID, KVID);
	}
	endWriting(shard->hashTable[i]);
	return result;
}

KV_t* ShardGetAll(Shard* shard, int* n){
	int i, j;
	KV_t* array;
	List current;
	if(shard->nElems == 0) return NULL;
	array = (KV_t*) malloc(sizeof(KV_t) * shard->nElems);
	//delay();
	for (i = 0, j = 0; i < HT_SIZE; i++) {
		beginReading(shard->hashTable[i]);
		current = shard->hashTable[i]->list;
		while (current != NULL) {
			array[j] = *(current->item);
			j++;
			current = current->next;
		}
		endReading(shard->hashTable[i]);
	}    
	*n = shard->nElems;
	return array;
}

