#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>

char *strlwr(char *str) {
  unsigned char *p = (unsigned char *)str;

  while (*p) {
    *p = tolower((unsigned char)*p);
    p++;
  }

  return str;
}

void debug_string(unsigned char *str) {
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
    // ASCII capital
    out = in + 32;
    // printf ("%d - %c => %d - %c\n", in, in, out, out);
  } else {
    out = in;
  }

  return out;
}

int cmp(const void *a, const void *b) { return *(char *)a - *(char *)b; }

long getMicrotime() {
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

int main(int argc, char **argv) {
  // fix Zurnaal and Sokolaad
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

  /*    lowerIfHigher(in[0]);*/
  unsigned char in_ord[50];
  strcpy(in_ord, in);
  qsort(in_ord, strlen(in_ord), sizeof(char), cmp);

  unsigned char possible[106000][50]; // array for all correct anagrams

  int in_sum = 0;
  int in_max = 0;
  int in_min = 300;
  for (int i = 0; i < in_len; i++) {
    in[i] = tolower(in[i]);
    in_sum += (int)in[i];
    if (in[i] > in_max) {
      in_max = in[i];
    }
    if (in[i] < in_min) {
      in_min = in[i];
    }
    //  printf ("%d - %c\n", in[i], in[i]);
  }

  // printf("|%s| - |%s| - sum %d - max %d - min %d\n", in, in_ord, in_sum,
  // in_max, in_min);

  unsigned char s_ord[50];
  int possible_len = 0;

  unsigned char *buffer;
  struct stat stat;

  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    printf("No dictionary file present. \n");
    return EXIT_FAILURE;
  }
  fstat(fd, &stat);
  /* PROT_READ disallows writing to buffer: will segv */

  unsigned char *text;
  buffer = mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (buffer != (void *)-1) {
    text = malloc(stat.st_size);
    memcpy(text, buffer, stat.st_size);
  }

  unsigned char *line;
  unsigned char *next_line = strtok(text, "\r\n");
  while (next_line != NULL) {
    int line_len;
  get_next:

    line_len = 0;
    line = next_line;
    next_line = strtok(NULL, "\r\n");
    // file is alphabetic
    if (line[0] <= in_max) {
      // check if length is same
      line_len = next_line - line - 2;

      // printf("%s - %d - %d\n", line, line_len, in_len);
      if (line_len == in_len) {
        // if sum is same
        // printf("SAME: %s - %d - %d\n", line, line_len, in_len);
        line[0] = lowerIfHigher(line[0]);
        int s_sum = 0;
        int s_max = 0;
        int s_min = 255;
        for (int i = 0; i < line_len; i++) {
          line[i] = tolower(line[i]);
          s_sum += (int)line[i];
          if (line[i] < in_min) {
            goto get_next;
          }
          if (line[i] > in_max) {
            goto get_next;
          }
          // printf ("%d - %c\n", line[i], line[i]);
        }
        // printf("%s - %d - %d\n", line, s_sum, in_sum);
        if (s_sum == in_sum) {
          // printf("%s - %d - %d\n", line, s_sum, in_sum);
          // if sorted are identical
          // strcpy(s_ord, line);

          memcpy(s_ord, line,
                 line_len + 1); // Not sure why but the +1 was needed, otherwise
                                // garbage was at end.
          qsort(s_ord, line_len, sizeof(char), cmp);

          if (strcmp(s_ord, in_ord) == 0) {
            strcpy(possible[possible_len], line);
            possible_len++;
          }
        }
      }
    } else {
      // printf("%d[%c] is too big\n", line[0], line[0]);
      char results[128] = ""; // the empty string is important
      for (int i = 0; i < possible_len; i++) {
        strcat(results, ",");
        strcat(results, possible[i]);
      }

      long end = getMicrotime();
      printf("%d%s \n", (end - start), results);
      exit(0);
    }
  }

  // OUTPUT
  char results[128] = ""; // the empty string is important
  for (int i = 0; i < possible_len; i++) {
    strcat(results, ",");
    strcat(results, possible[i]);
  }

  long end = getMicrotime();
  printf("%d%s \n", (end - start), results);
  exit(0);
}
