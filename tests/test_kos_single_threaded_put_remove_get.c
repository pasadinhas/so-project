#include <kos_client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_EL 100
#define NUM_SHARDS 10  
#define KEY_SIZE 20

//#define DEBUG_PRINT_ENABLED 1  // uncomment to enable DEBUG statements
#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif
int main(int argc, const  char* argv[] ) {
	char key[KEY_SIZE], value[KEY_SIZE];
	char* v;
	int i,j;
	int client_id=0;

	kos_init(1,1,NUM_SHARDS);


	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=NUM_EL; i>=0; i--) {
			sprintf(key, "k%d",i);
			sprintf(value, "val:%d",i);
			DEBUG("Element <%s,%s> being inserted in shard %d....\n", key, value, j);
			fflush(stdin);
			v=kos_put(client_id,j, key,value);
			if (v!=NULL) {
				printf("TEST FAILED - SHOULD RETURN NULL AND HAS RETURNED %s",v);
				exit(-1);
			}
		}
	}

	printf("------------------ ended inserting --------------------------------\n");

	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=NUM_EL-1; i>=NUM_EL/2; i--) {
			sprintf(key, "k%d",i);
			sprintf(value, "val:%d",i);
			v=kos_remove(client_id, j, key);
			if (v==NULL || strncmp(v,value,KEY_SIZE)!=0) {
				printf("Error when removing key %s from shard %d value should be %s and was returned %s",key,j,value,(v==NULL?"NULL":v));
				exit(1);
			}
			DEBUG("C:%d  %s %s removed from shard %d. value =%s\n", client_id, key, ( v==NULL ? "has not been" : "has been" ),j,
									( v==NULL ? "<missing>" : v ) );
		}
	}

	printf("------------------ ended removing --------------------------------\n");


	for (j=NUM_SHARDS-1; j>=0; j--) {	
		for (i=0; i<NUM_EL; i++) {
			sprintf(key, "k%d",i);
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

	printf("------------------ ended checking after removing --------------------------------\n");
	printf("\n--> TEST PASSED <--\n");
	return 0;
}
