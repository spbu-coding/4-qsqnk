//
// Created by Alexander Fedkin on 02.11.2020.
//

#include <stdio.h>

#ifndef HOMEWORK_4_BMP_HANDLER_H
#define HOMEWORK_4_BMP_HANDLER_H

typedef enum {
    BMPv3_OK = 0,
    BMPv3_ERROR,
    BMPv3_OUT_OF_MEMORY,
    BMPv3_IO_ERROR,
    BMPv3_FILE_NOT_FOUND,
    BMPv3_FILE_NOT_SUPPORTED,
    BMPv3_FILE_INVALID,
    BMPv3_INVALID_ARGUMENT,
    BMPv3_TYPE_MISMATCH,
    BMPv3_ERROR_NUM
} BMPv3_STATUS;

typedef struct BMPv3_header {
    short magic;
    long int file_size;
    short reserved1;
    short reserved2;
    long int data_offset;
    long int header_size;
    long int width;
    long int height;
    short planes;
    short bits_per_pixel;
    long int compression_type;
    long int image_data_size;
    long int h_pixels_per_meter;
    long int v_pixels_per_meter;
    long int colors_used;
    long int colors_required;
} BMPv3_Header;

typedef struct BMPv3 {
    BMPv3_Header header;
    unsigned char* palette;
    unsigned char* data;
} BMPv3;

BMPv3* read_BMPv3_file(char* filename);

void write_BMPv3_file(BMPv3* bmp, char* filename);

int write_header(BMPv3* bmp, FILE* f);

int	read_header(BMPv3* bmp, FILE* f);

BMPv3_STATUS BMP_get_error();

const char* BMP_get_error_description();

#define BMP_ERROR_CHECK(output_file, return_value) \
	if (BMP_get_error() != BMPv3_OK) \
	{\
		fprintf((output_file), "%s\n", BMP_get_error_description());\
		return(return_value);	\
	} \


#endif //HOMEWORK_4_BMP_HANDLER_H