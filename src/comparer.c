#include <stdio.h>
#include <string.h>
#include "bmp_handler.h"
#include <math.h>

#define NORMAL_ARGUMENTS_COUNT 2
#define error(...) (fprintf(stderr, __VA_ARGS__))
#define BYTES_COUNT_IN_PIXEL 3
#define MAX_FILENAME_SIZE 255
#define BMP_PALETTE_SIZE_8bpp (256 * 4)

int compare_images(BMPv3* image1, BMPv3* image2) {
    if (image1->header.bits_per_pixel != image2->header.bits_per_pixel) {
        error("%s", "Images must be of the same bitness");
        return -1;
    }
    if (image1->header.width != image2->header.width || abs(image1->header.height) != abs(image2->header.height)) {
        error("%s", "Images must be equal size");
        return -1;
    }
    int width = image1->header.width;
    int height = abs(image1->header.height);
    int bits_per_pixel = image1->header.bits_per_pixel;
    int count_diff = 0;
    if (bits_per_pixel == 8) {
        for (int i = 0; i < BMP_PALETTE_SIZE_8bpp; i++) {
            if (image1->palette[i] != image2->palette[i]) {
                error("%s", "Images have different palettes");
                return 0;
            }
        }
        if (image1->header.height * image2->header.height > 0) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    //printf("%u %u\n", image1->data[y * width + x], image2->data[y * width + x]);
                    if (image1->data[y * width + x] != image2->data[y * width + x]) {
                        error("%d %d\n", x, y);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    //printf("%u %u\n", image1->data[y * width + x], image2->data[y * width + x]);
                    if (image1->data[(height - y - 1) * width + x] != image2->data[y * width + x]) {
                        error("%d %d\n", x, y);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        }
    } else if (bits_per_pixel == 24) {
        if (image1->header.height * image2->header.height > 0) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    //printf("%u %u\n", image1->data[y * width + x], image2->data[y * width + x]);
                    if (image1->data[3 * (y * width + x)] != image2->data[3 * (y * width + x)] ||
                        image1->data[3 * (y * width + x) + 1] != image2->data[3 * (y * width + x) + 1] ||
                        image1->data[3 * (y * width + x) + 2] != image2->data[3 * (y * width + x) + 2]) {
                        error("%d %d\n", x, y);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        } else {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    //printf("%u %u\n", image1->data[y * width + x], image2->data[y * width + x]);
                    if (image1->data[3 * ((height - y - 1) * width + x)] != image2->data[3 * (y * width + x)] ||
                        image1->data[3 * ((height - y - 1) * width + x) + 1] != image2->data[3 * (y * width + x) + 1] ||
                        image1->data[3 * ((height - y - 1) * width + x) + 2] != image2->data[3 * (y * width + x) + 2]) {
                        error("%d %d\n", x, height - y - 1);
                        count_diff++;
                    }
                    if (count_diff == 100) {
                        return 0;
                    }
                }
            }
        }

    }
    return 0;
}

int scan_arguments(int count_of_arguments, char** arguments,
                   char* input_filename1, char* input_filename2) {
    if (count_of_arguments - 1 != NORMAL_ARGUMENTS_COUNT) {
        error("%s", "Count of arguments must be 2");
        return 0;
    }
    strcpy(input_filename1, arguments[1]);
    strcpy(input_filename2, arguments[2]);
    return 1;
}

int main(int argc, char* argv[]) {
    char input_filename1[MAX_FILENAME_SIZE];
    char input_filename2[MAX_FILENAME_SIZE];
    if (!scan_arguments(argc, argv, input_filename1, input_filename2)) {
        return -1;
    }
    BMPv3* image1 = read_BMPv3_file(input_filename1);
    BMP_ERROR_CHECK(stderr, -2);
    BMPv3* image2 = read_BMPv3_file(input_filename2);
    BMP_ERROR_CHECK(stderr, -2);
    if (compare_images(image1, image2)) {
        return -1;
    }
    return 0;
}