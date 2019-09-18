#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "defs.h"
#include "client_ds.h"

FILE file_server;
FILE file_client;
pthread_t tid_t;
long int tid;
char *client_file;
int client_no = 0;
int server = 0;
int client = 0;
int new_exit(char** args);
char** parseLine(char* line);
int start(char** args);
int alloc(char** args);
int dealloc(char** args);
int read_ID(char** args);
int store(char** args);
int close_session(char** args);
int infotab(char** args);
int cacheCheck(int index);
void cacheEviction(int index, char *name);
int cacheStore(int index, char* name);
int removeCache(int index);

typedef struct remote_cache{
    int index[4];
    char name[4][255];
}cache;

cache *block;

/* Builtin commands */
char *builtin_str[] = {
        "exit",
        "start",
        "alloc",
        "dealloc",
        "read",
        "store",
        "close",
        "infotab"
};


int(*builtin_func[])(char **) = {
        &new_exit,
        &start,
        &alloc,
        &dealloc,
        &read_ID,
        &store,
        &close_session,
        &infotab
};
void *session_thread(void *vargp){
    for(;;)
        ;
}
int new_exit(char** args){
    if(client > 0) {
        printf("Please close the session first before exiting.\n");
        return 1;
    }
    exit(1);
}

int alloc(char** args){ // create "alloc client_No client_tid"
    if (client < 1){
        cse320_print("Please first start a session \n");
        return 1;
    }
    /* Write to server alloc command */
    char* cmd = malloc(sizeof(char)*600);
    strcpy(cmd, args[0]);
    strcat(cmd, " ");
    char* strclient = malloc(sizeof(char)*10);
    snprintf(strclient, 10, "%d ", client_no);
    strcat(cmd, strclient);
    char* strtid = malloc(sizeof(char)*20);
    snprintf(strtid, 20, "%ld ", tid);
    strcat(cmd, strtid);
    write(server, cmd, sizeof(char)*600);
    sleep(2);
    free(strclient);
    free(strtid);
    free(cmd);

    /* Read back the index that server sends back and saves to table */
    char* resp = malloc(sizeof(char)*10);
    read(client, resp, sizeof(char)*10);
    int index = atoi(resp);
    if (index < 0){
        printf("There are no more empty records available \n");
        free(resp);
        return 1;
    }
    entry *new_entry = getNextFreeEntry(head_table);
    int id = getNextFreeIndex(head_table);
    new_entry->index = index;
    printf("Stored server index %i in the table \n", index);
    free(resp);
    return 1;
}

int dealloc(char** args){ // "dealloc ID"
    if (client < 1){
        printf("Please first start a session \n");
        return 1;
    }
    if (strcmp(args[1],"") == 0){
        printf("You did not enter an ID to dealloc, \n");
        return 1;
    }
    int id = atoi(args[1]);
    if(id ==0 ){
        if(strcmp(args[1], "0") == 0);
        else {
            printf("The ID you entered was inputted incorrectly, please try again\n");
            return 1;
        }
    }
    int first_index = id/4;
    int second_index = id%4;
    int index = removeEntry(head_table, first_index, second_index);
    if (index == -2){
        printf("Couldn't find the ID specified, please try again with a different (read: valid) ID\n ");
        return 1;
    }
    if (index == -1){
        printf("Entry is already empty");
        return 1;
    }
    removeCache(index);
    /* Write "dealloc index client_No" to server */
    char* cmd = malloc(sizeof(char)*600);
    char* strindex = malloc(sizeof(char)*10);
    char* strno = malloc(sizeof(char)*10);
    strcpy(cmd, args[0]);
    snprintf(strindex, 10, " %i ", index);
    strcat(cmd, strindex);
    snprintf(strno, 10, "%i ", client_no);
    strcat(cmd, strno);
    write(server, cmd, sizeof(char)*600);
    sleep(1);
    free(cmd);
    free(strindex);
    free(strno);

    char* resp = malloc (sizeof(char)*250);
    read(client, resp, sizeof(char)*250);
    cse320_print(resp);
    free(resp);
    return 1;
}

int read_ID(char** args){
    if(client < 1){
        cse320_print("Please first start a session \n");
        return 1;
    }
    if (strcmp(args[1], "") == 0){ // if user forgot to enter an ID
        cse320_print("You did not enter an ID to read. \n");
        return 1;
    }
    int id = atoi(args[1]);
    if(id ==0) {
        if (strcmp(args[1], "0")==0);
        else {
            printf("The ID you entered was inputted incorrectly, please try again\n");
            return 1;
        }
    }
    int first_index = id/4;
    int second_index = id%4;
    int index = getEntryIndex(head_table, first_index, second_index);
    if (index == -2){
        printf("The ID you entered is invalid, please try again \n");
        return 1;
    }
    if (index == -1){
        printf("The ID you entered has not yet been allocated on the server \n");
    }
    int check = cacheCheck(index);
    if(check == 1){ // cache hit
        return 1;
    }
    /* Creating command for server to read */
    char* cmd = malloc(sizeof(char)* 600);
    char* strclient = malloc(sizeof(char)*10);
    char* strindex = malloc(sizeof(char)*10);
    strcpy(cmd, args[0]);
    snprintf(strindex, 10, " %i ", index);
    strcat(cmd, strindex);
    snprintf(strclient, 10, "%i ", client_no);
    strcat(cmd, strclient);
    // This ensures that the command sent to server will be "read 'index' 'client_No'" for the server to read and register properly
    write(server, cmd, sizeof(char)*600);
    sleep(1);
    free(cmd);
    free(strclient);
    free(strindex);

    /* reading what server sends back to client */
    cmd = malloc(sizeof(char)*255);
    read(client, cmd, sizeof(char)*255);// eviction or cache storage
    char **cmd_args;
    cmd_args = parseLine(cmd);
    if ((strcmp(cmd_args[0], "Error1:") != 0) && (strcmp(cmd_args[0], "Error2:") != 0)) {
        if (check == 2)
            cacheEviction(index, cmd);
        else
            cacheStore(index, cmd);
        printf("%s\n", cmd);
    }
    if(strcmp(cmd_args[0], "Error1:") ==0)
        strcpy(cmd, "There was no record with the ID provided\n");
    else
        strcpy(cmd, "This record is currently empty\n");
    printf("%s", cmd);
    free(cmd);
    free(cmd_args);
    return 1;
}
int removeCache(int index){
    for (int i = 0; i < 4; i++){
        if (block->index[i] == index){
            block->index[i] = -1;
            strcpy(block->name[i], "");
            return 1;
        }
    }
    return 0;
}
int cacheCheck(int index){
    for(int i = 0; i <4; i++){
        if(block->index[i] == index){
            printf("cache hit\n");
            printf("%s\n", block->name[i]);
            return 1;
        }
    }
    printf("cache miss\n");
    for(int i =0; i<4; i++){
        if (block->index[i] == -1)
            return 3; // have space to store names and values
    }
    return 2; // eviction
}

int cacheStore(int index, char *name){
    for(int i = 0; i < 4; i++){
        if(block->index[i] == -1){
            block->index[i] = index;
            strcpy(block->name[i], name);
            return 1;
        }
    }
    return 0;
}

void cacheEviction(int index, char* name){
    printf("eviction\n");
    int i = rand() % 4;
    block->index[i] = index;
    strcpy(block->name[i],name);
}

int store(char** args){ // command will be "'store' 'index' 'name Z' 'client_No'"
    if (client == 0){
        cse320_print("You haven't started a new session yet\n");
        return 1;
    }
    int client_id =  atoi(args[1]);
    if(client_id == 0 ) {
        if (strcmp(args[1], "0")==0);
        else {
            printf("The ID you entered was inputted incorrectly, please try again\n");
            return 1;
        }
    }
    int first_index = client_id/4;
    int second_index = client_id%4;
    int id = getEntryIndex(head_table, first_index, second_index);
    if (id == -2){
        printf("Entry ID not found in this client\n");
        return 1;
    }
    if (id == -1){
        printf("This entry ID has not yet been indexed on the server\n");
        return 1;
    }
    char* cmd = malloc(sizeof(char)*600);
    strcpy(cmd, args[0]); // Store
    char* index = malloc(sizeof(char)*10);
    snprintf(index, 10, " %d ",id);
    strcat(cmd, index); // index
    strcat(cmd, args[2]); // name Z
    char* strclient = malloc(sizeof(char)*10);
    snprintf(strclient, 10, " %d", client_no);
    strcat(cmd, strclient);
    write(server, cmd, sizeof(char)*600); // "store index Z client_No"
    sleep(1);
    free(cmd);
    free(strclient);
    free(index);

    /* reading back server's response */
    cmd = malloc(sizeof(char)*400);
    read(client, cmd, sizeof(char)*400);
    if (cmd[0] == 'S')
        cacheStore(id, args[2]);
    cse320_print(cmd);
    free(cmd);
    return 1;
}


int close_session(char** args){
    if (client == 0){
        cse320_print("There is no session to close\n");
        return 1;
    }
    /* Create command that sends to server to free all records pertaining to this client */
    char* cmd = malloc(sizeof(char)* 600);
    char* strclient = malloc(sizeof(char)*30);
    strcpy(cmd, args[0]); // "close"
    strcat(cmd, " "); // " "
    snprintf(strclient, 30, "%d %ld", client_no, tid); // prints client's number and tid to command message
    strcat(cmd, strclient); // "close 'client_No' 'client_tid'"
    write(server, cmd, sizeof(char)*600); // sends to server
    sleep(2);
    pthread_cancel(tid_t);
    free(cmd);
    free(strclient);

    /* reading what server sends back to client */
    cmd = malloc(sizeof(char)*255);
    read(client, cmd, sizeof(char)*255);
    cse320_print(cmd);
    free(cmd);

    /* closes files */
    close(client);
    close(server);
    free(client_file);
    free(block);
    client = 0;
    server = 0;
}

int infotab(char** args){
    if (client == 0){
        cse320_print("There is no session to view data\n");
        return 1;
    }
    listTableOne(head_table);
    char *buf;
    printf("Select one of these table numbers to view their entries: ");
    ssize_t bufsize=0;
    getline(&buf, &bufsize, stdin);
    int index = atoi(buf);
    if(index < 0){
        printf("What you entered was read incorrectly, please try again\n");
        return 1;
    }
    listTableTwo(head_table, index);
    free(buf);
    return 1;

}

int start(char** args){
    char *buf;
    if (client > 0){ // If user were to input start again without closing
        cse320_print("A session has already been started, please close it before starting a new one.\n");
        return 1;
    }
    client_file = malloc(sizeof(char)*10);
    ssize_t bufsize=0;
    cse320_print("Please enter the client number you wish to start: (1-4) ");
    getline(&buf, &bufsize, stdin);
    strcpy(client_file, buf);
    free(buf);
    client_no = atoi(client_file);
    char *client_name = malloc(sizeof(char)*10);
    switch(client_no){
        case 1:
            client = open("client1",O_RDWR);
            strcpy(client_name, "client1");
            break;
        case 2:
            client = open("client2",O_RDWR);
            strcpy(client_name, "client2");
            break;
        case 3:
            client = open("client3",O_RDWR);
            strcpy(client_name, "client3");
            break;
        case 4:
            client = open("client4",O_RDWR);
            strcpy(client_name, "client4");
            break;
        default:
            cse320_print("There was an error in reading client number, retry the command with a number from 1-4 \n");
            free(client_name);
            free(client_file);
            return 1;
    }

    cse320_print("Establishing connection with server \n");
    server = open("fifo_server",O_RDWR);

    if(server < 0) {
        cse320_print("There was a error in opening the server file\n");
        free(client_name);
        free(client_file);
        close(client);
        close(server);
        return 1;
    }
    // CREATE THREAD HERE OR SOMESHIT
    pthread_create(&tid_t, NULL, session_thread, (void *) NULL); // thread to simulate a started session
    pthread_detach(tid_t);
    tid = tid_t;

    /* Initializing cache block */
    block = (cache *) malloc(sizeof(cache));
    for(int i =0; i <4; i++) block->index[i] = -1;
    /* Set to -1 to indicate they are empty, as 0 is an actual index */

    /* Create head table */
    head_table = (table_one *)malloc(sizeof(table_one));
    table_two *entries = (table_two *) malloc(sizeof(table_two));
    entry *entry1 = (entry *) malloc(sizeof(entry));
    entry1->index = -1;
    entry *entry2 = (entry *) malloc(sizeof(entry));
    entry2->index = -1;
    entry *entry3 = (entry *) malloc(sizeof(entry));
    entry3->index = -1;
    entry *entry4 = (entry *) malloc(sizeof(entry));
    entry4->index = -1;
    entries->entry1 = entry1;
    entries->entry2 = entry2;
    entries->entry3 = entry3;
    entries->entry4 = entry4;
    head_table->entries = entries;
    head_table->first_index = 0;
    head_table->next = NULL;
    /* create server message */
    buf=malloc(450*sizeof(char));
    strcpy(buf, "open ");
    strcat(buf, client_name);
    strcat(buf, " ");
    strcat(buf, client_file);
    char* strtid = malloc(20* sizeof(char));
    snprintf(strtid, 20, " %ld", tid);
    strcat(buf, strtid);
    // Saves "'open' 'client_file' 'clientNo.' 'client_tid" to buffer and writes it to server
    write(server,buf,450*sizeof(char)); // Will now write which client this remote will now open up
    printf("Wrote to the server \n");
    free(strtid);
    free(buf);
    free(client_name);
}


char** parseLine(char* line){
    char **strings = malloc(255*sizeof(char*)); // command will not be no longer than 255 characters
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


int main(void) {
    char* line; // Reading user commands
    char** line_args;
    int boolcheck;// opens the doors to all the museums

    while(1) {
        printf("shell>  "); // User enters a command here
        // line = malloc(sizeof(char)*255);
        ssize_t bufsize = 0;
        line = NULL;
        getline(&line, &bufsize, stdin); // Line becomes whatever that is inputted by user
        line_args = parseLine(line);
        for(int i=0; i<8;i++){ // There are 8 builtin commands
            if(strcmp(line_args[0], builtin_str[i]) == 0)
                (*builtin_func[i])(line_args);

        }
        free(line);
        free(line_args);
    }

}