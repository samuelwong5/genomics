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
    lzw_dict_entry *entry = (lzw_dict_entry *) malloc(sizeof(lzw_dict_entry));
    entry->seq = seq;
    entry->code = ++(dict->count);
    entry->next = NULL;
    if (dict->head == NULL)
        dict->head = entry;
    else
        dict->tail->next = entry;
    dict->tail = entry;
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
    int len = 1;
    lzw_dict_entry *entry;
    lzw_dict_entry *curr = dict->head;
    // Find longest match in the dictionary
    while (curr != NULL) {
        if (strncmp(curr->seq, seq, len) == 0) {
            len++;
            entry = curr;
        }
        curr = curr->next;
    }
    // Write to buffer
    bits_write(buffer, entry->code, dict->bits);
    // Add new entry to dict
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
    bits_print(d);
    data_free(d);
    lzw_dict_free(dict);
}