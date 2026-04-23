#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
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
        fprintf(stderr, "Unknown host %s.\n", hostname);
        exit(EXIT_FAILURE);
    }
    name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
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
    msgHeaderType h;
    msgIntType m;
    msgStringType str;
    int clientID = 0;

    h.clientID = 0;
    h.opID = 0; 
    writeSingleInt(sock, h, 0); 
    readSingleInt(sock,  &m);   
    clientID = m.msg;
    fprintf(stderr, "[CLIENT] M-am conectat! Am primit ID-ul: %d\n\n", clientID);

    //Deduplicare    
    //stabileste folderul pe care urm sa-l proceseze
    //daca se ruleaza "./inetclient folder_test", ia argumentul, altfel ia o valoare default
    char *input_folder = "imagini_input"; 
    if (argc > 1) {
        input_folder = argv[1];
    }

    h.clientID = clientID;
    h.opID = OPR_DEDUP_START; 

    fprintf(stderr, "=== START PATTERN DEDUPLICATION ===\n");
    fprintf(stderr, "[CLIENT] Trimit cererea pentru folderul: '%s'\n", input_folder);

    //trimite numele folderului ca string catre server
    if (writeSingleString(sock, h, input_folder) < 0) {
        fprintf(stderr, "[CLIENT] Eroare la trimiterea cererii!\n");
    } else {
        fprintf(stderr, "[CLIENT] Cerere trimisa! Astept ca serverul (C++) sa proceseze imaginile...\n");

        //clientul ingheata pana cand programul C++ (cv_worker) termina
        if (readSingleString(sock, &str) > 0) {
            fprintf(stderr, "[SERVER SPUNE]: %s\n", str.msg);
            //elibereaza memoria stringului primit
            free(str.msg); 
        } else {
            fprintf(stderr, "[CLIENT] Eroare! Serverul a inchis brusc conexiunea.\n");
        }
    }
    fprintf(stderr, "===================================\n\n");

    //inchide conexiunea
    h.clientID = clientID;
    h.opID = OPR_BYE;
    fprintf(stderr, "[CLIENT] Trimit OPR_BYE si inchid programul.\n");
    writeSingleInt(sock, h, 0); 

    close(sock);
    exit(EXIT_SUCCESS);
}
