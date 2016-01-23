#include "naive_rl.h"

int BUFFER_INIT_SIZE = 200;

char * naive_rl_encode_run(char *seq, char *buffer)
{
    char c = seq[0];
    int count = 0;
    while (*(seq++) == c && count < 127)
        count++;
    buffer[0] = c;
    buffer[1] = (char) count;
    //printf("Setting %d %c\n", count, c);
    return --seq;
}

char * naive_rl_encode(char *code)
{
    int BUFFER_CURR_SIZE = BUFFER_INIT_SIZE;
    char *result = malloc(BUFFER_CURR_SIZE);
    char *curr = result;
    int len = 0;
    while (*code != '\0') {
        len += 2;
        if (len >= BUFFER_INIT_SIZE) {
            BUFFER_CURR_SIZE *= 2;
            result = realloc(result, BUFFER_CURR_SIZE);
            curr = result + len - 2;
        }
        code = naive_rl_encode_run(code, curr);
        curr += 2;
    }
    *(++curr) = '\0';
    return result;
}

char * naive_rl_decode(char *data)
{
    int BUFFER_CURR_SIZE = BUFFER_INIT_SIZE;
    char *decoded = malloc(BUFFER_CURR_SIZE);
    char *curr = decoded;
    int total_len = 0;
    while (*data != '\0' && *(data + 1) != '\0')
    {
        char c = data[0];
        int len = (int) data[1];
        //printf("Got %d %c\n", len, c);
        total_len += len;
        if (total_len >= BUFFER_CURR_SIZE) {
            BUFFER_INIT_SIZE *= 2;
            decoded = realloc(decoded, BUFFER_INIT_SIZE);
            curr = decoded + total_len - len;
        }
        for (; len > 0; len--) {
            *(curr++) = c;
        }
        data += 2;
    }
    return decoded;
}

int naive_rl_benchmark(char *code)
{
    char *encoded = naive_rl_encode(code);
    int len = strlen(encoded);
    char *dec = naive_rl_decode(encoded);
    if (strncmp(code, dec, strlen(code)) != 0) {
        printf("[NAIVE RL ERROR]\n");
        printf("Original: %s\n", code);
        printf("Decoded : %s\n", dec);
    }
    free(dec);
    free(encoded);
    return len; 
}