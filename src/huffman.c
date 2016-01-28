#include "huffman.h"

huffman_tree * TREE;

void huffman_encode_char(data *buffer, char c)
{
    switch (c) {
    case 'H': bits_write(buffer, 0, 1); break;
    case 'G': bits_write(buffer, 4, 3); break;
    case 'C': bits_write(buffer, 20, 5); break;
    case 'A': bits_write(buffer, 42, 6); break;
    case '<': bits_write(buffer, 86, 7); break;
    case '9': bits_write(buffer, 174, 8); break;
    case ':': bits_write(buffer, 175, 8); break;
    case 'B': bits_write(buffer, 22, 5); break;
    case '>': bits_write(buffer, 92, 7); break;
    case '.': bits_write(buffer, 744, 10); break;
    case ')': bits_write(buffer, 2980, 12); break;
    case 'I': bits_write(buffer, 2981, 12); break;
    case '+': bits_write(buffer, 1491, 11); break;
    case '6': bits_write(buffer, 373, 9); break;
    case ';': bits_write(buffer, 187, 8); break;
    case '#': bits_write(buffer, 47, 6); break;
    case 'F': bits_write(buffer, 6, 3); break;
    case 'E': bits_write(buffer, 14, 4); break;
    case 'D': bits_write(buffer, 30, 5); break;
    case '2': bits_write(buffer, 992, 10); break;
    case ',': bits_write(buffer, 1986, 11); break;
    case '/': bits_write(buffer, 1987, 11); break;
    case '8': bits_write(buffer, 497, 9); break;
    case '5': bits_write(buffer, 498, 9); break;
    case '7': bits_write(buffer, 499, 9); break;
    case '?': bits_write(buffer, 125, 7); break;
    case '=': bits_write(buffer, 252, 8); break;
    case '3': bits_write(buffer, 1012, 10); break;
    case '(': bits_write(buffer, 4052, 12); break;
    case '*': bits_write(buffer, 4053, 12); break;
    case '%': bits_write(buffer, 16216, 14); break;
    case '&': bits_write(buffer, 16217, 14); break;
    case '\'': bits_write(buffer, 8109, 13); break;
    case '-': bits_write(buffer, 4055, 12); break;
    case '4': bits_write(buffer, 1014, 10); break;
    case '0': bits_write(buffer, 2030, 11); break;
    case '1': bits_write(buffer, 2031, 11); break;
    case '@': bits_write(buffer, 127, 7); break;
    }
}

data * huffman_encode(char *code)
{
    data *d = data_init();
    while (code[0] != '\0') {
        huffman_encode_char(d, code[0]);
        code++;
    }
    return d;
}

void huffman_tree_init(void)
{
    huffman_tree *t0 = huffman_tree_new('H', NULL, NULL);
    huffman_tree *t100 = huffman_tree_new('G', NULL, NULL);
    huffman_tree *t10100 = huffman_tree_new('C', NULL, NULL);
    huffman_tree *t101010 = huffman_tree_new('A', NULL, NULL);
    huffman_tree *t1010110 = huffman_tree_new('<', NULL, NULL);
    huffman_tree *t10101110 = huffman_tree_new('9', NULL, NULL);
    huffman_tree *t10101111 = huffman_tree_new(':', NULL, NULL);
    huffman_tree *t1010111 = huffman_tree_new('\0', t10101110, t10101111);
    huffman_tree *t101011 = huffman_tree_new('\0', t1010110, t1010111);
    huffman_tree *t10101 = huffman_tree_new('\0', t101010, t101011);
    huffman_tree *t1010 = huffman_tree_new('\0', t10100, t10101);
    huffman_tree *t10110 = huffman_tree_new('B', NULL, NULL);
    huffman_tree *t1011100 = huffman_tree_new('>', NULL, NULL);
    huffman_tree *t1011101000 = huffman_tree_new('.', NULL, NULL);
    huffman_tree *t101110100100 = huffman_tree_new(')', NULL, NULL);
    huffman_tree *t101110100101 = huffman_tree_new('I', NULL, NULL);
    huffman_tree *t10111010010 = huffman_tree_new('\0', t101110100100, t101110100101);
    huffman_tree *t10111010011 = huffman_tree_new('+', NULL, NULL);
    huffman_tree *t1011101001 = huffman_tree_new('\0', t10111010010, t10111010011);
    huffman_tree *t101110100 = huffman_tree_new('\0', t1011101000, t1011101001);
    huffman_tree *t101110101 = huffman_tree_new('6', NULL, NULL);
    huffman_tree *t10111010 = huffman_tree_new('\0', t101110100, t101110101);
    huffman_tree *t10111011 = huffman_tree_new(';', NULL, NULL);
    huffman_tree *t1011101 = huffman_tree_new('\0', t10111010, t10111011);
    huffman_tree *t101110 = huffman_tree_new('\0', t1011100, t1011101);
    huffman_tree *t101111 = huffman_tree_new('#', NULL, NULL);
    huffman_tree *t10111 = huffman_tree_new('\0', t101110, t101111);
    huffman_tree *t1011 = huffman_tree_new('\0', t10110, t10111);
    huffman_tree *t101 = huffman_tree_new('\0', t1010, t1011);
    huffman_tree *t10 = huffman_tree_new('\0', t100, t101);
    huffman_tree *t110 = huffman_tree_new('F', NULL, NULL);
    huffman_tree *t1110 = huffman_tree_new('E', NULL, NULL);
    huffman_tree *t11110 = huffman_tree_new('D', NULL, NULL);
    huffman_tree *t1111100000 = huffman_tree_new('2', NULL, NULL);
    huffman_tree *t11111000010 = huffman_tree_new(',', NULL, NULL);
    huffman_tree *t11111000011 = huffman_tree_new('/', NULL, NULL);
    huffman_tree *t1111100001 = huffman_tree_new('\0', t11111000010, t11111000011);
    huffman_tree *t111110000 = huffman_tree_new('\0', t1111100000, t1111100001);
    huffman_tree *t111110001 = huffman_tree_new('8', NULL, NULL);
    huffman_tree *t11111000 = huffman_tree_new('\0', t111110000, t111110001);
    huffman_tree *t111110010 = huffman_tree_new('5', NULL, NULL);
    huffman_tree *t111110011 = huffman_tree_new('7', NULL, NULL);
    huffman_tree *t11111001 = huffman_tree_new('\0', t111110010, t111110011);
    huffman_tree *t1111100 = huffman_tree_new('\0', t11111000, t11111001);
    huffman_tree *t1111101 = huffman_tree_new('?', NULL, NULL);
    huffman_tree *t111110 = huffman_tree_new('\0', t1111100, t1111101);
    huffman_tree *t11111100 = huffman_tree_new('=', NULL, NULL);
    huffman_tree *t1111110100 = huffman_tree_new('3', NULL, NULL);
    huffman_tree *t111111010100 = huffman_tree_new('(', NULL, NULL);
    huffman_tree *t111111010101 = huffman_tree_new('*', NULL, NULL);
    huffman_tree *t11111101010 = huffman_tree_new('\0', t111111010100, t111111010101);
    huffman_tree *t11111101011000 = huffman_tree_new('%', NULL, NULL);
    huffman_tree *t11111101011001 = huffman_tree_new('&', NULL, NULL);
    huffman_tree *t1111110101100 = huffman_tree_new('\0', t11111101011000, t11111101011001);
    huffman_tree *t1111110101101 = huffman_tree_new('\'', NULL, NULL);
    huffman_tree *t111111010110 = huffman_tree_new('\0', t1111110101100, t1111110101101);
    huffman_tree *t111111010111 = huffman_tree_new('-', NULL, NULL);
    huffman_tree *t11111101011 = huffman_tree_new('\0', t111111010110, t111111010111);
    huffman_tree *t1111110101 = huffman_tree_new('\0', t11111101010, t11111101011);
    huffman_tree *t111111010 = huffman_tree_new('\0', t1111110100, t1111110101);
    huffman_tree *t1111110110 = huffman_tree_new('4', NULL, NULL);
    huffman_tree *t11111101110 = huffman_tree_new('0', NULL, NULL);
    huffman_tree *t11111101111 = huffman_tree_new('1', NULL, NULL);
    huffman_tree *t1111110111 = huffman_tree_new('\0', t11111101110, t11111101111);
    huffman_tree *t111111011 = huffman_tree_new('\0', t1111110110, t1111110111);
    huffman_tree *t11111101 = huffman_tree_new('\0', t111111010, t111111011);
    huffman_tree *t1111110 = huffman_tree_new('\0', t11111100, t11111101);
    huffman_tree *t1111111 = huffman_tree_new('@', NULL, NULL);
    huffman_tree *t111111 = huffman_tree_new('\0', t1111110, t1111111);
    huffman_tree *t11111 = huffman_tree_new('\0', t111110, t111111);
    huffman_tree *t1111 = huffman_tree_new('\0', t11110, t11111);
    huffman_tree *t111 = huffman_tree_new('\0', t1110, t1111);
    huffman_tree *t11 = huffman_tree_new('\0', t110, t111);
    huffman_tree *t1 = huffman_tree_new('\0', t10, t11);
    TREE = huffman_tree_new('\0', t0, t1);
}

huffman_tree * huffman_tree_new(char c, huffman_tree *zero, huffman_tree *one)
{
    huffman_tree *t = malloc(sizeof(huffman_tree));
    t->c = c;
    t->zero = zero;
    t->one = one;
    return t;
}

void huffman_tree_free_r(huffman_tree *t)
{
    if (t->zero != NULL)
        huffman_tree_free_r(t->zero);
    if (t->one != NULL)
        huffman_tree_free_r(t->one);
    free(t);
}

void huffman_tree_free(void)
{
    huffman_tree_free_r(TREE);
}

char * huffman_decode(data *buffer)
{
    int result_size = 200;
    char *decode = malloc(result_size);
    int index = 0;
    huffman_tree *curr = TREE;
    while (!data_end(buffer)) {
        while (curr->c == '\0') {
            uint32_t i = bits_read(buffer, 1);
            if (i == 0)
                curr = curr->zero;
            else
                curr = curr->one;
        }
        decode[index++] = curr->c;
        if (index >= result_size) {
            result_size *= 2;
            decode = realloc(decode, result_size);
        }
        curr = TREE;
    }
    decode[index] = '\0';
    return decode;
}

int huffman_benchmark(char *code)
{
    data *d = huffman_encode(code);
    int result = data_size(d);
    /*char *dec = huffman_decode(d);
    if (strncmp(dec, code, strlen(code)) != 0) {
        printf("[HUFFMAN ERROR]\n");
        printf("Original: %s\n", code);
        printf("Decoded : %s\n", dec);
    }
    free(dec);*/
    data_free(d);
    return result;
}