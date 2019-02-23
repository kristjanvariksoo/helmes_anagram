#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>

void debug_string(char * str) {
    printf("\n%d[%c]", str[0], str[0]);
    for (int i = 1; i < strlen(str); i++) {
        printf(" - %d[%c]", str[i], str[i]);
    }
    printf("\n");
    return;
}

//https://codereview.stackexchange.com/a/40792
size_t iso8859_1_to_utf8(char * content, size_t max_size) {
    char * src, * dst;

    //first run to see if there's enough space for the new bytes
    for (src = dst = content;* src; src++, dst++) {
        if ( * src & 0x80) {
            // If the high bit is set in the ISO-8859-1 representation, then
            // the UTF-8 representation requires two bytes (one more than usual).
            ++dst;
        }
    }

    if (dst - content + 1 > max_size) {
        // Inform caller of the space required
        return dst - content + 1;
    }

    *(dst + 1) = '\0';
    while (dst > src) {
        if ( * src & 0x80) {
            * dst-- = 0x80 | ( * src & 0x3f); // trailing byte
            * dst-- = 0xc0 | ( * ((unsigned char * ) src--) >> 6); // leading byte
        } else {
            * dst-- = * src--;
        }
    }
    return 0; // SUCCESS
}

//https://codereview.stackexchange.com/a/40857
char * iso88959_to_utf8(const char * str) {
    char * utf8 = malloc(1 + (2 * strlen(str)));

    if (utf8) {
        char * c = utf8;
        for (;* str; ++str) {
            if ( * str & 0x80) {
                * c++ = * str;
            } else {
                * c++ = (char)(0xc0 | (unsigned) * str >> 6);
                * c++ = (char)(0x80 | ( * str & 0x3f));
            }
        }
        * c++ = '\0';
    }
    return utf8;
}

//https://stackoverflow.com/a/11173493
/* UTF-8 to ISO-8859-1/ISO-8859-15 mapper.
 * Return 0..255 for valid ISO-8859-15 code points, 256 otherwise.
*/
static inline unsigned int to_latin9(const unsigned int code)
{
    /* Code points 0 to U+00FF are the same in both. */
    if (code < 256U)
        return code;
    switch (code) {
    case 0x0152U: return 188U; /* U+0152 = 0xBC: OE ligature */
    case 0x0153U: return 189U; /* U+0153 = 0xBD: oe ligature */
    case 0x0160U: return 166U; /* U+0160 = 0xA6: S with caron */
    case 0x0161U: return 168U; /* U+0161 = 0xA8: s with caron */
    case 0x0178U: return 190U; /* U+0178 = 0xBE: Y with diaresis */
    case 0x017DU: return 180U; /* U+017D = 0xB4: Z with caron */
    case 0x017EU: return 184U; /* U+017E = 0xB8: z with caron */
    case 0x20ACU: return 164U; /* U+20AC = 0xA4: Euro */
    default:      return 256U;
    }
}

/* Convert an UTF-8 string to ISO-8859-15.
 * All invalid sequences are ignored.
 * Note: output == input is allowed,
 * but   input < output < input + length
 * is not.
 * Output has to have room for (length+1) chars, including the trailing NUL byte.
*/
size_t utf8_to_latin9(char *const output, const char *const input, const size_t length)
{
    unsigned char             *out = (unsigned char *)output;
    const unsigned char       *in  = (const unsigned char *)input;
    const unsigned char *const end = (const unsigned char *)input + length;
    unsigned int               c;

    while (in < end)
        if (*in < 128)
            *(out++) = *(in++); /* Valid codepoint */
        else
        if (*in < 192)
            in++;               /* 10000000 .. 10111111 are invalid */
        else
        if (*in < 224) {        /* 110xxxxx 10xxxxxx */
            if (in + 1 >= end)
                break;
            if ((in[1] & 192U) == 128U) {
                c = to_latin9( (((unsigned int)(in[0] & 0x1FU)) << 6U)
                             |  ((unsigned int)(in[1] & 0x3FU)) );
                if (c < 256)
                    *(out++) = c;
            }
            in += 2;

        } else
        if (*in < 240) {        /* 1110xxxx 10xxxxxx 10xxxxxx */
            if (in + 2 >= end)
                break;
            if ((in[1] & 192U) == 128U &&
                (in[2] & 192U) == 128U) {
                c = to_latin9( (((unsigned int)(in[0] & 0x0FU)) << 12U)
                             | (((unsigned int)(in[1] & 0x3FU)) << 6U)
                             |  ((unsigned int)(in[2] & 0x3FU)) );
                if (c < 256)
                    *(out++) = c;
            }
            in += 3;

        } else
        if (*in < 248) {        /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
            if (in + 3 >= end)
                break;
            if ((in[1] & 192U) == 128U &&
                (in[2] & 192U) == 128U &&
                (in[3] & 192U) == 128U) {
                c = to_latin9( (((unsigned int)(in[0] & 0x07U)) << 18U)
                             | (((unsigned int)(in[1] & 0x3FU)) << 12U)
                             | (((unsigned int)(in[2] & 0x3FU)) << 6U)
                             |  ((unsigned int)(in[3] & 0x3FU)) );
                if (c < 256)
                    *(out++) = c;
            }
            in += 4;

        } else
        if (*in < 252) {        /* 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
            if (in + 4 >= end)
                break;
            if ((in[1] & 192U) == 128U &&
                (in[2] & 192U) == 128U &&
                (in[3] & 192U) == 128U &&
                (in[4] & 192U) == 128U) {
                c = to_latin9( (((unsigned int)(in[0] & 0x03U)) << 24U)
                             | (((unsigned int)(in[1] & 0x3FU)) << 18U)
                             | (((unsigned int)(in[2] & 0x3FU)) << 12U)
                             | (((unsigned int)(in[3] & 0x3FU)) << 6U)
                             |  ((unsigned int)(in[4] & 0x3FU)) );
                if (c < 256)
                    *(out++) = c;
            }
            in += 5;

        } else
        if (*in < 254) {        /* 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx */
            if (in + 5 >= end)
                break;
            if ((in[1] & 192U) == 128U &&
                (in[2] & 192U) == 128U &&
                (in[3] & 192U) == 128U &&
                (in[4] & 192U) == 128U &&
                (in[5] & 192U) == 128U) {
                c = to_latin9( (((unsigned int)(in[0] & 0x01U)) << 30U)
                             | (((unsigned int)(in[1] & 0x3FU)) << 24U)
                             | (((unsigned int)(in[2] & 0x3FU)) << 18U)
                             | (((unsigned int)(in[3] & 0x3FU)) << 12U)
                             | (((unsigned int)(in[4] & 0x3FU)) << 6U)
                             |  ((unsigned int)(in[5] & 0x3FU)) );
                if (c < 256)
                    *(out++) = c;
            }
            in += 6;

        } else
            in++;               /* 11111110 and 11111111 are invalid */

    /* Terminate the output string. */
    *out = '\0';

    return (size_t)(out - (unsigned char *)output);
}

int cmp(const void * a,
    const void * b) {
    return *(char * ) a - * (char * ) b;
}

long getMicrotime() {
    struct timeval currentTime;
    gettimeofday( & currentTime, NULL);
    return currentTime.tv_sec * (int) 1e6 + currentTime.tv_usec;
}

int main(int argc, char ** argv) {
    //printf("Dictionary: %s\nWord to anagram: %s\n", argv[1], argv[2]);
    long start = getMicrotime();
    nice(-20);
    char in[30];
    strcpy(in , argv[2]); //{-11,114,110,118,97,108,103,101}; //õrnvalge

    utf8_to_latin9(in, in, strlen(in)); //linux terminal arguments come in at utf-8, we work in latin9 encoding

    char in_ord[30] = "";
    strcpy(in_ord, in );
    qsort(in_ord, strlen(in_ord), sizeof(char), cmp);

    char possible[106000][30]; //array for all correct anagrams

    //Get length of input word
    int in_len = strlen(in);
    int in_sum = 0;
    for (int i = 0; i < in_len; i++) {
        in_sum += (int) in [i];
    }

    FILE * f;
    char s[30];
    char s_ord[30];
    char s2[30];

    f = fopen(argv[1], "r");
    if (!f)
        return 1;
    int possible_len = 0;
    int s_len = 0;

    while (fgets(s2, 20, f) != NULL) {
        strcpy(s, s2);
        s_len = strlen(s);
        if (s[s_len - 1] == 10) {s[s_len - 1] = 0;}
        if (s[s_len - 2] == 13) {s[s_len - 2] = 0;}

        s_len = strlen(s);
        if (s_len == in_len) {
            int s_sum = 0;
            for (int i = 0; i < s_len; i++) {
                s_sum += (int) s[i];
            }
            if (s_sum == in_sum) {
                size_t numbers_len = strlen(s);
                strcpy(s_ord, s);
                qsort(s_ord, strlen(s_ord), sizeof(char), cmp);

                if (strcmp(s_ord, in_ord) == 0) {
                    strcpy(possible[possible_len], s);
                    possible_len++;
                    printf(s);
                }

            }
        }

    }



    fclose(f);


    //OUTPUT
    setlocale(LC_ALL, "");
    char results[128];   // Use an array which is large enough
    printf("matches %d \n", possible_len);
    for (int i = 0; i < possible_len; i++) {
        char a[30];
        strcpy(a, possible[i]);
        iso8859_1_to_utf8(a, 100);
        strcat(results, ",");
        strcat(results, a);
    }

    long end = getMicrotime();
    printf("%d%s\n", (end - start), results);
    exit(0);
}
