#include <delay.h>
#include <unistd.h>

#define DELAY 0
#define DELAY_TIME 1

void delay() {
	
	#ifdef DELAY
		sleep(DELAY_TIME);
	#endif
}
