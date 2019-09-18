
/* This defines the internal structure of record_ID field.
 * The two least significant bits
 * represents the index of this record in the second table.
 * The next 4 bits represents the record's index in the first table
 * The first two bits are currently unused
structure of record_ID
    uint8_t empty:2; // reserved for future use
    uint8_t first_index:4; // 4 bits that indicates the index of the record in the first table
    uint8_t second_index:2; // 2 bits that indicate the index of the record in the second table
*/

typedef struct record_list {
    int index;
    int occupied; // 0 means empty. 1 means in use
    long int client_ID;
    char name[255];
    struct record_list *next;
}record;


void createAlbum(int N, record *head_ref);
record* getRecord(record *head_ref, int index);
char* getRecordName(record *head_ref, int index);
void freeRecords(record *head_ref, int N);
void freeClientRecords(record *head_ref, long int client_tid);
void readClientRecords(record *head_ref, long int client_tid);
int storeRecord(record *head_ref, int index, char* name, long int tid);
record* getNextFreeRecord(record *head_ref);
record *records_head;