#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "bits.h"

/* Huffman Encoding with precalculated encoding table. 

Frequencies of first 100 000 000 FASTQ quality score characters:

!: 0
": 0
#: 1567228
$: 0
%: 8877
&: 9156
': 19829
(: 26096
): 23180
*: 34243
+: 47254
,: 52308
-: 39501
.: 90169
/: 55292
0: 79515
1: 81286
2: 102120
3: 122755
4: 160220
5: 213786
6: 202781
7: 232397
8: 211097
9: 337662
:: 383548
;: 409741
<: 608926
=: 537489
>: 769120
?: 1108534
@: 1119625
A: 1283933
B: 2617060
C: 2476649
D: 4132378
E: 7464186
F: 12214754
G: 8363603
H: 52770095
I: 23607

FASTQ file: ERR161544_1.fastq
  - downloaded from http://www.ebi.ac.uk/ena/data/view/ERP001652

Computed binary encoding:

H: 0
G: 100
C: 10100
A: 101010
<: 1010110
9: 10101110
:: 10101111
B: 10110
>: 1011100
.: 1011101000
): 101110100100
I: 101110100101
+: 10111010011
6: 101110101
;: 10111011
#: 101111
F: 110
E: 1110
D: 11110
2: 1111100000
,: 11111000010
/: 11111000011
8: 111110001
5: 111110010
7: 111110011
?: 1111101
=: 11111100
3: 1111110100
(: 111111010100
*: 111111010101
%: 11111101011000
&: 11111101011001
': 1111110101101
-: 111111010111
4: 1111110110
0: 11111101110
1: 11111101111
@: 1111111

 */

typedef struct huffman_tree {
    char c;                    // Character associated with the leaf node (if non leaf then c = '\0'
    struct huffman_tree *zero; // Pointer to next node where current bit is zero
    struct huffman_tree *one;  // Pointer to next node where current bit is one
} huffman_tree;

void huffman_encode_char(data *, char);
data * huffman_encode(char *);
void huffman_tree_init(void);
void huffman_tree_free(void);
void huffman_tree_free_r(huffman_tree *);
huffman_tree * huffman_tree_new(char, huffman_tree *, huffman_tree *);
char * huffman_decode(data *);
int huffman_benchmark(char *);

#endif