#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "bmp_handler.h"
#include "qdbmp.h"

#define NORMAL_ARGUMENTS_COUNT 3
#define error(...) (fprintf(stderr, __VA_ARGS__))
#define BYTES_COUNT_IN_PIXEL 3
#define PALETTE_SIZE_8bbp (256 * 4)
#define MAX_FILENAME_SIZE 255

int is_filename_incorrect(char* filename, char* key) {
    unsigned int filename_length = strlen(filename);
    unsigned int key_length = strlen(key);
    for (int i = 0; i < key_length; i++) {
        if (tolower(filename[filename_length - key_length + i]) != key[i]) {
            return 1;
        }
    }
    return 0;
}

typedef enum {
    MINE,
    THEIRS
} REALIZATION_TYPE;

int scan_arguments(int count_of_arguments, char** arguments, REALIZATION_TYPE* realization,
                   char* input_filename, char* output_filename) {
    if (count_of_arguments - 1 != NORMAL_ARGUMENTS_COUNT) {
        error("%s", "Count of arguments must be 3");
        return 1;
    }
    if (strcmp(arguments[1], "--mine") == 0) {
        *realization = MINE;
    } else if (strcmp(arguments[1], "--theirs") == 0) {
        *realization = THEIRS;
    } else {
        error("%s", "Incorrect type of realization");
        return 1;
    }
    strcpy(input_filename, arguments[2]);
    strcpy(output_filename, arguments[3]);
    if (is_filename_incorrect(input_filename, ".bmp") || is_filename_incorrect(output_filename, ".bmp")) {
        error("%s", "File must be in bmp format");
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    REALIZATION_TYPE realization;
    char input_filename[MAX_FILENAME_SIZE];
    char output_filename[MAX_FILENAME_SIZE];
    if (scan_arguments(argc, argv, &realization, input_filename, output_filename)) {
        return -1;
    }
    if (realization == MINE) {
        BMPv3* image = read_BMPv3_file(input_filename);
        BMP_ERROR_CHECK(stderr, -2);
        if (image->header.bits_per_pixel == 24) {
            for (unsigned long int i = 0; i < image->header.width * image->header.height * BYTES_COUNT_IN_PIXEL; i++) {
                image->data[i] = ~image->data[i];
            }
        }
        else if (image->header.bits_per_pixel == 8) {
            for (int i = 0; i < PALETTE_SIZE_8bbp; i++) {
                if ((i + 1) % 4 != 0) {
                    image->palette[i] = ~image->palette[i];
                }
            }
        }
        else {
            error("%s", "File is not a supported BMP variant");
            return -1;
        }
        write_BMPv3_file(image, output_filename);
        BMP_ERROR_CHECK(stderr, -1);
    } else if (realization == THEIRS) {
        BMP* image = BMP_ReadFile(input_filename);
        BMP_CHECK_ERROR(stdout, -2);
        unsigned long int width = BMP_GetWidth(image);
        unsigned long int height = BMP_GetHeight(image);
        unsigned char r, g, b;
        if (image->Header.BitsPerPixel == 24) {
            for (unsigned long int x = 0; x < width; ++x) {
                for (unsigned long int y = 0 ;y < height; ++y) {
                    BMP_GetPixelRGB(image, x, y, &r, &g, &b);
                    BMP_SetPixelRGB(image, x, y, 255 - r, 255 - g, 255 - b);
                }
            }
        } else if (image->Header.BitsPerPixel == 8) {
            for (int i = 0; i < PALETTE_SIZE_8bbp; i++) {
                if ((i + 1) % 4 != 0) {
                    image->Palette[i] = ~image->Palette[i];
                }
            }
        } else {
            error("%s", "File is not a supported BMP variant");
            return -1;
        }
        BMP_WriteFile(image, output_filename);
        BMP_CHECK_ERROR(stdout, -1);
    }
    return 0;
}
