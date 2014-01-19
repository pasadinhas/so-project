#ifndef DELAY_H
#define DELAY_H 1

/* 

This function *MUST* be invoked before processing each client request. It injects artificial delays (simulating for instance interactions with some remote server) and allows stressing the synchronization mechanisms.

*/
void delay();
#endif
