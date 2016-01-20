#include "lzw.h"

lzw_dict * lzw_dict_init(char *alphabet)
{
    lzw_dict *dict = (lzw_dict *) malloc(sizeof(lzw_dict));
    dict->head = NULL;
    dict->tail = NULL;
    dict->count = 0;
    dict->bits = 1;
    dict->next_bit = 2;
    int i = 0;
    for (; i < strlen(alphabet); i++) {
        char *c = malloc(2);
        *c = alphabet[i];
        c[1] = '\0';
        lzw_dict_add(dict, c);
    }
    return dict;
}

void lzw_dict_add(lzw_dict *dict, char *seq)
{
    // Create new entry
    lzw_dict_entry *entry = (lzw_dict_entry *)malloc(sizeof(lzw_dict_entry));
    entry->seq = seq;
    entry->code = ++(dict->count);
    entry->next = NULL;
    entry->length = strlen(seq);

    // Insert entry into head of linked-list
    if (dict->tail == NULL) {
        dict->tail = entry;
        entry->next = NULL;
    } else
        entry->next = dict->head;
    dict->head = entry;

    if (dict->count >= dict->next_bit) {
        dict->bits++;
        dict->next_bit *= 2;
    }
}

void lzw_dict_free(lzw_dict *dict)
{
    lzw_dict_entry *curr = dict->head;
    lzw_dict_entry *next;
    while (curr != NULL) {
        next = curr->next;
        free(curr->seq);
        free(curr);
        curr = next;
    }
    free(dict);
}

void lzw_dict_print(lzw_dict *dict)
{
    lzw_dict_entry *curr = dict->head;
    printf("HEAD");
    while (curr != NULL) {
        printf(" - %s", curr->seq);
        curr = curr->next;
    }
    printf("\n");
}

char * lzw_encode(lzw_dict *dict, char *seq, data *buffer)
{
    int len; // Insert length 1 into dict
    lzw_dict_entry *curr = dict->head;
    // Find longest match in the dictionary
    while (curr != NULL) {
        if (strncmp(curr->seq, seq, curr->length) == 0) {
            len = curr->length + 1;
            break;
        }
        curr = curr->next;
    }
    // Exit if character at seq[0] not in initial dictionary.
    if (curr == NULL) {
        fprintf(stderr, "[FATAL] Character %c not in initial alphabet. Program will now terminate.", seq[0]);
        exit(-1);
    }
    // Write to buffer
    bits_write(buffer, curr->code, dict->bits);
    // Add new entry to dict if not EOF
    if (*(seq + len) != '\0') {
        char *new_seq = malloc(len);
        strncpy(new_seq, seq, len);
        lzw_dict_add(dict, new_seq);
    }
    return seq + len - 1;
}

int main() {
    char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *code = "TOBEORNOTTOBEORTOBEORNOT";
    data *d = data_init();
    lzw_dict *dict = lzw_dict_init(alphabet);
    while (*code != '\0')
        code = lzw_encode(dict, code, d);
    bits_write(d, 0, dict->bits); // Stop code = 0
    lzw_dict_print(dict);
    bits_print(d);
    data_free(d);
    lzw_dict_free(dict);
}