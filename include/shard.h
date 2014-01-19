#ifndef __SHARD_H__
#define __SHARD_H__

#include <pthread.h>
#include <semaphore.h>
#include "kos_client.h"

#define HT_SIZE 10
#define FALSE 0
#define TRUE 1

/** *****************
 *  Data Structures 
 ** ****************/

typedef struct node
{
	KV_t* item;
	int KVID;
	struct node *next;
} * List;

typedef struct syncedList {
	List list;
	pthread_mutex_t mutex;
	sem_t readers, writers;
	int waitingReaders, waitingWriters, writing, nReaders;
} SynchronizedList;

typedef struct shard {
	pthread_mutex_t fileMutex;
	SynchronizedList** hashTable;
	int nElems;
	int shardID;
} Shard;

/** *****************
 *  Functions 
 ** ****************/

/* Hashtable */
int hash(char* key);
SynchronizedList** newHashTable(int hashSize);

/* List */
KV_t* NewKV(char* key, char* value);
List NewListNode(KV_t* item);
List insertList(List head, int shardID, KV_t* item, char** result, int* KVID);
KV_t* searchList(List head, char* key);
List removeItemList(List head, char* key, char** result, int* KVID);

/* SynchronizedList */
SynchronizedList *newSynchronizedList();
void beginReading(SynchronizedList *sl);
void endReading(SynchronizedList *sl);
void beginWriting(SynchronizedList *sl);
void endWriting(SynchronizedList *sl);

/* Shard */
extern Shard** shards;

int newShardArray(int n, int hashSize);
Shard* newShard(int hashSize, int shardID);
char* ShardSearch(Shard* shard, char* key);
char* ShardInsert(Shard* shard, char* key, char* value); 
char* ShardDelete(Shard* shard, char* key);
KV_t* ShardGetAll(Shard* shard, int* n);
void ShardInsertFromFile(Shard* shard, char* key, char* value, int KVID);

#endif
