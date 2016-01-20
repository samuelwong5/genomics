#include "rl.h"

int OFFSET;
int CHAR_LENGTH;  // Bits for storing each character, which is ceil(log_2(size of alphabet))
int COUNT_LENGTH = 4; // Predefined number of bits for storing the 'run-length'
int MAX_LENGTH;

char * rl_encode_run(char *seq, data *buffer)
{
    char c = seq[0];
    int count = 0;
    while (*(seq++) == c && count < MAX_LENGTH)
        count++;
    //printf("[%d %d] -> ", count, ((int) c) - OFFSET);
    bits_write(buffer, ((int) c) - OFFSET, CHAR_LENGTH);
    bits_write(buffer, count, COUNT_LENGTH);
    //int test_c = bits_read(buffer, CHAR_LENGTH);
    //int test_count = bits_read(buffer, COUNT_LENGTH); 
    //printf("[%d %d]\n", test_count, test_c);
    //bits_print(buffer);
    return --seq;
}

char * rl_decode(char *alphabet, data *buffer)
{
    rl_init(alphabet);
    char *plaintext = malloc(200);
    char *curr = plaintext;
    int len = 0;
    uint32_t next_char = bits_read(buffer, CHAR_LENGTH);
    int count = 0;
    while (next_char != 0) {
        char c = (char) (next_char + OFFSET);
        int count = (int) bits_read(buffer, COUNT_LENGTH);
        //printf(">%d %d< ", count, next_char);
        len += count;
        for (; count > 0; count--)
            *(curr++) = c;
        next_char = bits_read(buffer, CHAR_LENGTH);
    }
    plaintext = realloc(plaintext, len + 1);
    plaintext[len] = '\0';
    return plaintext;
}

data * rl_encode(char *alphabet, char *code)
{
    rl_init(alphabet);
    //printf("Char length: %d\n", CHAR_LENGTH);
    data *d = data_init();
    while (code[0] != '\0')
        code = rl_encode_run(code, d);
    bits_write(d, 0, CHAR_LENGTH);   // Encode Stopcode
    return d;
}

void rl_init(char *alphabet)
{
    MAX_LENGTH = pow(2, COUNT_LENGTH);
    OFFSET = ((int)alphabet[0]) - 1; // Leave 0 for stopcode
    double C_LENGTH = ceil(log(strlen(alphabet + 1)) / log(2));
    CHAR_LENGTH = (int)C_LENGTH;
}

int main(void)
{
    char *alphabet = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHI";
    char *code = "HEHHHHFFHHHHHFEHFFHHGHHHCHBHFEHFEHDBGEEFDFFHFFBF>BCEEEFECE@?ADDDAFEE>DADEDDD?G:BCCA=?@@@5>@DE?AE>C@B\0";
    data *d = rl_encode(alphabet, code);
    bits_print(d);
    char *dec = rl_decode(alphabet, d);
    data_free(d);
    free(dec);
    printf("Original: %s\n", dec);
    printf("After   : %s\n", dec);
}