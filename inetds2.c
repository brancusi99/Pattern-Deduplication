#include <pthread.h>
#include <stdio.h>
//#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/select.h>
#include <netinet/in.h>
//#include <netdb.h>
//#include <arpa/inet.h>
#include "proto.h"
#include "inttypes.h"
#include "time.h"
#include "sys/select.h"
#include "sys/wait.h"

int static inet_socket(uint16_t port, short reuse)
{
    int sock;
    struct sockaddr_in name;

    /* Create the socket. */
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        //    perror ("socket");
        pthread_exit(NULL);
    }

    if (reuse)
    {
        int reuseAddrON = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseAddrON, sizeof(reuseAddrON)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            pthread_exit(NULL);
        }
    }

    /* Give the socket a name. */
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0)
    {
        perror("bind");
        pthread_exit(NULL);
    }

    return sock;
}

int static create_client_id(void)
{
    /* Create some unique ID. e.g. UNIX timestamp... */
    char ctsmp[12];
    time_t rawtime;
    struct tm *timeinfo;
    int uuid;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(ctsmp, 12, "%s", timeinfo);

    uuid = atoi(ctsmp);
    /* NOTICE: YOU NEED TO STORE THIS INFORMATION AT SERVER LEVEL !*/
    return uuid;
}

//functia rulata de inetthr din server.c
void *inet_main(void *args)
{
    int port = *((int *)args);
    int sock;
    size_t size;
    fd_set active_fd_set;
    fd_set read_fd_set;
    struct sockaddr_in clientname;

    //serverul creeaza socketul, incepe sa asculte si adauga acest socket 
    //principal intr-o colectie (active_fd_set)

    if ((sock = inet_socket(port, 1)) < 0)
    {
        pthread_exit(NULL);
    }
    if (listen(sock, 1) < 0)
    {
        pthread_exit(NULL);
    }
    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);

    //in loc sa blocheze threadul asteptand un client (accept())
    //sau asteptand date de la un client deja conectat (recv())
    //select() monitorizeaza toate conexiunile simultan 
    //asteapta pana cand orice file descriptor (socket) din lista spune ceva

    while (1)
    {
        int cnt;
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            pthread_exit(NULL);
        }

        /* Service all the sockets with input pending. */
        for (cnt = 0; cnt < FD_SETSIZE; ++cnt){
            //daca un socket e marcat FD_ISSET, inseamna ca are date
            if (FD_ISSET(cnt, &read_fd_set))
            {
                //daca este socketul principal (i==sock) 
                //are o conexiune noua
                //serverul apelează accept(), obtine un socket nou pentru 
                //acest client si il adauga in lista de ascultare (FD_SET)
                if (cnt == sock)
                { /* Connection request on original socket. */
                    int new;
                    size = sizeof(clientname);
                    new = accept(sock, (struct sockaddr *)&clientname,
                                 (socklen_t *)&size);
                    if (new < 0)
                    {
                        pthread_exit(NULL);
                    }

                    FD_SET(new, &active_fd_set);
                }
                //client deja conectat a trimis date
                else
                { /* Data arriving on an already-connected socket. */
                    int clientID;
                    /* Protocolul simplu utilizat:
                    Se verifica intai header-ul.
                    Pe baza informatiei din header, se decide operatia de urmat

                    */
                    msgHeaderType hdr = peekMsgHeader(cnt);
                    if ((clientID = hdr.clientID) < 0)
                    {
                        // Protocol error: missing client ID. Close connection
                        fprintf(stderr, "There's something wrong! Negative ClientID.\t Closing connection, probably the client was terminated.\n");
                        close(cnt);
                        FD_CLR(cnt, &active_fd_set);
                    }
                    else
                    {
                        //daca id-ul este 0, genereaza un id si il trimite inapoi clientului
                        if (clientID == 0)
                        {
                            int newID;
                            msgIntType msg;
                            newID = create_client_id();
                            fprintf(stderr, "\tDetected new client! New clientID: %d\n", newID);
                            if (readSingleInt(cnt, &msg) < 0)
                            {
                                // Cannot read from client. This is impossible :) Close connection!
                                close(cnt);
                                FD_CLR(cnt, &active_fd_set);
                            }
                            if (writeSingleInt(cnt, hdr, newID) < 0)
                            {
                                // Cannot write to client. Close connection!
                                close(cnt);
                                FD_CLR(cnt, &active_fd_set);
                            }
                        }
                        else
                        { /* Already identified. Existing client... communication continues */
                            /* YOU SHOULD CHECK IF THIS IS AN EXISTING CLIENT !!! */
                            int operation;//, dsize;

                            operation = hdr.opID;
                            if (operation == -1)
                            { // Protocol error: missing or incorect operation
                                close(cnt);
                                FD_CLR(cnt, &active_fd_set);
                            }


                        //OPR_BYE: serverul inchide socketul (close(i)) si il scoate din lista (FD_CLR)
                            switch (operation)
                            {
                            case OPR_DEDUP_START:
                                {
                                    msgStringType input_dir;
                                    //citeste numele folderului trimis de client (ex: "client_files/batch1")
                                    if (readSingleString(cnt, &input_dir) < 0) {
                                        close(cnt); FD_CLR(cnt, &active_fd_set);
                                    }
                                    
                                    char output_dir[256] = "serv_files/output_dedup"; //aici salevaza rezultatul

                                    pid_t pid = fork();
                                    if (pid == 0) {
                                        //procesul anseaza aplicatia C++ 
                                        char *args[] = {"./cv_worker", input_dir.msg, output_dir, NULL};
                                        execv(args[0], args);
                                        
                                        //execv a esuat 
                                        perror("Eroare la lansarea cv_worker");
                                        exit(EXIT_FAILURE);
                                    } else if (pid > 0) {
                                        //procesul parinte asteapta terminarea
                                        int status;
                                        waitpid(pid, &status, 0);

                                        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                                            writeSingleString(cnt, hdr, "Procesare OpenCV completă! Fisier salvat pe server.");
                                        } else {
                                            writeSingleString(cnt, hdr, "Eroare in procesarea OpenCV.");
                                        }
                                    }
                                    free(input_dir.msg);
                                }
                                break;

                            case OPR_BYE:
                            default:
                                close(cnt);
                                FD_CLR(cnt, &active_fd_set);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    pthread_exit(NULL);
}
