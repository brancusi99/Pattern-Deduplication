#include <stddef.h>
#include <errno.h>
#include <pthread.h>
// #include <ncurses.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/select.h>

void *inet_main(void *args);


#define INETPORT 18081


//functia main a programului
//porneste serverul

int main()
{
    int iport;

    pthread_t inetthr;//,       /* INET Thread: the INET server component */


    iport = INETPORT;
    pthread_create(&inetthr, NULL, inet_main, &iport);

    pthread_join(inetthr, NULL);

    return 0;
}
