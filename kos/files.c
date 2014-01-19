#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "files.h"
#include "shard.h"

typedef struct intList {
	int value;
	struct intList* next;
} IntegerList;

IntegerList* newIntegerListNode(int value, IntegerList* next) {
	IntegerList* newNode = (IntegerList *) malloc(sizeof(IntegerList));
	newNode->value = value;
	newNode->next = next;
	return newNode;
}

IntegerList* popIntegerListValue(IntegerList* head, int* value) {
	IntegerList* newHead = head->next;
	*value = head->value;
	//free(head);
	return newHead;
}

// Global Variables to this file
int* fileSize;
pthread_mutex_t* fileLocks;
pthread_mutex_t* freeIDsLocks;
IntegerList** freeIDs;


// This is done assuming the shards start at 0 and end ate nShards - 1
int initFiles(int nShards) {
	int i;
	fileSize = (int*) malloc(sizeof(int) * nShards);
	freeIDs = (IntegerList**) malloc(sizeof(IntegerList*) * nShards);
	fileLocks = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * nShards);
	freeIDsLocks = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * nShards);
	if (fileSize == NULL || freeIDs == NULL || fileLocks == NULL) return -1;
	for (i = 0; i < nShards; i++) {
		freeIDs[i] = NULL;
		fileSize[i] = fileInit(i);
	}
	return 0;
}

/* PASADINHAS FUNCTION */
int fileCalculateOffset(int KVID) {
	return KVID * 2 * KV_SIZE + sizeof(int);
}

/* Used to get the next KVID */
int fileIncrementSize(int fd, int shardID) {
	fileSize[shardID]++;
	lseek(fd, 0, SEEK_SET);
	write(fd, &(fileSize[shardID]), sizeof(int));
	// example: if file size was 0 (no KVs in the file) we increment the file size to 1 and return the index 0 (which is empty)
	return fileSize[shardID] - 1;
}

int fileDecrementSize(int fd, int shardID) {
	fileSize[shardID]--;
	lseek(fd, 0, SEEK_SET);
	write(fd, &(fileSize[shardID]), sizeof(int));
	// example: if file size was 0 (no KVs in the file) we increment the file size to 1 and return the index 0 (which is empty)
	return fileSize[shardID];
}

/* Used in ShardInsert if key already exists */
int fileUpdateValue(int shardID, int KVID, char* value) {
	int fd, i;
	char fileName[FILE_NAME_SIZE];
	char cleanValue[KV_SIZE];

	// open file
	sprintf(fileName, "f%d", shardID);
	fd = open(fileName, O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) return -1;

	// create string to write
	for (i = 0; i < KV_SIZE; i++)
		cleanValue[i] = '\0';
	sprintf(cleanValue, "%s", value);

	// position offset and write
	lseek(fd, fileCalculateOffset(KVID) + KV_SIZE, SEEK_SET);
	write(fd, cleanValue, KV_SIZE);
	close(fd);

	return 0;
}

/* Used in ShardInsert if key was not found in KOS */
int fileWriteKV(int shardID, int KVID, char* key, char* value) {
	int fd, i;
	char fileName[FILE_NAME_SIZE];
	char cleanValue[KV_SIZE], cleanKey[KV_SIZE];

	// open file
	sprintf(fileName, "f%d", shardID);
	fd = open(fileName, O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) return -1;

	// create strings to write
	for (i = 0; i < KV_SIZE; i++) {
		cleanValue[i] = '\0';
		cleanKey[i] = '\0';
	}
	sprintf(cleanValue, "%s", value);
	sprintf(cleanKey, "%s", key);

	// position offset and write
	lseek(fd, fileCalculateOffset(KVID), SEEK_SET);
	write(fd, cleanKey, KV_SIZE);
	write(fd, cleanValue, KV_SIZE);

	
	close(fd);

	return 0;
}

/* Returns file size */
int fileGetSize(int fd) {
	int size;
	lseek(fd, 0, SEEK_SET);
	read(fd, &size, sizeof(int));
	return size;
}

/* Used in ShardDelete */
// Isn't the push to the freeIDs missing?
int fileDeleteKV(int shardID, int KVID) {
	int fd, i;
	char fileName[FILE_NAME_SIZE];
	char cleanString[KV_SIZE * 2];

	// open file
	sprintf(fileName, "f%d", shardID);
	fd = open(fileName, O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) return -1;

	// create string to write
	for (i = 0; i < KV_SIZE * 2; i++)
		cleanString[i] = '\0';

	// position offset and write
	lseek(fd, fileCalculateOffset(KVID), SEEK_SET);
	write(fd, cleanString, KV_SIZE * 2);
	close(fd);
	
	pthread_mutex_lock(&(freeIDsLocks[shardID]));
	freeIDs[shardID] = newIntegerListNode(KVID, freeIDs[shardID]);
	pthread_mutex_unlock(&(freeIDsLocks[shardID]));
	//printf("»»» New free spot <%d> in Shard %d\n", KVID, shardID);

	return 0;
}

/* Returns file size if file exists, returns 0 and creates file is file doesnt exist */
int fileInit(int shardID) {
	int fd, i;
	char fileName[FILE_NAME_SIZE];
	char key[KV_SIZE], value[KV_SIZE];
	// open file
	sprintf(fileName, "f%d", shardID);
	fd = open(fileName, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0) {
		// file doesn't exist - creating new one.
		fd = open(fileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
		if (fd < 0) return -1;
		lseek(fd, 0, SEEK_SET);
		write(fd, 0, sizeof(int));
		close(fd);
		return 0;
	} else {
		fileSize[shardID] = fileGetSize(fd);
		lseek(fd, fileCalculateOffset(0), SEEK_SET);
		for (i = 0; i < fileSize[shardID]; i++) {
			read(fd, key, KV_SIZE);
			read(fd, value, KV_SIZE);
			if (key[0] == '\0') {
				do {
					lseek(fd, fileCalculateOffset(fileDecrementSize(fd, shardID)), SEEK_SET);
					read(fd, key, KV_SIZE);
					read(fd, value, KV_SIZE);
				} while(key[0] == '\0' && fileSize[shardID] > (i + 1));
				lseek(fd, fileCalculateOffset(i), SEEK_SET);
				write(fd, key, KV_SIZE);
				write(fd, value, KV_SIZE);
			}
			ShardInsertFromFile(shards[shardID], key, value, i); 
		}
		ftruncate(fd, (off_t) (sizeof(int) + fileSize[shardID] * 2 * KV_SIZE * sizeof(char)));
		close(fd);
		return 0;
	}
}

int fileNextKVID(int shardID) {
	int aux, fd;
	char fileName[FILE_NAME_SIZE];
	// open file
	sprintf(fileName, "f%d", shardID);
	fd = open(fileName, O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) return -1;
	// update value
	pthread_mutex_lock(&(freeIDsLocks[shardID]));
	if(freeIDs[shardID] != NULL) {
		freeIDs[shardID] = popIntegerListValue(freeIDs[shardID], &aux);
		pthread_mutex_unlock(&(freeIDsLocks[shardID]));
		//printf(">>> Insert (Shard %d) - free spot at <%d>\n", shardID, aux);
	} else {
		pthread_mutex_unlock(&(freeIDsLocks[shardID]));
		pthread_mutex_lock(&(fileLocks[shardID]));
		aux = fileIncrementSize(fd, shardID);
		pthread_mutex_unlock(&(fileLocks[shardID]));
		//printf(">>> WRITE at Shard #%d - Too bad, no free spot :c writing at <%d> and file size is now <%d>\n", shardID, aux, fileSize[shardID]);
	}
	// close file
	close(fd);
	return aux;
}