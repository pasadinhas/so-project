#include <kos_client.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_EL 1000
#define NUM_SHARDS 10
#define NUM_CLIENT_THREADS 10
#define NUM_SERVER_THREADS 3

#define KEY_SIZE 20

// #define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements
#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif


void *client_thread(void *arg) {

	char key[KEY_SIZE], value[KEY_SIZE];
	char* v;
	int i,j;
	int client_id=*( (int*)arg);


	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=NUM_EL; i>=0; i--) {
			sprintf(key, "k-c%d-%d",client_id,i);
			sprintf(value, "val:%d",i);
			v=kos_put(client_id, j, key,value);
			DEBUG("C:%d  <%s,%s> inserted in shard %d. Prev Value=%s\n", client_id, key, value, j, ( v==NULL ? "<missing>" : v ) );
		}
	}

	printf("------------------- %d:1/4 ENDED INSERTING -----------------------\n",client_id);

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k-c%d-%d",client_id,i);
			sprintf(value, "val:%d",i);
			v=kos_get(client_id, j, key);
			if (strncmp(v,value,KEY_SIZE)!=0) {
				printf("Error on key %s value should be %s and was returned %s",key,value,v);
				exit(1);
			}
			DEBUG("C:%d  %s %s found in shard %d: value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),j,
									( v==NULL ? "<missing>" : v ) );
	
		}
	}
	
	printf("------------------ %d:2/4 ENDED READING  ---------------------\n",client_id);

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=NUM_EL-1; i>=NUM_EL/2; i--) {
			sprintf(key, "k-c%d-%d",client_id,i);
			sprintf(value, "val:%d",i);
			v=kos_remove(client_id, j, key);
			if (strncmp(v,value,KEY_SIZE)!=0) {
				printf("Error when removing key %s value should be %s and was returned %s",key,value,v);
				exit(1);
			}
			DEBUG("C:%d  %s %s removed from shard %d. value =%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),j,
									( v==NULL ? "<missing>" : v ) );
		}
	}

	printf("----------------- %d-3/4 ENDED REMOVING -------------------------\n",client_id);

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k-c%d-%d",client_id,i);
			sprintf(value, "val:%d",i);
			v=kos_get(client_id, j, key);
			if (i>=NUM_EL/2 && v!=NULL) {
				printf("Error when gettin key %s value should be NULL and was returned %s",key,v);
				exit(1);
			}
			if (i<NUM_EL/2 && strncmp(v,value,KEY_SIZE)!=0 ) {
				printf("Error on key %s value should be %s and was returned %s",key,value,v);
				exit(1);
			}
			DEBUG("C:%d  %s %s found in shard %d. value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ) ,j, ( v==NULL ? "<missing>" : v ) );
		}
	}

	printf("----------------- %d-4/4 ENDED CHECKING AFTER REMOVE -----------------\n",client_id);




	return NULL;
}


int main(int argc, const  char* argv[] ) {

	int i,s,ret;
	int* res;
	pthread_t* threads=(pthread_t*)malloc(sizeof(pthread_t)*NUM_CLIENT_THREADS);
	int* ids=(int*) malloc(sizeof(int)*NUM_CLIENT_THREADS);

	ret=kos_init(NUM_CLIENT_THREADS,NUM_SERVER_THREADS,NUM_SHARDS);

	//printf("KoS inited");

	if (ret!=0)  {
			printf("kos_init failed with code %d!\n",ret);
			return -1;
		}
		
	for (i=0; i<NUM_CLIENT_THREADS; i++) {	
		ids[i]=i;		
		
		if ( (s=pthread_create(&threads[i], NULL, &client_thread, &(ids[i])) ) ) {
			printf("pthread_create failed with code %d!\n",s);
			return -1;
		}
	}

	for (i=0; i<NUM_CLIENT_THREADS; i++) {	
               s = pthread_join(threads[i], (void**) &res);
               if (s != 0) {
                   printf("pthread_join failed with code %d",s);
			return -1;
		}
           }

	printf("\n--> TEST PASSED <--\n");

	return 0;
}
