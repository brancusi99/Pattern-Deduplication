#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "proto.h"

#define DEBUG
// Implementation of msgHeaderType peekMsgHeader (int sock)

/*
"dictionarul" aplicatiei TCP
cand trimiti date prin raw TCP, reteaua nu stie ce e un int, 
un char* sau o structura 
reteaua vede doar un sir continuu de octeți (bytes) 
rolul acestui fisier este sa impacheteze (serializeze) variabilele din C 
intr-un format binar standardizat inainte de a le pune pe cablu, si sa le 
despacheteze (deserializeze) la destinațtie
*/

msgHeaderType peekMsgHeader(int sock)
{ // Use this function to 'peek' into message structure. Take a look, it doesn't heart :)
    ssize_t bytes_read;
    msgHeaderType hdr;
    hdr.msgSize = htonl(sizeof(hdr));
    bytes_read = recv(sock, &hdr, sizeof(hdr), MSG_PEEK | MSG_WAITALL);
    // Mandatory conversions!
    hdr.msgSize = ntohl(hdr.msgSize);
    hdr.clientID = ntohl(hdr.clientID);
    hdr.opID = ntohl(hdr.opID);

    // End of mandatory conversions!
    if (bytes_read == -1)
    {
        hdr.opID = hdr.clientID = -1; // Something weird happened!
    }
    if (bytes_read == 0)
    {
        hdr.opID = hdr.clientID = OPR_BYE; // Connection closed for some reason. Just close it!
    }
#ifdef DEBUG
    fprintf(stderr, "\tReceived msgHeader: %d %d, %d (%ld)\n", hdr.msgSize, hdr.clientID, hdr.opID, bytes_read);
#endif
    return hdr;
}

int readSingleInt(int sock, msgIntType *msg_in)
{ // Simple read/write facilities for SingleInt
    size_t bytes_read;
    singleIntMsgType msg_struct;
    bytes_read = recv(sock, &msg_struct, sizeof(msg_struct), MSG_WAITALL);
    if (bytes_read <= 0)
    {
        msg_in->msg = -1;
        return -1;
    }
    msg_in->msg = ntohl(msg_struct.i.msg);
    return bytes_read;
}

int writeSingleInt(int sock, msgHeaderType hdr, int val)
{ // Build the message and send it!
    singleIntMsgType msg_struct;
    msg_struct.header.clientID = htonl(hdr.clientID);
    msg_struct.header.opID = htonl(hdr.opID);
    msg_struct.i.msg = htonl(val);
    msg_struct.header.msgSize = htonl(sizeof(msg_struct));
    ssize_t bytes_sent;
    bytes_sent = send(sock, &msg_struct, sizeof(msg_struct), 0);
    if (bytes_sent == -1)
    {
        // Something weird happened! Report and close
        return -1;
    }
    if (bytes_sent == 0)
    {
        // Cannot send! Connection close, Just report and close connection!
        return -1;
    }
    return bytes_sent;
}

int readMultiInt(int sock, msgIntType *msg1, msgIntType *msg2)
{ // Simple read/write facilities for SingleInt
    size_t bytes_sent;
    multiIntMsgType msg_struct;
    bytes_sent = recv(sock, &msg_struct, sizeof(msg_struct), MSG_WAITALL);
    if (bytes_sent <= 0)
    {
        msg1->msg = msg2->msg = -1;
        return -1;
    }
    msg1->msg = ntohl(msg_struct.i.msg1);
    msg2->msg = ntohl(msg_struct.i.msg2);
    return bytes_sent;
}

int writeMultiInt(int sock, msgHeaderType hdr, int val1, int val2)
{ // Build the message and send it!
    multiIntMsgType msg_struct;
    msg_struct.header.clientID = htonl(hdr.clientID);
    msg_struct.header.opID = htonl(hdr.opID);
    msg_struct.i.msg1 = htonl(val1);
    msg_struct.i.msg2 = htonl(val2);
    msg_struct.header.msgSize = htonl(sizeof(msg_struct));
    ssize_t bytes_sent;
    bytes_sent = send(sock, &msg_struct, sizeof(msg_struct), 0);
    if (bytes_sent == -1)
    {
        // Something weird happened! Report and close
        return -1;
    }
    if (bytes_sent == 0)
    {
        // Cannot send! Connection close, Just report and close connection!
        return -1;
    }
    return bytes_sent;
}

int readSingleString(int sock, msgStringType *str)
{   // Simple read/write facilities for SingleInt
    /* REDO readSingleSting as follows:
    Receive a singleIntFirst (with the size of your string).
    Receive the real string next. No Padding!
       */
    size_t bytes_sent;
    msgIntType size_msg;
    bytes_sent = readSingleInt(sock, &size_msg); // Skip the header....
    fprintf(stderr, "The string size was received: %d\n", size_msg.msg);
    str->msg = (char *)malloc(size_msg.msg + 1);
    bytes_sent = recv(sock, str->msg, size_msg.msg, MSG_WAITALL);
    fprintf(stderr, "\tReceived stream is {%ld}\n", bytes_sent);

    str->msg[size_msg.msg] = '\0';
    fprintf(stderr, "\tReceived message is {%s}\n", str->msg);
    return bytes_sent;
}

int writeSingleString(int sock, msgHeaderType hdr, char *str)
{
    /* REDO writeSingleString as follows:
    Send a SingleInteger first. The sent value is the string.
    Send the real string next. No Padding!
     */

    ssize_t bytes_sent;
    int strSize = strlen(str);
    bytes_sent = writeSingleInt(sock, hdr, strSize);
    if (bytes_sent == -1)
    {
        // Something weird happened! Report and close
        return -1;
    }
    if (bytes_sent == 0)
    {
        // Cannot send! Connection close, Just report and close connection!
        return -1;
    }

    fprintf(stderr, "\tSent size notification [%d]\n", strSize);
    bytes_sent = write(2, str, strSize);
    bytes_sent = send(sock, str, strSize, 0);
    fprintf(stderr, "|\t[%ld/%ld//%ld]\n", bytes_sent, sizeof(singleStringType), sizeof(msgStringType));
    return bytes_sent;
}
