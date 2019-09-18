#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

void main(){
    int client1, client2, client3, client4;
    client1 = mkfifo("client1", 0666);
    if (client1 <0){
        printf("Unable to create a client server\n");
        exit(1);
    }
    client2 = mkfifo("client2", 0666);
    if (client2 <0){
        printf("Unable to create a client server\n");
        exit(1);
    }
    client3 = mkfifo("client3", 0666);
    if (client3 <0){
        printf("Unable to create a client server\n");
        exit(1);
    }
    client4 = mkfifo("client4", 0666);
    if (client4 <0){
        printf("Unable to create a client server\n");
        exit(1);
    }
    int fifo_server;
    fifo_server = mkfifo("fifo_server", 0666);
    if (fifo_server <0){
        printf("Unable to create a client server\n");
        exit(1);
    }
}