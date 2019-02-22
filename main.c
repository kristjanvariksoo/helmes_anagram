#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <iconv.h>
#include <unistd.h>

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

int cmp (const void *a, const void *b) {
   return *(char*)a - *(char*)b;
}

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *myThreadFun(void *vargp)
{
    //sleep(1);
    //printf("Printing GeeksQuiz from Thread \n");
    return NULL;
}

long getMicrotime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

int main(int argc, char** argv) {
  //printf("Dictionary: %s\nWord to anagram: %s\n", argv[1], argv[2]);
  long start = getMicrotime();
  nice(-20);
  char in[30];
  strcpy(in,argv[2]);//{-11,114,110,118,97,108,103,101}; //õrnvalge

  utf8_to_latin9(in, in, strlen(in));


  char in_ord[30] = "";
  strcpy(in_ord, in);
  qsort(in_ord, strlen(in_ord), sizeof(char), cmp);

  char possible[106000][30];

  //Get length of input word
  int in_len = strlen(in); //when line read from file it will be 1 longer, also NB because of unicode this is not real number of charachters in file

  int in_sum = 0;
  for (int i = 0; i < in_len; i++)
  {
    in_sum+=(int)in[i];
    //printf("%d[%c] - ",in[i], in[i]);
  }
  //printf("NOTHING \n");

  //printf("String %s (ordered as %s) is %d long and has naive sum of %d\n",in, in_ord, in_len, in_sum);

    pthread_t thread_id;

        FILE *f;
        char s[30];
        char s_ord[30];
        char s2[30];

        f=fopen(argv[1],"r");
        if (!f)
            return 1;
        int possible_len = 0;
        int s_len = 0;
        while (fgets(s2,30,f)!=NULL) {
          strcpy(s, s2);
          s_len = strlen(s);
          s[s_len-1] = 0;
          if (s[s_len-2] == 13) {s[s_len-2] = 0;}

          s_len = strlen(s);
//          printf("Atleast it got read... %s\n",s);
//          printf("%d != %d\n", s_len, in_len);
          if (s_len == in_len) {
//            printf("lens were same\n");
            int s_sum = 0;
            for (int i = 0; i < s_len; i++)  {
//              printf("%d[%c] - ", s[i], s[i]);
              s_sum+=(int)s[i];
            }
//            printf("NOTHING \n");

//            printf("%s, %d, %s, %d", in, in_sum, s, s_sum);
            if (s_sum == in_sum) { //-10 to remove \n charachter
//              printf("sums were same\n");
              size_t numbers_len = strlen(s);

              strcpy(s_ord, s);
              //printf("BEFORE: %s\n", s);
              qsort(s_ord, strlen(s_ord), sizeof(char), cmp);
              //printf("AFTER: %s\n", s_ord);

              if (strcmp(s_ord,in_ord) == 0) {
                strcpy(possible[possible_len], s);
                possible_len++;
                //printf("%d - %d - %s\n",strlen(s), s_sum, s);
              }

            }
          }
          //printf("%d - %s",strlen(s), s);

        }
        fclose(f);

/*
FILE *f = fopen("lemmad.txt", "rb");
fseek(f, 0, SEEK_END);
long fsize = ftell(f);
fseek(f, 0, SEEK_SET);  // same as rewind(f);

char *string = malloc(fsize + 1);
fread(string, fsize, 1, f);
fclose(f);
char *p, *temp;
  p = strtok_r(string, "\n", &temp);
  do {
      //printf("current line = %s", p);
  } while ((p = strtok_r(NULL, "\n", &temp)) != NULL);

string[fsize] = 0;
*/

    //printf("Before Thread\n");
    pthread_create(&thread_id, NULL, myThreadFun, NULL);
    pthread_join(thread_id, NULL);
    //printf("After Thread\n");
    //printf("There were %d matches\n", possible_len);
    //printf("%d\n", strlen("\n"));
    long end = getMicrotime();
    printf("It took %d microseconds\n", (end-start));


    exit(0);
}
