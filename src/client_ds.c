#include "client_ds.h"
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdint.h>
#include <pthread.h>
#include <fcntl.h>
#include "defs.h"



void createNextTable(table_one *prev){
    table_one *temp;
    temp = (table_one *)malloc(sizeof(table_one));
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
    temp->entries = entries;
    temp->first_index = 0;
    temp->next = NULL;
    prev->next = temp;
}

void freeTables(table_one *head_ref){ // also be sure to send "free client_tid" to server to empty those records
    table_one *temp = head_ref;
    table_one *prev = temp;
    while(temp != NULL){
        prev = temp;
        temp = temp->next;
        table_two *temp_two = prev->entries;
        free(temp_two->entry1);
        free(temp_two->entry2);
        free(temp_two->entry3);
        free(temp_two->entry4);
        free(temp_two);
        free(prev);
    }
}


entry* getNextFreeEntry(table_one *head_ref){
    table_one *temp = head_ref;
    int i=0;
    int j=0;
    table_one *prev = temp;
    while(temp != NULL){
        table_two *entries = temp->entries;
        if (entries->entry1->index == -1){
            return entries->entry1;
        }
        else if(entries->entry2->index == -1){
            return entries->entry2;
        }
        else if (entries->entry3->index == -1){
            return entries->entry3;
        }
        else if (entries->entry4->index == -1){
            return entries->entry4;
        }
        else{
            prev = temp;
            temp = temp->next;
            i++;
        }
    }
    if(temp == NULL){
        createNextTable(prev); // creates a new table if table is all full
        prev->next->first_index = prev->first_index+1;
        prev->next->entries->second_index = prev->next->first_index*4;
        return getNextFreeEntry(head_table); // reruns program with new table added
    }
}

int getNextFreeIndex(table_one *head_ref){
    table_one *temp = head_ref;
    int i=0;
    int j=0;
    table_one *prev = temp;
    while(temp != NULL){
        table_two  *entries = temp->entries;
        if (entries->entry1->index == -1){
            int j = 0;
            break;
        }
        else if(entries->entry2->index == -1){
            int j = 1;
            break;
        }
        else if (entries->entry3->index == -1){
            int j = 2;
            break;
        }
        else if (entries->entry4->index == -1){
            int j = 3;
            break;
        }
        else{
            prev = temp;
            temp = temp->next;
            i++;
        }
    }
    if(temp == NULL){
        createNextTable(prev); // creates a new table if table is all full
        prev->next->first_index = prev->first_index+1;
        prev->next->entries->second_index = prev->next->first_index*4;
        return getNextFreeIndex(head_table); // reruns program with new table added
    }
    int index = 4*i +j;
    return index;
}

entry* getEntry(table_one *head_ref, int first_index, int second_index){
    table_one *temp = head_ref;
    for(int i =0; i < first_index; i++){
        if(temp == NULL){
            return NULL;
        }
        temp = temp->next;
    }
    if (temp == NULL){
        return NULL;
    }
    switch (second_index){
        case 0:
            return temp->entries->entry1;

        case 1:
            return temp->entries->entry2;

        case 2:
            return temp->entries->entry3;

        case 3:
            return temp->entries->entry4;

        default:
            return NULL;
    }
}


int getEntryIndex(table_one *head_ref, int first_index, int second_index){
    entry *temp = getEntry(head_ref, first_index, second_index);
    if (temp == NULL)
        return -2;
    else
        return temp->index;
}

int removeEntry(table_one *head_ref, int first_index, int second_index){
    entry *temp = getEntry(head_ref, first_index, second_index);
    if (temp == NULL)
        return -2;
    int i = temp->index;
    temp->index = -1;
    return i;
}

void listTableOne(table_one *head_ref){
    table_one *temp = head_ref;
    int i =0;
    while(temp != NULL){
        printf("Table #%i: ", i);
       // printf("%s", temp);
        printf("\n");
        i++;
        temp = temp->next;
    }
}

void listTableTwo(table_one *head_ref, int first_index){
    table_one *temp = head_ref;
    while(temp != NULL && temp->first_index != first_index){
        temp = temp->next;
    }
    if(temp == NULL){
        printf("Could not find that table, try again \n");
        return;
    }
    table_two *temp_two = temp->entries;
    printf("ID %i: Remote Index: %i\n", first_index*4+0, temp_two->entry1->index);
    printf("ID %i: Remote Index: %i\n", first_index*4+1, temp_two->entry2->index);
    printf("ID %i: Remote Index: %i\n", first_index*4+2, temp_two->entry3->index);
    printf("ID %i: Remote Index: %i\n", first_index*4+3, temp_two->entry4->index);
}