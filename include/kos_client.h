#ifndef KOS_H
#define KOS_H 1

#define KV_SIZE 20
typedef struct KV_t {
	char key[KV_SIZE];
	char value[KV_SIZE];
} KV_t;



/* Functions exposed by the KOS system to the Clients */

/* Used to initialize the server side of the application. It takes as input parameter the number of threads to be concurrently activated on the server side,  the size of the internal buffer used to enqueue user client requests, and the number of existing shards. 

This function returns:
   *   0, if the initialization was successful; 
   *  -1 if the initialization failed;   */
int kos_init(int num_server_threads, int buf_size, int num_shards);


/*NOTE:
 All the operations specified below takes, among others, the following parameters:
- the clientid: which is meant to be used to allow mapping the client to a specific server thread (needed by the first part of the project).
- the shardId: which identifies the shard in which the key should be removed.

In case an invalid clientId/shardId is passed as input parameter, all functions should return NULL. 

- The keys maintained by KOS must be not NULL, but the values (of key/value pairs) can be NULL.

*/

/* returns NULL if key is not present. Otherwise, it returns the value associated with the key passed as input. 
*/

char* kos_get(int clientid, int shardId, char* key);

/* insert the <key, value> pair passed as input parameter in KOS,returning: NULL, if the key was not previously present in KOS; the previous value associated with the key, otherwise. 
*/
char* kos_put(int clientid, int shardId, char* key, char* value);

/* Removes the <key, value> pair passed as input parameter in KOS, returning: NULL, if the key was not present in KOS; the value associated with the key whose removal was requested, if the key was  previously present in KOS. 
*/


char* kos_remove(int clientid, int shardId, char* key);

/* returns an array of KV_t containing all the key value pairs stored in the specified shard of KOS (NULL in case the shard is empty). It stores in dim the number of key/value pairs present in the specified shard. If the clientId or shardId are invalid (e.g. too large with respect to the value specified upon inizialitization of KOS), this function assigns -1 to the "dim" parameter.   */
KV_t* kos_getAllKeys(int clientid, int shardId, int* dim);


#endif
