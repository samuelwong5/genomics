#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rl.h"
#include "lzw.h"
#include "naive_rl.h"
#include "huffman.h"

int MAX_TESTS = 10000;
int QS_BUFFER_SIZE = 1000;

typedef struct benchmark_t {
    uint32_t ori_total;
    uint32_t enc_bytes;
    float cmp_ratio;
    uintmax_t time;
} benchmark_t;

typedef int(*benchmark_func)(char *code);

typedef struct benchmark_thread {
    pthread_t *t;
    uint32_t enc_bytes;
    uint32_t tot_bytes;
    char *code;
    int status; 
    benchmark_func bf;
} benchmark_thread;


void * bmark(void *arg)
{
    benchmark_thread *bt = (benchmark_thread *) arg;
    while (0 == 0) {
        while (bt->status == 0) {}
        strtok(bt->code, "\n");
        int enc_bytes = bf(bt->code);
        bt->enc_bytes += enc_bytes;
        bt->tot_bytes += strlen(bt->code);
        bt->status = 0;
    }
}

benchmark_t * benchmark(FILE *f, benchmark_func bf)
{
    fseek(f, 0, SEEK_SET);
    int tests = MAX_TESTS;
    char *code = malloc(QS_BUFFER_SIZE);
    uint32_t ori_total = 0;
    uint32_t enc_total = 0;

    time_t start = time(NULL);
    int arr[100] = { 0 };
    int THREADS = 10;
    benchmark_thread **bt_list = malloc(THREADS);
    for (int i = 0; i < THREADS; i++) {
        benchmark_thread *bt = malloc(sizeof(benchmark_thread));
        bt->t = malloc(sizeof(pthread_t));
        bt->enc_bytes = 0;
        bt->code = malloc(sizeof(QS_BUFFER_SIZE));
        bt->status = 0;
        bt->bf = bf;
        pthread_create(bt->t, NULL, bmark, (void *) bt);
    }

    int index = 0;

    while (tests--> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        if (fscanf(f, "%*[^\n]\n", NULL) == EOF)
            break;
        if (fscanf(f, "%*[^\n]\n", NULL) == EOF)
            break;
        while (bt_list[index]->status == 1) { }
        fgets(bt_list[index]->code, QS_BUFFER_SIZE, f);
        bt_list[index]->status = 1;
        index = ++index % THREADS;
    }

    /*while (tests--> 0 && fscanf(f, "%*[^\n]\n", NULL) != EOF) {
        // Skip first three lines (one is skipped in while expression
        if (fscanf(f, "%*[^\n]\n", NULL) == EOF)
            break;
        if (fscanf(f, "%*[^\n]\n", NULL) == EOF)
            break;

        // Get quality scores
        fgets(code, QS_BUFFER_SIZE, f);
        strtok(code, "\n");
        int enc_bytes = bf(code);
        printf("[%d] %s (len: %d)\n", enc_bytes, code, strlen(code));
        ori_total += strlen(code);
        enc_total += enc_bytes;
    }*/
    time_t finish = time(NULL);   
    free(code);

    // Return results
    benchmark_t *results = malloc(sizeof(benchmark_t));
    for (int i = 0; i < THREADS; i++) {
        results->ori_total = bt_list[i]->tot_bytes;
        results->enc_bytes = bt_list[i]->enc_bytes;
        pthread_join(*(bt_list[i]->t), NULL);
        free(bt_list[i]);
    }
    free(bt_list);
    results->time = (uintmax_t) finish - (uintmax_t) start;    
    float comp = (float)enc_total - (float)ori_total;
    results->cmp_ratio = (comp / ori_total * 100) + 100;
    return results;
}

void test(void) {
    FILE *f = fopen("../data/ERR161544_1.ql", "r");
    char *code = malloc(200);
    int total = 0;
    for (int i = 0; i < 100; i++) {
        fgets(code, 200, f);
        strtok(code, "\n");
        int enc_bytes = lzw_benchmark(code);
        printf("[%d] %s\n", enc_bytes, code);
        total += enc_bytes;
    }
    printf("Total: %d\n", total);
    free(code);
    fclose(f);
}


int main(void)
{
    FILE *f = fopen("../data/ERR161544_1.fastq", "r");
    //test();
    benchmark_t *results;
    //results = benchmark(f, rl_benchmark);
    //printf("Results:\n  Algorithm       Bytes    Ratio    Time\n  - Original  %9 " PRIu32 "\n", results->ori_total);
    //printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "RL", results->enc_bytes, results->cmp_ratio, results->time);
    //free(results);
    

    //huffman_tree_init();
    //results = benchmark(f, huffman_benchmark);
    //printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "HUFFMAN", results->enc_bytes, results->cmp_ratio, results->time);
    //free(results);
    //huffman_tree_free();

    results = benchmark(f, lzw_benchmark);
    printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "LZW", results->enc_bytes, results->cmp_ratio, results->time);
    free(results);
    
    //results = benchmark(f, naive_rl_benchmark);
    //printf("  - %-8s  %9" PRIu32 "   %4.2f%   (%2llus)\n", "NAIVE RL", results->enc_bytes, results->cmp_ratio, results->time);
    //free(results);
    

    fclose(f);
}