/* Copyright (c) 2016, Pelagicore AB */

#include <stdlib.h>
#include <stdio.h>

#define MEM_CHUNK_SZ 1024 * 1024

int main(int argc, char **argv) {
	size_t allocated = 0;
	while (1) {
		malloc(MEM_CHUNK_SZ);
		allocated += MEM_CHUNK_SZ;
		printf("Allocated %zd bytes of ram in total\n", allocated);
	}
}
