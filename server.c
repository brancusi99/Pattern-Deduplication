#include <stddef.h>
#include <errno.h>
#include <pthread.h>
// #include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

void *unix_main(void *args);
void *inet_main(void *args);
void *soap_main(void *args);

// WINDOW *mainwnd ;
//#define UNIXSOCKET "/tmp/unixds"
//pentru conexiuni tcp:
#define INETPORT 18081
//pentru conexiuni http:
//#define SOAPPORT 18082

//un mutex previne coliziunea threadurilor in caz ca vor sa faca 
//ceva in acelasi timp
pthread_mutex_t curmtx = PTHREAD_MUTEX_INITIALIZER;

//functia main a programului
//porneste serverul

int main()
{
    int iport;
    //, sport;

    pthread_t //unixthr, /* UNIX Thread: the UNIX server component */
        inetthr;//,       /* INET Thread: the INET server component */
        //soapthr;       /* SOAP Thread: the SOAP server component */
                       //	workerthr ;  /* The Worker Thread: use it for WORK tasks (various) */

    iport = INETPORT;
    pthread_create(&inetthr, NULL, inet_main, &iport);

    pthread_join(inetthr, NULL);

    return 0;
}
