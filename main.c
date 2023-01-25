#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#include "eglut.h"

const char log_file[] = "/tmp/lol.log";

void es3gears_main(void);

int main(int argc, char **argv) {
	FILE *fp = NULL;

	if ((fp = fopen(log_file, "a")) == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	for (int i = 0; i < argc; i++) {
		fprintf(fp, "hi from %s\n", DEFAULT_APP_ID);
		fprintf(fp, "argv[%d]: '%s'\n", i, argv[i]);
	}

	if (fclose(fp) == EOF) {
		perror("fclose");
		return EXIT_FAILURE;
	}

	fp = NULL;

	es3gears_main();

	return EXIT_SUCCESS;
}
