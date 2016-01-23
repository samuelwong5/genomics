#include "huffman.h"

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

huffman_tree * huffman_tree_init(void)
{
    huffman_tree *tree;
    return tree;
}

huffman_tree * huffman_tree_new(char c, huffman_tree *zero, huffman_tree *one)
{
    huffman_tree *t = malloc(sizeof(huffman_tree));
    t->c = c;
    t->zero = zero;
    t->one = one;
    return t;
}

void huffman_tree_free(huffman_tree *t)
{
    if (t->zero != NULL)
        huffman_tree_free(t->zero);
    if (t->one != NULL)
        huffman_tree_free(t->one);
    free(t);
}

char * huffman_decode(data *buffer)
{
    int result_size = 200;
    char *decode = malloc(result_size);
    int index = 0;
    huffman_tree *head = huffman_tree_init();
    huffman_tree *curr = head;
    while (!data_end(buffer)) {
        while (curr->c != '\0') {
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
        curr = head;
    }
    decode[index] = '\0';
    return decode;
}

int huffman_benchmark(char *code)
{
    data *d = huffman_encode(code);
    int result = data_size(d);
    data_free(d);
    return result;
}