#ifndef __FILES_H__
#define __FILES_H__

#include <pthread.h>

#define MAXSIZE 42 //20 from key + 20 from value + 1 from '&' + 1 from '\n'
#define FILE_NAME_SIZE 5 //1 from 'f' + 3 from shardID + 1 from '\0'
#define KV_SIZE 20

int initFiles(int nShards);
int fileUpdateValue(int shardID, int KVID, char* value);
int fileWriteKV(int shardID, int KVID, char* key, char* value);
int fileDeleteKV(int shardID, int KVID);
int fileInit(int shardID);
int fileReadAll(int shardID);
int fileNextKVID(int shardID);

#endif