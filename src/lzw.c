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
        lzw_dict_add(dict, c, 0);
    }
    return dict;
}

// If decoding, set decode int to 1. If encoding, set decode int to -1.
void lzw_dict_add(lzw_dict *dict, char *seq, int decode)
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

    if (dict->count + decode >= dict->next_bit) {
        dict->bits++;
        dict->next_bit *= 2;
    }
}

char * lzw_dict_get(lzw_dict *dict, int code)
{
    lzw_dict_entry *curr = dict->head;
    while (curr != NULL && curr->code > code)  {
        curr = curr->next;
    }
    if (curr->code != code)
        return NULL;
    return curr->seq;
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
        printf(" - {%d : %s}", curr->code, curr->seq);
        curr = curr->next;
    }
    printf(" - TAIL\n");
}

char * lzw_encode_run(lzw_dict *dict, char *seq, data *buffer)
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

    // Add new entry to dict 
    char *new_seq = malloc(len + 1);
    strncpy(new_seq, seq, len);
    new_seq[len] = '\0';
    lzw_dict_add(dict, new_seq, -1);
    bits_write(buffer, curr->code, dict->bits);
    return seq + len - 1;
}

data * lzw_encode(char *alphabet, char *code)
{
    data *d = data_init();
    lzw_dict *dict = lzw_dict_init(alphabet);
    while (*code != '\0')
        code = lzw_encode_run(dict, code, d);
    bits_write(d, 0, dict->bits); // Stop code = 0
    lzw_dict_free(dict);
    return d;
}

char * lzw_decode(char *alphabet, data *d)
{
    lzw_dict *dict = lzw_dict_init(alphabet);
    int pt_len = 100;
    int total_len = 0;
    char *plaintext = malloc(pt_len);
    char *curr = plaintext;
    // Get the sequence corresponding to the code
    int code = (int) bits_read(d, dict->bits);
    char *last_seq = NULL;
    while (code != 0) {
        char *seq = lzw_dict_get(dict, code);
        if (seq == NULL) {  // Edge case when cScSc where c is a character and S is a string
            int seq_len = strlen(last_seq) + 1;
            seq = malloc(seq_len + 1);
            strncpy(seq, last_seq, seq_len - 1);
            seq[seq_len - 1] = last_seq[0];
            seq[seq_len] = '\0';
            lzw_dict_add(dict, seq, 1);
            total_len += seq_len;
            strncpy(curr, seq, seq_len);
            curr += seq_len;
        } else {
            int seq_len = strlen(seq);
            // Increase size of result if needed
            if (pt_len < total_len + seq_len) {
                pt_len *= 2;
                plaintext = realloc(plaintext, pt_len);
                curr = plaintext + total_len;
            }
            // Copy sequence into result
            total_len += seq_len;
            strncpy(curr, seq, seq_len);
            curr += seq_len;
            // Add new entry to dict which is concat of last_seq and first char of current seq
            if (last_seq != NULL) {
                int new_seq_len = strlen(last_seq) + 2;
                char *new_seq = malloc(new_seq_len);

                strncpy(new_seq, last_seq, new_seq_len - 2);
                new_seq[new_seq_len - 2] = seq[0];
                new_seq[new_seq_len - 1] = '\0';
                lzw_dict_add(dict, new_seq, 1);
            }
        }
        last_seq = seq;
        code = (int)bits_read(d, dict->bits);
    }
    plaintext[total_len] = '\0';

    lzw_dict_free(dict);
    return plaintext;
}

int lzw_benchmark(char *code) 
{
    char *alphabet = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI";
    data *d = lzw_encode(alphabet, code);
    int result = data_size(d);
    char *dec = lzw_decode(alphabet, d);
    if (strncmp(dec, code, strlen(code)) != 0) {
        printf("[LZW ERROR]\n");
        printf("Original: %s\n", code);
        printf("Decoded : %s\n", dec);
    }
    free(dec);
    data_free(d);
    return result;

}
