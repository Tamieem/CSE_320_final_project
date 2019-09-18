#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef struct art_entry{
    int index;
    uint8_t client_ID;
}entry;

typedef  struct table_two{
    int second_index; // will be 0, 4, 8, 12, etc...
    entry *entry1;
    entry *entry2;
    entry *entry3;
    entry *entry4;
}table_two;

typedef struct table_one{
    int first_index;
    table_two *entries;
    struct table_one *next;
}table_one;


void createNextTable(table_one *head_ref);
void freeTables(table_one *head_ref);
entry* getNextFreeEntry(table_one *head_ref);
int getNextFreeIndex(table_one *head_ref);
entry* getEntry(table_one *head_ref, int first_index, int second_index);
int getEntryIndex(table_one *head_ref, int first_index, int second_index);
int removeEntry(table_one *head_ref, int first_index, int second_index);
void listTableOne(table_one *head_ref);
void listTableTwo(table_one *head_ref, int first_index);
table_one *head_table;