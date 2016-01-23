#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rl.h"
#include "lzw.h"
#include "naive_rl.h"

int MAX_TESTS = 1000000;
int QS_BUFFER_SIZE = 1000;

typedef struct benchmark_t {
    uint32_t ori_total;
    uint32_t enc_bytes;
    float cmp_ratio;
    uintmax_t time;
} benchmark_t;

typedef int(*benchmark_func)(char *code);

benchmark_t * benchmark(FILE *f, benchmark_func bf)
{
    fseek(f, 0, SEEK_SET);
    int tests = MAX_TESTS;
    char *code = malloc(QS_BUFFER_SIZE);
    uint32_t ori_total = 0;
    uint32_t enc_total = 0;

    time_t start = time(NULL);
    while (tests--> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        fscanf(f, "%*[^\n]\n", NULL);
        fscanf(f, "%*[^\n]\n", NULL);

        // Get quality scores
        fgets(code, QS_BUFFER_SIZE, f);
        strtok(code, "\n");

        int enc_bytes = bf(code);
        ori_total += strlen(code);
        enc_total += enc_bytes;
    }
    time_t finish = time(NULL);   
    free(code);

    // Return results
    benchmark_t *results = malloc(sizeof(benchmark_t));
    results->ori_total = ori_total;
    results->enc_bytes = enc_total;
    results->time = (uintmax_t) finish - (uintmax_t) start;    
    float comp = (float)enc_total - (float)ori_total;
    results->cmp_ratio = (comp / ori_total * 100) + 100;
    return results;
}



int main(void)
{
    FILE *f = fopen("../data/ERR161544_1.fastq", "r");
    
    benchmark_t *results = benchmark(f, rl_benchmark);
    printf("Results:\n  Algorithm       Bytes    Ratio    Time\n  - Original  %9 " PRIu32 "\n", results->ori_total);
    printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "RL", results->enc_bytes, results->cmp_ratio, results->time);
    free(results);
    
    results = benchmark(f, lzw_benchmark);
    printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "LZW", results->enc_bytes, results->cmp_ratio, results->time);
    free(results);
    
    results = benchmark(f, naive_rl_benchmark);
    printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "NAIVE RL", results->enc_bytes, results->cmp_ratio, results->time);
    free(results);
    
    fclose(f);
}