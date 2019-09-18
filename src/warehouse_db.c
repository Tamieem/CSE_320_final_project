#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include "defs.h"
#include "warehouse_ds.h"





int server;
int clients[4]; // list of client pipes
long int clients_tid[4]; // list of client's tids
int new_exit(char** args);
int open_client(char** args);
int alloc(char** args);
int dealloc(char** args);
int read_ID(char** args);
int dump(char** args);
int list(char** args);
int close_session(char** args);
int store(char** args);
void shell(int signum);
char** parseLine(char* line);
int N=0; // Number of total records
char* buf;




char *client_str[] = {
        "open",
        "alloc",
        "dealloc",
        "read",
        "store",
        "close"
};
int(*client_func[])(char **) = {
        &open_client, // opens clients
        &alloc,
        &dealloc,
        &read_ID,
        &store, // Stores the art
        &close_session // free all records pertaining to ID of client that has closed
};



char *builtin_str[] = {
        "exit",
        "list",
        "dump"
};
int(*builtin_func[])(char **) = {
        &new_exit,
        &list,
        &dump
};


int alloc(char** args){ // args will be "'alloc' 'client_No' 'client_ID'"
    int client_no = atoi(args[1])-1;
    long int client_tid = atol(args[2]);
    printf("Recieved alloc request from client %i\n", atoi(args[1]));
    record *temp;
    temp = getNextFreeRecord(records_head);
    /* Now write back to client index of allocated record */
    char* resp = malloc(sizeof(char)*10);
    if (temp == NULL){
        int x = -1;
        snprintf(resp, 10, "%i ", x);
        write(clients[client_no], resp, sizeof(char)*10);
        free(resp);
        return 1;
    }
    temp->client_ID = client_tid;
    snprintf(resp, 10, "%i ", temp->index);
    write(clients[client_no], resp, sizeof(char)*10);
    free(resp);
    return 1;
}

int dealloc(char** args){ // args will be "'dealloc' 'index' 'client_No'"

    int client_no = atoi(args[2])-1;
    int index = atoi(args[1]);
    printf("Received dealloc request from client %i \n", atoi(args[2]));
    record *temp = getRecord(records_head, index);
    char* resp = malloc(sizeof(char)*250);
    if (temp == NULL){
        strcpy(resp, "Index not found\n");
        write(clients[client_no], resp, sizeof(char)*250);
        free(resp);
        sleep(1);
        return  1;
    }
    temp->occupied =0;
    temp->client_ID = 0;
    strcpy(resp, "Index dealloc'd\n");
    write(clients[client_no], resp, sizeof(char)*250);
    free(resp);
    sleep(1);
    return 1;
}

int read_ID(char** args){ // args will be "'read' 'index' 'client_No'"
    printf("Received name request from client %s \n", args[2]);
    int index = atoi(args[1]);
    char *name;
    name = getRecordName(records_head, index);
    int client_no = atoi(args[2])-1;
    write(clients[client_no], name, sizeof(char)*255);
    printf("Sent name to client\n");
    free(name);
    return 1;
}

int store(char** args){ // args will be "'store' 'index' 'name Z' 'client_No'"
    int client_no = atoi(args[3]) - 1;
    printf("Received store request from client %i\n", atoi(args[3]));
    char* name = malloc(sizeof(char)*255);
    strcpy(name, args[2]);
    int index = atoi(args[1]);
    char* resp = malloc(sizeof(char)*255);
    int x = storeRecord(records_head, index, name, clients_tid[client_no]);
    free(name);
    if (x == 0)
        strcat(resp, "Could not find record with matching ID\n");
    else if (x == 2)
        strcat(resp, "Record is occupied by a different client.\n");
    else if (x == 3)
        strcat(resp, "Record is already occupied\n");
    else{
        strcat(resp, "Stored record in database\n");
    }
    write(clients[client_no], resp, sizeof(char)*255);
    free(resp);

}

int close_session(char** args){ // args will be "close 'client_No' 'client_tid"
    int client_id = atoi(args[1])-1;
    printf("Received close request from client %i\n", atoi(args[1]));
    long int client_tid = atol(args[2]);
    freeClientRecords(records_head, client_tid); // frees all records pertaining to the closing client
    char* resp = malloc(sizeof(char)*255);
    strcpy(resp, "Closed Client\n");
    write(clients[client_id], resp, sizeof(char)*255);
    close(clients[client_id]);
    clients[client_id] = 0;
    clients_tid[client_id]=0;
    printf("Client %i has terminated\n", atoi(args[1]));
    free(resp);
    sleep(1);
//    if(client_tid == clients_tid[client_id]) { // if you open multiple clients on the same pipe, it will only close the full session if the most recent pipe closes it
//        close(clients[client_id - 1]); // closes client pipe
//        clients[client_id - 1] = 0;
//        clients_tid[client_id - 1] = 0;
//    }
}

int new_exit(char** args){
    close(server);
    free(args);
    freeRecords(records_head, N);
    free(buf);
    exit(1);
}

int listX(char** args){
    int X = atoi(args[1]);
    if (X < 1){
        printf("Could not read which client number you wanted to list.\n Re-Enter 'list X' with X being [1,4]");
        return 1;
    }
    int tid = clients_tid[X-1];
    readClientRecords(records_head, tid);
}

int list(char** args){
    if (args[1] != NULL){
        return listX(args);
    }
    printf("Currently Active Clients: \n");
    for(int i = 0; i <4; i++) { // there's 4 max clients
        if (clients[i] > 0){
            printf("Client No.%i ID = %li\n", i+1, clients_tid[i]);
        }
    }
    return 1;
}



int dump(char** args){
    record *temp = records_head;
    while(temp != NULL){
        if (temp->occupied == 1) {
            printf("Index: %i\n", temp->index);
            printf("Name: %s\n", temp->name);
            printf("Belongs to Client ID: %li\n\n", temp->client_ID);
        }
        temp = temp->next;
    }

}


int open_client(char** args){ // args will be "'open' 'client_Name' 'client No' 'client_tid'"
    printf("Received connection request from client%s \n", args[2]);
    int client_no = atoi(args[2])-1; // -1 because index of array is one less
    clients[client_no] = open(args[1], O_RDWR); // saves client pipe number to array
    clients_tid[client_no] = atol(args[3]); // saves client tid to array
    if (clients[client_no] < 1){
        cse320_print("Connection to client failed \n");
        close(clients[client_no]);
        return 1;
    }
    cse320_print("Connected to client\n");
    return 1;
}
int main(int argc, char** argv){
    signal(SIGINT, shell);
    signal(SIGSTOP, shell);
    if(argc < 2){
        cse320_print("You forgot to enter the number of records to store in the server.\n");
        return 1;
    }
    // argv[1] should have N
    N = atoi(argv[1]);
    char** line_args;
    if (N < 1){
        cse320_print("Please enter a valid number when running the program \n");
        return 1;
    }
    records_head = (record *) malloc(sizeof(record));
    createAlbum(N, records_head); // Function that will malloc enough room for N records
    server = open("fifo_server", O_RDWR); // opens server pipe
    cse320_print("Opened server\n");
//    buf = malloc(sizeof(char)*255);
//    read(server, buf, (sizeof(char)*255));
//    printf("Server is now connected to %s \n", buf);
//    free(buf);
    while(1){
        // Split buf into string array on space
        // read the first command and decided what to do with it
        // will need multiple functions to figure out what to do and call them
        // Just like shell commands but reading from pipe instead of command line
        buf = malloc(sizeof(char)*600);
        read(server, buf, (sizeof(char)*600));
        sleep(1);
        line_args = parseLine(buf);
        for(int i=0; i<6;i++){ // There are 6 commands to read from client
            if(strcmp(line_args[0], client_str[i]) == 0)
                (*client_func[i])(line_args);

        }
        free(line_args);
        free(buf);
        sleep(1);
    }

}


char** parseLine(char* line){
    char **strings = malloc(255*sizeof(char*)); // command will not be no longer than "add art name size price"
    char *string;
    string = (char*)strtok(line, " ");
    int i =0;
    size_t len;
    char* delim = " ";
    while(string != NULL){
        len = strlen(string);
        if (string[len-1] == '\n')
            string[len-1] = '\0';
        strings[i] = string;
        i++;
        string = (char *)strtok(NULL, " ");
    }
    strings[i] = NULL;
    return strings;
}


void shell(int signum) {
    signal(SIGSTOP, shell);
    signal(SIGINT, shell);
    char* line; // Reading user commands
    char** line_args;
    int boolcheck;// opens the doors to all the museums
    printf("shell>  "); // User enters a command here
    // line = malloc(sizeof(char)*255);
    ssize_t bufsize = 0;
    line = NULL;
    getline(&line, &bufsize, stdin); // Line becomes whatever that is inputted by user
    line_args = parseLine(line);
    for(int i=0; i < 3; i++) { // There are 3 builtin commands
        if (strcmp(line_args[0], builtin_str[i]) == 0) {
            free(line);
            (*builtin_func[i])(line_args);
            break;
        }
    }
    free(line);
    free(line_args);

}