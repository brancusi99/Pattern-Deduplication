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

int main(void)
{
    int iport;

    pthread_t inetthr;//,       /* INET Thread: the INET server component */


    iport = INETPORT;
    pthread_create(&inetthr, NULL, inet_main, &iport);

    pthread_join(inetthr, NULL);

    return 0;
}


/*
Exemplu rulare:
./serverds
        Received msgHeader: 16 0, 0 (12)
        Detected new client! New clientID: 1776975063
        Received msgHeader: 16 1776975063, 6 (12)
The string size was received: 13
        Received stream is {13}
        Received message is {imagini_input}
Imagine incarcata: berlin_16627548_1.jpg (1440x960)
Imagine incarcata: berlin_16627548_2.jpg (1440x960)
Imagine incarcata: berlin_16627548_3.jpg (1440x960)
S-au incarcat 3 imagini.
Imagine salvata cu succes la: serv_files/output_dedup/test_output.png
        Sent size notification [52]
Procesare OpenCV completă! Fisier salvat pe server.|    [52/24//8]
        Received msgHeader: 16 1776975063, 5 (12)
*/

