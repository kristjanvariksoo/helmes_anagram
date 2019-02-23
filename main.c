#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <errno.h>

void debug_string(char * str) {
    printf("\n%d[%c]", str[0], str[0]);
    for (int i = 1; i < strlen(str); i++) {
        printf(" - %d[%c]", str[i], str[i]);
    }
    printf("\n");
    return;
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

    debug_string(in);
    //utf8_to_latin9(in); //linux terminal arguments come in at utf-8, we work in latin9 encoding
    debug_string(in);

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
        strcat(results, ",");
        strcat(results, possible[i]);
    }

    long end = getMicrotime();
    printf("%d%s\n", (end - start), results);
    exit(0);
}
