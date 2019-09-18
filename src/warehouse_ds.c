#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "defs.h"
#include "warehouse_ds.h"



void createAlbum(int N, record *head_ref){
    record *temp = head_ref;
    for(int i = 0; i < N-1; i++){
        record *new_rec = (record *) malloc(sizeof(record));
        new_rec->next = NULL;
        temp->occupied =0;
        temp->index = i;
        temp->next = new_rec;
        temp = temp->next;
    }
    return;
}

record* getRecord(record *head_ref, int index){
    record *temp = head_ref;
    while(temp != NULL && temp->index != index)
        temp = temp->next;
    if (temp == NULL)
        return NULL;
    return temp;
}
char* getRecordName(record *head_ref, int index){
    char *name = malloc(sizeof(char)*255);
    record *temp = head_ref;
    while(temp != NULL && temp->index != index)
        temp = temp->next;
    if (temp == NULL){
        strcpy(name, "Error1: There was no record with the ID provided\n");
        return name;
    }
    if (temp->occupied == 0){
        strcpy(name, "Error2: This record is currently empty\n");
        return name;
    }
    strcpy(name, temp->name);
    strcat(name, "\n");
    return name;
}

void freeRecords(record *head_ref, int N){
    record *temp = head_ref;
    record *prev = temp;
    //for (int i = 0; i< N; i++){
      //  if(temp == NULL)
    while(temp != NULL){
        temp = temp->next;
        free(prev);
        prev = temp;
    }
    free(prev);
}

void freeClientRecords(record *head_ref, long int client_tid){
    record *temp = head_ref;
    while(temp != NULL){
        if(temp->client_ID == client_tid){
            temp->occupied = 0;
            temp->client_ID = 0;
        }
        temp = temp->next;
    }
}

void readClientRecords(record *head_ref, long int client_tid){
    record *temp = head_ref;
    while(temp != NULL) {
        if (temp->client_ID == client_tid){
            if(strcmp(temp->name, "") != 0)
                printf("Index #%i - %s\n", temp->index, temp->name);
            else
                printf("Index #%i - <Currently Empty>\n", temp->index);
        }
        temp = temp->next;
    }
}

int storeRecord(record *head_ref, int index, char* name, long int tid){
    record *temp = head_ref;
    while(temp != NULL && temp->index != index){
        temp = temp->next;
    }
    if(temp == NULL)
        return 0;

    if (temp->client_ID != tid && temp->client_ID != 0)
        return 2;

    if (temp->occupied == 1)
        return 3;

    temp->client_ID = tid; // stores clients id to record
    strcpy(temp->name, name);
    temp->occupied = 1;
    return 1;
}

record* getNextFreeRecord(record *head_ref){
    record *temp = head_ref;
    while(temp != NULL)
        if(temp->occupied == 0 && temp->client_ID == 0)
            break;
        else
            temp = temp->next;
    if (temp == NULL)
        return NULL;
    if (temp->occupied==0 && temp->client_ID ==0)
        return temp;
}
