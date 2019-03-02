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
#include <stdbool.h>

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = tolower((unsigned char)*p);
      p++;
  }

  return str;
}

void debug_string(unsigned char * str) {
    printf("\n%d[%c]", str[0], str[0]);
    for (int i = 1; i < strlen(str); i++) {
        printf(" - %i[%c]", str[i], str[i]);
    }
    printf("\n");
    return;
}

char lowerIfHigher(unsigned char in) {
  char out;

  if (in > 40 && in < 91) {
    //ASCII capital
    out = in + 32;
    //printf ("%d - %c => %d - %c\n", in, in, out, out);
  } else {
    out = in;
  }

  return out;
}

bool isAnagram(int * empty_hist, int * in_hist, unsigned char * line, int line_len, int in_len) {
  int line_hist[256];

  if (in_len == line_len){
    memcpy(line_hist, in_hist, sizeof(int)*256);

    for (int i = 0; i < line_len; i++) {
      line[i] = tolower(line[i]);
      if (line_hist[line[i]] < 1) {
        return false;
      } else {
        line_hist[line[i]] -= 1;
      }
    }

    if (memcmp(line_hist, empty_hist, sizeof (int) * 256) == 0) {
      printf("SUKSIS!\n");
      return true;
    } else {
      printf("FAIL???");
      return false;
    }
  } else {
    return false; //line len not same
  }
  printf("WE WERENT SUPPOSED TO GET HERE!");
  return false;
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
    long start = getMicrotime();
    setlocale(LC_ALL, "");

    nice(-20);


    int in_len;
    if (argc == 3) {
      in_len = strlen(argv[2]);
    } else if (argc == 4) {
      in_len = strlen(argv[2]) + 1 + strlen(argv[3]);
    } else {
      exit(EXIT_FAILURE);
    }

    unsigned char *in;
    in = malloc(in_len);

    if (argc == 3) {
      memcpy(in, argv[2], in_len);
    } else if (argc == 4) {
      memcpy(in, argv[2], in_len);
      strcat(in, " ");
      strcat(in, argv[3]);
    } else {
      exit(EXIT_FAILURE);
    }

    in = strlwr(in);

    int in_hist[256] = {0};
    int empty_hist[256] = {0};

    for (int i = 0; i < in_len; i++) {
      in_hist[in[i]] += 1;
    }

    unsigned char possible[106000][50]; //array for all correct anagrams

    int in_sum = 0;
    for (int i = 0; i < in_len; i++) {
      in[i] = tolower(in[i]);
      in_sum += (int) in [i];
    }

    int possible_len = 0;

    unsigned char *buffer;
    struct stat stat;

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0 ) {printf("Broken \n");return EXIT_FAILURE;}
    fstat(fd, &stat);
    /* PROT_READ disallows writing to buffer: will segv */

    unsigned char *text;
    buffer = mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if ( buffer != (void*)-1 ) {
      text = malloc(stat.st_size);
      memcpy(text, buffer, stat.st_size);
    }

    unsigned char *line;
    unsigned char *next_line = strtok(text, "\r\n");
    int line_hist[256];
    while (next_line != NULL) {
      int line_len = 0;
      line = next_line;
      next_line = strtok(NULL, "\r\n");
      //check if length is same
      line_len = next_line - line - 2;
      if (isAnagram(empty_hist, in_hist, line, line_len, in_len)) {
        strcpy(possible[possible_len], line);
        possible_len++;
      } else {
        //printf("NOT AN ANAGRAM");
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
