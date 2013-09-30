/*
 * randomdelay.c
 * by Keith Gaughan <http://talideon.com/>
 *
 * Runs a command after a randomised delay (in minutes).
 *
 * Copyright (c) Keith Gaughan, 2013.
 *
 * "THE BEERWARE LICENSE" (Revision 42):
 * Keith Gaughan wrote this program. As long as you retain this notice,
 * you can do whatever you like with it. If we meet some day, and you
 * think this program is worth it, you can buy me a beer in return.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** argv) {
	int maximum;
	char* prog_name;

	prog_name = (char*) (intptr_t) basename(strdup(argv[0]));
	if (argc < 3) {
		fprintf(stderr,
				"Usage: %s <max> <cmd..>\n"
				"Execute a command after a random delay (in minutes).\n",
				prog_name);
		return EXIT_FAILURE;
	}

	srand(time(NULL));
	maximum = atoi(argv[1]);
	sleep(60 * (maximum > 0 ? rand() % maximum : 0));

	execvp(argv[2], &(argv[2]));

	fprintf(stderr, "%s: %s\n", prog_name, strerror(errno));
	return EXIT_FAILURE;
}
