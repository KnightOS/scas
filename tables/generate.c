#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char **argv) {
    if (argc != 3) {
        printf("Usage: %s SOURCE DESTINATION\n", argv[0]);
        return 1;
    }
    FILE *source = fopen(argv[1], "rb");
    if (!source) {
        perror("Error opening source file");
        return 1;
    }
    FILE *destination = fopen(argv[2], "wb");
    if (!destination) {
        perror("Error opening destination file");
        fclose(source);
        return 1;
    }
    int return_code = 1;

    if (fseek(source, 0, SEEK_END) != 0) {
        puts("Seek failed!");
        goto cleanup;
    }
    long length = ftell(source);
    if (length == -1) {
        puts("Failed to determine source file size!");
        goto cleanup;
    }
    rewind(source);
    char *buf = malloc(length);
    if (!buf) {
        puts("out of memory!");
        goto cleanup;
    }
    if (fread(buf, 1, length, source) != (size_t)length) {
        puts("Failed to read source file into buffer!");
        goto cleanup;
    }

    if (fprintf(destination, "const char z80_tab[%lu] = {\n\t", length) < 0) {
        puts("Failed to print to file!");
        goto cleanup;
    }
    for (long i = 0; i < length; i++) {
        if (fprintf(destination, "0x%02x,%s", buf[i], (i + 1) % 8 == 0 ? "\n\t" : " ") < 0) {
            puts("Failed to print to file!");
            goto cleanup;
        }
    }

    if (fwrite("\n};\n", 1, 4, destination) != 4) {
        puts("Failed to write to file!");
        goto cleanup;
    }
    return_code = 0;
cleanup:
    if (fflush(destination) != 0) {
        puts("Failed to flush file!");
        return_code = 1;
    }
    if (fclose(destination) != 0) {
        puts("Failed to close output file!");
        return_code = 1;
    }
    if (fclose(source) != 0) {
        puts("Failed to close input file!");
        return_code = 1;
    }

    return return_code;
}
