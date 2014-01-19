#include <kos_client.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_EL 100
#define NUM_SHARDS 10
#define NUM_CLIENT_THREADS 10
#define NUM_SERVER_THREADS 2

#define KEY_SIZE 20

// #define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements
#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif


int lookup(char* key, char* value, KV_t* dump,int dim) {
	int i=0;
	for (;i<dim;i++) {
		if ( (!strncmp(key,dump[i].key,KEY_SIZE)) &&  (!strncmp(value,dump[i].value,KEY_SIZE) ) )
		 return 0;
	}
	return -1;
}


void *client_thread(void *arg) {

	char key[KEY_SIZE], value[KEY_SIZE], value2[KEY_SIZE];
	char* v;
	int i,dim;
	int client_id=*( (int*)arg);
	KV_t* dump;


	for (i=NUM_EL-1; i>=0; i--) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_put(client_id, client_id, key,value);
		DEBUG("C:%d  <%s,%s> inserted in shard %d. Prev Value=%s\n", client_id, key, value, j, ( v==NULL ? "<missing>" : v ) );
	}

	printf("------------------- %d:1/7 ENDED INSERTING -----------------------\n",client_id);

	for (i=0; i<NUM_EL; i++) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_get(client_id, client_id, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error on key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s found in shard %d: value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );	
	}

	
	printf("------------------ %d:2/7 ENDED READING  ---------------------\n",client_id);


	dump=kos_getAllKeys(client_id, client_id, &dim);
	if (dim!=NUM_EL) {
		printf("TEST FAILED - SHOULD RETURN %d ELEMS AND HAS RETURNED %d",NUM_EL,dim);
		exit(-1);
	}
		
	for (i=0; i<NUM_EL; i++) {
		sprintf(key, "k-c%d-%d", client_id, i);
		sprintf(value, "val:%d",i);
		if (lookup(key,value,dump, dim)!=0) {
			printf("TEST FAILED - Error on <%s,%s>, shard %d - not returned in dump\n",key,value,client_id);
			exit(-1);
		}			

	}


	printf("----------------- %d-3/7 ENDED GET ALL KEYS -------------------------\n",client_id);


	for (i=NUM_EL-1; i>=NUM_EL/2; i--) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_remove(client_id, client_id, key);
		if (strncmp(v,value,KEY_SIZE)!=0) {
			printf("Error when removing key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s removed from shard %d. value =%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),client_id,
								( v==NULL ? "<missing>" : v ) );
	}



	printf("----------------- %d-4/7 ENDED REMOVING -------------------------\n",client_id);


	for (i=0; i<NUM_EL; i++) {
		sprintf(key, "k-c%d-%d",client_id,i);
		sprintf(value, "val:%d",i);
		v=kos_get(client_id, client_id, key);
		if (i>=NUM_EL/2 && v!=NULL) {
			printf("Error when gettin key %s value should be NULL and was returned %s",key,v);
			exit(1);
		}
		if (i<NUM_EL/2 && strncmp(v,value,KEY_SIZE)!=0 ) {
			printf("Error on key %s value should be %s and was returned %s",key,value,v);
			exit(1);
		}
		DEBUG("C:%d  %s %s found in shard %d. value=%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ) ,client_id, ( v==NULL ? "<missing>" : v ) );
	}

	printf("----------------- %d-5/7 ENDED CHECKING AFTER REMOVE -----------------\n",client_id);


		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k-c%d-%d",client_id,i);
			sprintf(value, "val:%d",i*10);
			sprintf(value2, "val:%d",i*1);
			v=kos_put(client_id, client_id, key,value);

			if (i>=NUM_EL/2 && v!=NULL) {
				printf("Error when getting key %s value should be NULL and was returned %s",key,v);
				exit(1);
			}
			if (i<NUM_EL/2 && strncmp(v,value2,KEY_SIZE)!=0 ) {
				printf("Error on key %s value should be %s and was returned %s",key,value2,v);
				exit(1);
			}


			DEBUG("C:%d  <%s,%s> inserted in shard %d. Prev Value=%s\n", client_id, key, value, clent_id, ( v==NULL ? "<missing>" : v ) );
		}


	printf("----------------- %d-6/7 ENDED 2nd PUT WAVE ----------------\n",client_id);



		dump=kos_getAllKeys(client_id, client_id, &dim);
		if (dim!=NUM_EL) {
			printf("TEST FAILED - SHOULD RETURN %d ELEMS AND HAS RETURNED %d",NUM_EL,dim);
			exit(-1);
		}
			
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k-c%d-%d", client_id, i);
			sprintf(value, "val:%d",i*10);
			if (lookup(key,value,dump, dim)!=0) {
				printf("TEST FAILED - Error on <%s,%s>, shard %d - not returned in dump\n",key,value,client_id);
				exit(-1);
			}			

		}

	

	printf("----------------- %d-7/7 ENDED FINAL ALL KEYS CHECK ----------------------\n",client_id);

	return NULL;
}


int main(int argc, const  char* argv[] ) {

	int i,s,ret;
	int* res;
	pthread_t* threads=(pthread_t*)malloc(sizeof(pthread_t)*NUM_CLIENT_THREADS);
	int* ids=(int*) malloc(sizeof(int)*NUM_CLIENT_THREADS);

	// this test uses NUM_CLIENT_THREADS shards, as each client executes commands in its own shard
	ret=kos_init(NUM_CLIENT_THREADS,NUM_SERVER_THREADS,NUM_CLIENT_THREADS);

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
