#include <string.h>
#include <stdlib.h>

int main(void) {
	char *a = malloc(255);
	strcpy(a, "abcdefghijk");
	strtok(a, "b");
	return 0;
}
