#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <ctype.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

/* pneumonoultramicroscopicsilicovolcanoconiosis - a lung disease caused by
 * inhaling silica dust. */
#define LONGEST_WORD 45

#define CHUNK_SIZE  (8 * 1024)

typedef struct count {
    char *key;
    size_t value;
} count;

static int cmp_func(const void *a, const void *b)
{
    const count *const p = *(const count * const *) a;
    const count *const q = *(const count * const *) b;
    
    const int cmp = (p->value < q->value) - (p->value > q->value);

    if (cmp == 0) {
        cmp = (p < q) - (p > q); /* Compare pointers. */
    }

    return cmp;
}

/* Credit: chux - https://stackoverflow.com/a/77963761/20017547 */
static void replace_punctuation(size_t len, unsigned char s[static len])
{
    // ".,;:!?\"()[]{}-"
    static const unsigned char table[UCHAR_MAX + 1] = {
        ['.'] = '.' ^ ' ',[','] = ',' ^ ' ',[';'] = ';' ^ ' ',[':'] = ':' ^ ' ',
        ['!'] = '!' ^ ' ',['?'] = '?' ^ ' ',['"'] = '"' ^ ' ',['('] = '(' ^ ' ',
        [')'] = ')' ^ ' ',['['] = '[' ^ ' ',[']'] = ']' ^ ' ',['{'] = '{' ^ ' ',
        ['}'] = '}' ^ ' ',['-'] = '-' ^ ' '
    };

    for (size_t i = 0; i < len; ++i) {
        s[i] ^= table[s[i]];
    }
}

static void process_chunk(count **ht, size_t curr_chunk_size, unsigned char chunk[static curr_chunk_size])
{
    size_t i = 0;

    while (true) {
        /* Malformed input? Perhaps add a check and make the program slower,
         * or give the user what it deserves. */
        unsigned char word[LONGEST_WORD];
        size_t word_len = 0;

        while (isspace(chunk[i])) {
            ++i;
        }

        const size_t start = i;

        /* Profiling showed that much of the time is spent in this loop. */
        for (; i < curr_chunk_size && !isspace(chunk[i]);
            ++i) {
            word[word_len++] = (unsigned char) tolower(chunk[i]);
        }

        if (i == start) {
            return;
        }

        word[word_len] = '\0';

        /* Skip words beginning with a digit. */
        if (isdigit(word[0])) {
            continue;
        }

        /* Strip possessive nouns. */
        if (word_len >= 2 && word[word_len - 1] == 's'
            && word[word_len - 2] == '\'') {
            word[word_len - 2] = '\0';
        } else if (word[word_len - 1] == '\'') {
            word[word_len - 1] = '\0';
        }

        const size_t new_count = shget(*ht, word);

        shput(*ht, word, new_count + 1U);
    }
}

static count *load_ht(FILE *stream)
{
    count *ht = NULL;

    /* Store the string keys in an arena private to this hash table. */
    sh_new_arena(ht);

    unsigned char chunk[CHUNK_SIZE];
    size_t used_offset = 0;

    while (true) {
        const size_t nread =
            fread(chunk + used_offset, 1, CHUNK_SIZE - used_offset, stream);

        if (ferror(stream)) {
            shfree(ht);
            return NULL;
        }

        if (nread + used_offset == 0) {
            break;
        }

        /* Search for last white-space character in chunk and process up to there. */
        /* Can we replace this with a library function? */
        size_t curr_chunk_end;

        for (curr_chunk_end = nread + used_offset - 1; curr_chunk_end != SIZE_MAX;
            --curr_chunk_end) {
            const unsigned char c = chunk[curr_chunk_end];

            if (isspace(c)) {
                break;
            }
        }

        /* How can we iterate the chunk just once? */
        const size_t curr_chunk_size =
            curr_chunk_end != SIZE_MAX ? curr_chunk_end : nread + used_offset;

        replace_punctuation(curr_chunk_size, chunk);
        process_chunk(&ht, curr_chunk_size, chunk);

        /* Move down remaining partial word. */
        if (curr_chunk_end != SIZE_MAX) {
            used_offset = (nread + used_offset - 1) - curr_chunk_end;
            memmove(chunk, chunk + curr_chunk_end + 1, used_offset);
        } else {
            used_offset = 0;
        }
    }

    return ht;
}

int main(void)
{
    count *ht = load_ht(stdin);

    if (!ht) {
        perror("fread()");
        return EXIT_FAILURE;
    }

    size_t ht_len = shlenu(ht);
    int rv = EXIT_FAILURE;
    count **const ordered = malloc(sizeof *ordered * ht_len);

    if (!ordered) {
        goto exit;
    }

    /* This keeps the array's contents contiguous, and replaces ht_len
     * calls to malloc() with one. 
     * See: https://www.c-faq.com/aryptr/dynmuldimary.html 
     */
    ordered[0] = malloc(sizeof **ordered * ht_len);
    
    if (!ordered[0]) {
        goto exit;
    }

    for (size_t i = 0; i < ht_len; ++i) {
        ordered[i] = ordered[0] + i; 
        ordered[i]->key = ht[i].key;
        ordered[i]->value = ht[i].value;
    }

    qsort(ordered, ht_len, sizeof *ordered, cmp_func);

    for (size_t i = 0; i < ht_len; ++i) {
        printf("%-*s\t%zu\n", LONGEST_WORD, ordered[i]->key, ordered[i]->value);
    }

    /* We are exiting. No need to free the memory. */
    rv = EXIT_SUCCESS;

  exit:
    return rv;
}
