#include <stdlib.h>
#include "kos_request.h"

/* Generic new kos_request_t */
kos_request_t * newKosRequest(kos_request_action_t action, int shardID, char *key, char *value, int *size) {
    kos_request_t *request = (kos_request_t *) malloc(sizeof(kos_request_t));
    request->action = action;
    request->shardID = shardID;
    request->key = key;
    request->value = value;
    request->dim_ptr = size;
    return request;
}

kos_request_t * newKosRequestGET(int shardID, char *key) {
    return newKosRequest(GET, shardID, key, NULL, NULL);
}

kos_request_t * newKosRequestPUT(int shardID, char *key, char *value) {
    return newKosRequest(PUT, shardID, key, value, NULL);
}

kos_request_t * newKosRequestREMOVE(int shardID, char *key) {
    return newKosRequest(REMOVE, shardID, key, NULL, NULL);
}

kos_request_t * newKosRequestGETALL(int shardID, int *dim) {
    return newKosRequest(GETALL, shardID, NULL, NULL, dim);
}






