#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rl.h"
#include "lzw.h"
#include "naive_rl.h"

int MAX_TESTS = 1000000;

int main(void)
{
    FILE *f = fopen("../data/ERR161544_1.fastq", "r");
    int TESTS = MAX_TESTS;
    char *code = malloc(200);
    int ori_total = 0;
    int rl_total = 0;
    int lzw_total = 0;
    uintmax_t rl_time = 0;
    uintmax_t lzw_time = 0;
    time_t rl_start = time(NULL);
    while (TESTS --> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        fscanf(f, "%*[^\n]\n", NULL);
        fscanf(f, "%*[^\n]\n", NULL);

        // Get quality scores
        char *code = malloc(200);
        fgets(code, 200, f);
        strtok(code, "\n");

        int rl_bytes = rl_benchmark(code);
        ori_total += strlen(code);
        rl_total += rl_bytes;
    }
    time_t rl_finish = time(NULL);
    rl_time = ((uintmax_t)rl_finish) - ((uintmax_t)rl_start);
    fseek(f, 0, SEEK_SET);

    TESTS = MAX_TESTS;
    time_t lzw_start = time(NULL);
    while (TESTS--> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        fscanf(f, "%*[^\n]\n", NULL);
        fscanf(f, "%*[^\n]\n", NULL);

        // Get quality scores
        char *code = malloc(200);
        fgets(code, 200, f);
        strtok(code, "\n");

        int lzw_bytes = lzw_benchmark(code);
        lzw_total += lzw_bytes;
    }
    time_t lzw_finish = time(NULL);
    lzw_time = ((uintmax_t) lzw_finish) - ((uintmax_t) lzw_start);
    fseek(f, 0, SEEK_SET);

    TESTS = MAX_TESTS;
    time_t naive_rl_start = time(NULL);
    int naive_rl_total = 0;
    while (TESTS--> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        fscanf(f, "%*[^\n]\n", NULL);
        fscanf(f, "%*[^\n]\n", NULL);

        // Get quality scores
        char *code = malloc(200);
        fgets(code, 200, f);
        strtok(code, "\n");

        naive_rl_total += naive_rl_benchmark(code);
    }
    time_t naive_rl_finish = time(NULL);
    time_t naive_rl_time = ((uintmax_t) naive_rl_finish) - ((uintmax_t) naive_rl_start);

    float rl_delta = rl_total - ori_total;
    rl_delta = rl_delta / ori_total * 100;
    float lzw_delta = lzw_total - ori_total;
    lzw_delta = lzw_delta / ori_total * 100;
    float n_rl_delta = naive_rl_total - ori_total;
    n_rl_delta = n_rl_delta / ori_total * 100;


    printf("Results:\n  - Original: %5d\n  - RL      : %5d [%4.2f\%] (%llu s)\n  - LZW     : %5d [%4.2f\%] (%llu s)\n  - NAIVE_RL: %5d [%4.2f\%] (%llu s)\n", ori_total, rl_total, rl_delta, rl_time, lzw_total, lzw_delta, lzw_time, naive_rl_total, n_rl_delta, naive_rl_time);
    free(code);
    fclose(f);
}