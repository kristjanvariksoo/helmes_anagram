#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

void debug_string(char * str) {
    printf("\n%d[%c]", str[0], str[0]);
    for (int i = 1; i < strlen(str); i++) {
        printf(" - %i[%c]", str[i], str[i]);
    }
    printf("\n");
    return;
}

char lowerIfHigher(char in) {
  char out;
  out = in;
  wprintf("lowerIfHigher %i[%c]\n", in, in);
  wchar_t star = 0x2605;
  wprintf(L"%lc\n", star);
  wprintf(L"%lc\n", star);
  wprintf(L"%lc\n", star);
  if (in < 91) { //Uppercase ascii or sth special
    if (in > 0) {
      out = in+32;
      printf("lowerIfHigher %i[%c] => %i[%c]\n", in, in, out, out);
    } else {
      wchar_t star = 0x2605;
      wprintf(L"%lc\n", star);
      wprintf(L"%lc\n", star);
      wprintf(L"%lc\n", star);
      wprintf(L"%lc\n", star);
      wprintf(L"%lc\n", star);
      wprintf(L"%lc\n", star);

      printf("lowerIfHigher %i[%c] => %i[%c]\n", in, in, in+32, in+32);
    }
  }
  return out;
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

/*
void mysweetstrcpy(char *dest, char *src, unsigned int *len) {
  char *ptr = dest;
  while((*ptr) != '\0') {
    *ptr = src[*len];
    ++(*len);
  }
}
*/
int main(int argc, char ** argv) {
    //printf("Dictionary: %s\nWord to anagram: %s\n", argv[1], argv[2]);
    long start = getMicrotime();
    setlocale(LC_ALL, "");

    nice(-20);

    int in_len = strlen(argv[2]);
    char *in;
    in = malloc(in_len);
    memcpy(in, argv[2], in_len);
/*    lowerIfHigher(in[0]);*/
    char in_ord[30];
    strcpy(in_ord, in );
    qsort(in_ord, strlen(in_ord), sizeof(char), cmp);

    char possible[106000][30]; //array for all correct anagrams

    int in_sum = 0;
    for (int i = 0; i < in_len; i++) {
        in_sum += (int) in [i];
    }

    char s_ord[30];
    int possible_len = 0;

    char *buffer;
    struct stat stat;

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0 ) {printf("Broken \n");return EXIT_FAILURE;}
    fstat(fd, &stat);
    /* PROT_READ disallows writing to buffer: will segv */

    char *text;
    buffer = mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ( buffer != (void*)-1 ) {
      text = malloc(stat.st_size);
      memcpy(text, buffer, stat.st_size);
    }



    char *line;
    char *next_line = strtok(text, "\r\n");
    while (next_line != NULL) {
      int line_len = 0;
      line = next_line;
      next_line = strtok(NULL, "\r\n");




      //check if length is same
      line_len = next_line - line - 2;
      //printf("%d - %d\n", line_len, in_len);
      if (line_len == in_len) {

        //if sum is same
        int s_sum = 0;
        for (int i = 0; i < line_len; i++) {
            s_sum += (int) line[i];
        }
        if (s_sum == in_sum) {
          //printf("%s - %d - %d\n", line, s_sum, in_sum);
          //if sorted are identical
          //strcpy(s_ord, line);

          memcpy(s_ord, line, line_len + 1); //Not sure why but the +1 was needed, otherwise garbage was at end.
          qsort(s_ord, line_len, sizeof(char), cmp);

          if (strcmp(s_ord, in_ord) == 0) {
              strcpy(possible[possible_len], line);

              possible_len++;
          }
        }
      }
    }

    //OUTPUT
    char results[128] = ""; //the empty string is important
    for (int i = 0; i < possible_len; i++) {
        strcat(results, ",");
        strcat(results, possible[i]);
    }

    long end = getMicrotime();
    printf("%d%s \n", (end - start), results);
    exit(0);
}
