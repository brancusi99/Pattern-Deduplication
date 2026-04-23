#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "proto.h"

#define PORT            18081
#define SERVERHOST      "127.0.0.1" 

// initializeaza conexiunea catre server
void init_sockaddr(struct sockaddr_in *name, const char *hostname, uint16_t port) {
    struct hostent *hostinfo;

    name->sin_family = AF_INET;
    name->sin_port = htons(port);
    hostinfo = gethostbyname(hostname);
    if (hostinfo == NULL) {
        (void)fprintf(stderr, "Unknown host %s.\n", hostname);
        exit(EXIT_FAILURE);
    }
    name->sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];
}

int main(int argc, char *argv[]) {
    int sock; 
    struct sockaddr_in servername;

    //creare socket
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket (client)");
        exit(EXIT_FAILURE);
    }

    //conectare la server
    init_sockaddr(&servername, SERVERHOST, PORT);
    if (0 > connect(sock, (struct sockaddr *) &servername, sizeof(servername))) {
        perror("connect (client)");
        exit(EXIT_FAILURE);
    }

    //hansdshakeul intiial
    msgHeaderType hdr;
    msgIntType msg;
    msgStringType str;
    int clientID = 0;

    hdr.clientID = 0;
    hdr.opID = 0; 
    hdr.msgSize = 0;
    (void)writeSingleInt(sock, hdr, 0); 
    (void)readSingleInt(sock,  &msg);   
    clientID = msg.msg;
    (void)fprintf(stderr, "[CLIENT] M-am conectat! Am primit ID-ul: %d\n\n", clientID);

    //Deduplicare    
    //stabileste folderul pe care urm sa-l proceseze
    //daca se ruleaza "./inetclient folder_test", ia argumentul, altfel ia o valoare default
    char *input_folder = "imagini_input"; 
    if (argc > 1) {
        input_folder = argv[1];
    }

    hdr.clientID = clientID;
    hdr.opID = OPR_DEDUP_START; 

    (void)fprintf(stderr, "=== START PATTERN DEDUPLICATION ===\n");
    (void)fprintf(stderr, "[CLIENT] Trimit cererea pentru folderul: '%s'\n", input_folder);

    //trimite numele folderului ca string catre server
    if (writeSingleString(sock, hdr, input_folder) < 0) {
        (void)fprintf(stderr, "[CLIENT] Eroare la trimiterea cererii!\n");
    } else {
        (void)fprintf(stderr, "[CLIENT] Cerere trimisa! Astept ca serverul (C++) sa proceseze imaginile...\n");

        //clientul ingheata pana cand programul C++ (cv_worker) termina
        if (readSingleString(sock, &str) > 0) {
            (void)fprintf(stderr, "[SERVER SPUNE]: %s\n", str.msg);
            //elibereaza memoria stringului primit
            free(str.msg); 
        } else {
            (void)fprintf(stderr, "[CLIENT] Eroare! Serverul a inchis brusc conexiunea.\n");
        }
    }
    (void)fprintf(stderr, "===================================\n\n");

    //inchide conexiunea
    hdr.clientID = clientID;
    hdr.opID = OPR_BYE;
    (void)fprintf(stderr, "[CLIENT] Trimit OPR_BYE si inchid programul.\n");
    writeSingleInt(sock, hdr, 0); 

    close(sock);
    exit(EXIT_SUCCESS);
}

/*
Exemplu rulare:
make inetclient
gcc -g -I/opt/homebrew/include inetsample2.c proto.o -Wall -o inetclient
Delia@Not-MacBonk-Air test % ./inetclient
[CLIENT] M-am conectat! Am primit ID-ul: 1776975063

=== START PATTERN DEDUPLICATION ===
[CLIENT] Trimit cererea pentru folderul: 'imagini_input'
        Sent size notification [13]
imagini_input|  [13/24//8]
[CLIENT] Cerere trimisa! Astept ca serverul (C++) sa proceseze imaginile...
The string size was received: 52
        Received stream is {52}
        Received message is {Procesare OpenCV completă! Fisier salvat pe server.}
[SERVER SPUNE]: Procesare OpenCV completă! Fisier salvat pe server.
===================================

[CLIENT] Trimit OPR_BYE si inchid programul.
*/

