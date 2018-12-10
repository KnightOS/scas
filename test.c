#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>

/**
 * Exit codes:
 * 
 * 1: error locating scas
 * 2: error allocating temporary files
 * 3: test failure
 * 
 */

const char *scas = "./scas";
const char *test = NULL;

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--scas")) {
			if (i + 1 == argc) {
				fprintf(stderr, "No argument given to `--scas`!\n");
				return 1;
			}
			scas = argv[++i];
		}
		if (!strcmp(argv[i], "--test")) {
			if (i + 1 == argc) {
				fputs("No argument given to `--test`!\n", stderr);
				return 1;
			}
			test = argv[++i];
		}
	}
	if (test == NULL) {
		fprintf(stderr, "Usage: %s --test test [--scas /path/to/scas]\n", argv[0]);
		return 0;
	}
	// Make sure scas exists
	if (access(scas, F_OK) == -1) {
		fprintf(stderr, "%s does not exist!\n", scas);
		return 1;
	}
	// Get a temp file to use for bin	
	char binpath[256] = "/tmp/scastestXXXXXX";
	int binfd = mkstemp(binpath);
	if (binfd == -1) {
		fputs("Error allocating temporary file for binary output!\n", stderr);
		return 2;
	}
	{
		// Generate buffer containing file names and path to scas
		char buf[1024];
		strcpy(buf, scas);
		DIR *dir;
		if ((dir = opendir(test)) == NULL) {
			fputs("Error reading in test dir!\n", stderr);
			return 3;
		}
		struct dirent *ent;
		while ((ent = readdir (dir)) != NULL) {
			if (ent->d_name[0] != '.' && strcmp(ent->d_name, "test.bin")) {
				sprintf(buf + strlen(buf), " %s/%s", test, ent->d_name);
			}
		}
		sprintf(buf + strlen(buf), " -o %s", binpath);
		// Assemble the file
		if (system(buf) != 0) {
			fprintf(stderr, "Failed command: %s\n", buf);
			fputs("Error assembling!\n", stderr);
			remove(binpath);
			return 3;
		}
	}
	// Read in the binary
	FILE *binfile = fdopen(binfd, "r");
	if (binfile == NULL) {
		fputs("Error opening binfile!\n", stderr);
		return 3;
	}
	// Get binary size
	long size;
	if ((fseek(binfile, 0, SEEK_END) == -1) || ((size = ftell(binfile)) == -1)) {
		fputs("Error obtaining binary size!\n", stderr);
		return 3;
	}
	rewind(binfile);
	// Read it in
	uint8_t bin[size];
	fread(bin, 1, size, binfile);
	fclose(binfile);
	// Read in expected result
	char buf[1024];
	sprintf(buf, "%s/test.bin", test);
	FILE *correct_bin = fopen(buf, "r");
	if (correct_bin == NULL) {
		fputs("Error opening correct binary!\n", stderr);
		remove(binpath);
		return 3;
	}
	// Get size of correct file
	long correct_size;
	if ((fseek(correct_bin, 0, SEEK_END) == -1) || ((correct_size = ftell(correct_bin)) == -1)) {
		fputs("Error obtaining size of correct binary!\n", stderr);
		remove(binpath);
		return 3;
	}
	rewind(correct_bin);
	if (correct_size != size) {
		fputs("Output has incorrect size!\n", stderr);
		remove(binpath);
		return 3;
	}
	uint8_t correct[correct_size];
	fread(correct, 1, correct_size, correct_bin);
	fclose(correct_bin);
	if (memcmp(correct, bin, size) != 0) {
		fputs("Test result is incorrect!\n", stderr);
		fprintf(stderr, "Leaving wrong binary in %s\n", binpath);
		return 3;
	}
	remove(binpath);
	return 0;
}
