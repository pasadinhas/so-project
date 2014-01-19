#ifndef __KOS_REQUEST_H__
#define __KOS_REQUEST_H__

typedef enum kos_request_action {
    PUT,
    GET,
    GETALL,
    REMOVE
} kos_request_action_t;

typedef struct {
    kos_request_action_t action;
    int shardID;
    int *dim_ptr; /* Used only with GETALL */
    char *key;
    char *value; /* Used only with PUT */
} kos_request_t;

kos_request_t * newKosRequestGET(int shardID, char *key);
kos_request_t * newKosRequestPUT(int shardID, char *key, char *value);
kos_request_t * newKosRequestREMOVE(int shardID, char *key);
kos_request_t * newKosRequestGETALL(int shardID, int *dim);

#endif
