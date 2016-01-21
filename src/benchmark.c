#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rl.h"
#include "lzw.h"

int main(void)
{
    FILE *f = fopen("../data/ERR161544_1.fastq", "r");
    int TESTS = 1000000;
    char *code = malloc(200);
    int ori_total = 0;
    int rl_total = 0;
    int lzw_total = 0;
    uintmax_t rl_time = 0;
    uintmax_t lzw_time = 0;
    while (TESTS --> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        fscanf(f, "%*[^\n]\n", NULL);
        fscanf(f, "%*[^\n]\n", NULL);

        // Get quality scores
        char *code = malloc(200);
        fgets(code, 200, f);
        strtok(code, "\n");

        time_t rl_start = time(NULL);
        int rl_bytes = rl_benchmark(code);
        time_t rl_finish = time(NULL);
        rl_time += ((uintmax_t) rl_finish) - ((uintmax_t) rl_start);

        time_t lzw_start = time(NULL);
        int lzw_bytes = lzw_benchmark(code);
        time_t lzw_finish = time(NULL);
        lzw_time += ((uintmax_t) lzw_finish) - ((uintmax_t) lzw_start);

        ori_total += strlen(code);
        rl_total += rl_bytes;
        lzw_total += lzw_bytes;
        //printf("Original: %4d    RL: %4d    LZW: %4d\n", strlen(code), rl_bytes, lzw_bytes);
    }
    float rl_delta = rl_total - ori_total;
    rl_delta = rl_delta / ori_total * 100;
    float lzw_delta = lzw_total - ori_total;
    lzw_delta = lzw_delta / ori_total * 100;

    printf("Results:\n  - Original: %5d\n  - RL      : %5d [%4.2f\%]\n  - LZW     : %5d [%4.2f\%]\n", ori_total, rl_total, rl_delta, lzw_total, lzw_delta);
    free(code);
    fclose(f);
}