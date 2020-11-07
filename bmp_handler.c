//
// Created by Alexander Fedkin on 03.11.2020.
//

#include "bmp_handler.h"
#include <stdlib.h>

#define BMP_PALETTE_SIZE_8bpp (256 * 4)
#define HEADER_BYTES_SIZE 54

static BMPv3_STATUS BMP_LAST_ERROR_CODE = BMPv3_OK;

static const char* BMP_ERRORS[] = {
        "",
        "General error",
        "Could not allocate enough memory to complete the operation",
        "File input/output error",
        "File not found",
        "File is not a supported BMP variant (must be uncompressed 4, 8, 24 or 32 BPP)",
        "File is not a valid BMP image",
        "An argument is invalid or out of range",
        "The requested action is not compatible with the BMP's type"
};

const char* BMP_get_error_description() {
    if (BMP_LAST_ERROR_CODE > 0 && BMP_LAST_ERROR_CODE < BMPv3_ERROR_NUM) {
        return BMP_ERRORS[BMP_LAST_ERROR_CODE];
    } else {
        return NULL;
    }
}

BMPv3* read_BMPv3_file(char* filename) {
    BMPv3* bmp;
    FILE* f;
    long int palette_size = 0;
    if (filename == NULL) {
        BMP_LAST_ERROR_CODE = BMPv3_INVALID_ARGUMENT;
        return NULL;
    }
    bmp = (BMPv3*)calloc(1, sizeof(BMPv3));
    if (bmp == NULL) {
        BMP_LAST_ERROR_CODE = BMPv3_OUT_OF_MEMORY;
        return NULL;
    }
    f = fopen(filename, "rb");
    if (f == NULL) {
        BMP_LAST_ERROR_CODE = BMPv3_FILE_NOT_FOUND;
        free(bmp);
        return NULL;
    }
    if (read_header(bmp, f) != BMPv3_OK || bmp->header.magic != 0x4D42) {
        BMP_LAST_ERROR_CODE = BMPv3_FILE_INVALID;
        fclose(f);
        free(bmp);
        return NULL;
    }
    if (bmp->header.bits_per_pixel == 8) {
        palette_size = BMP_PALETTE_SIZE_8bpp;
    }
    if ((bmp->header.bits_per_pixel != 24 && bmp->header.bits_per_pixel != 8)
         || bmp->header.compression_type != 0 || bmp->header.header_size != 40) {
        BMP_LAST_ERROR_CODE = BMPv3_FILE_NOT_SUPPORTED;
        fclose(f);
        free(bmp);
        return NULL;
    }
    if (palette_size > 0) {
        bmp->palette = (unsigned char*)malloc(palette_size * sizeof(unsigned char));
        if (bmp->palette == NULL) {
            BMP_LAST_ERROR_CODE = BMPv3_OUT_OF_MEMORY;
            fclose(f);
            free(bmp);
            return NULL;
        }
        if (fread(bmp->palette, sizeof(unsigned char), palette_size, f) != palette_size) {
            BMP_LAST_ERROR_CODE = BMPv3_FILE_INVALID;
            fclose(f);
            free(bmp->palette);
            free(bmp);
            return NULL;
        }
    } else {
        bmp->palette = NULL;
    }
    bmp->data = (unsigned char*)malloc(bmp->header.image_data_size);
    if (bmp->data == NULL) {
        BMP_LAST_ERROR_CODE = BMPv3_OUT_OF_MEMORY;
        fclose(f);
        free(bmp->palette);
        free(bmp);
        return NULL;
    }
    if (fread(bmp->data, sizeof(unsigned char), bmp->header.image_data_size, f) != bmp->header.image_data_size) {
        BMP_LAST_ERROR_CODE = BMPv3_FILE_INVALID;
        fclose(f);
        free(bmp->data);
        free(bmp->palette);
        free(bmp);
        return NULL;
    }
    fclose(f);
    BMP_LAST_ERROR_CODE = BMPv3_OK;
    return bmp;
}

long int get_4byte_int(short first_byte_index, unsigned char* header_bytes) {
    short i = first_byte_index;
    long int x = header_bytes[i + 3] << 24 | header_bytes[i + 2] << 16 | header_bytes[i + 1] << 8 | header_bytes[i];
    return x;
}

short get_2byte_int(short first_byte_index, unsigned char* header_bytes) {
    short i = first_byte_index;
    short x = header_bytes[i + 1] << 8 | header_bytes[i];
    return x;
}

void write_4byte_hex(long int x, short first_element_index, unsigned char* array_of_bytes) {
    short i = first_element_index;
    array_of_bytes[i + 3] = (unsigned char)((x & 0xff000000) >> 24);
    array_of_bytes[i + 2] = (unsigned char)((x & 0x00ff0000) >> 16);
    array_of_bytes[i + 1] = (unsigned char)((x & 0x0000ff00) >> 8);
    array_of_bytes[i] = (unsigned char)((x & 0x000000ff) >> 0);
}

void write_2byte_hex(short x, short first_element_index, unsigned char* array_of_bytes) {
    short i = first_element_index;
    array_of_bytes[i + 1] = (unsigned char)((x & 0xff00) >> 8);
    array_of_bytes[i] = (unsigned char)((x & 0x00ff) >> 0);
}

int	read_header(BMPv3* bmp, FILE* f) {
    if (bmp == NULL || f == NULL) {
        return BMPv3_INVALID_ARGUMENT;
    }
    unsigned char header_bytes[HEADER_BYTES_SIZE];
    if (fread(header_bytes, HEADER_BYTES_SIZE, 1, f) != 1) {
        return BMPv3_IO_ERROR;
    }
    bmp->header.magic = get_2byte_int(0, header_bytes);
    bmp->header.file_size  = get_4byte_int(2, header_bytes);
    bmp->header.reserved1 = get_2byte_int(6, header_bytes);
    bmp->header.reserved2 = get_2byte_int(8, header_bytes);
    bmp->header.data_offset = get_4byte_int(10, header_bytes);
    bmp->header.header_size = get_4byte_int(14, header_bytes);
    bmp->header.width = get_4byte_int(18, header_bytes);
    bmp->header.height = get_4byte_int(22, header_bytes);
    bmp->header.planes = get_2byte_int(26, header_bytes);
    bmp->header.bits_per_pixel = get_2byte_int(28, header_bytes);
    bmp->header.compression_type = get_4byte_int(30, header_bytes);
    bmp->header.image_data_size = get_4byte_int(34, header_bytes);
    bmp->header.h_pixels_per_meter = get_4byte_int(38, header_bytes);
    bmp->header.v_pixels_per_meter = get_4byte_int(42, header_bytes);
    bmp->header.colors_used = get_4byte_int(46, header_bytes);
    bmp->header.colors_required = get_4byte_int(50, header_bytes);
    return BMPv3_OK;
}

void write_BMPv3_file(BMPv3* bmp, char* filename) {
    FILE* f;
    long int palette_size = 0;
    if (bmp->header.bits_per_pixel == 8) {
        palette_size = BMP_PALETTE_SIZE_8bpp;
    }
    if (filename == NULL) {
        BMP_LAST_ERROR_CODE = BMPv3_INVALID_ARGUMENT;
        return;
    }
    f = fopen(filename, "wb");
    if (f == NULL) {
        BMP_LAST_ERROR_CODE = BMPv3_IO_ERROR;
        return;
    }
    if (write_header(bmp, f) != BMPv3_OK) {
        BMP_LAST_ERROR_CODE = BMPv3_IO_ERROR;
        fclose(f);
        return;
    }
    if (palette_size > 0) {
        if (fwrite(bmp->palette, sizeof(unsigned char), palette_size, f) != palette_size) {
            BMP_LAST_ERROR_CODE = BMPv3_IO_ERROR;
            fclose(f);
            return;
        }
    }
    if (fwrite(bmp->data, sizeof(unsigned char), bmp->header.image_data_size, f) != bmp->header.image_data_size) {
        BMP_LAST_ERROR_CODE = BMPv3_IO_ERROR;
        fclose(f);
        return;
    }
    BMP_LAST_ERROR_CODE = BMPv3_OK;
    fclose(f);
}

int write_header(BMPv3* bmp, FILE* f) {
    if (bmp == NULL || f == NULL) {
        return BMPv3_INVALID_ARGUMENT;
    }
    unsigned char array_of_bytes[HEADER_BYTES_SIZE];
    write_2byte_hex(bmp->header.magic, 0, array_of_bytes);
    write_4byte_hex(bmp->header.file_size, 2, array_of_bytes);
    write_2byte_hex(bmp->header.reserved1, 6, array_of_bytes);
    write_2byte_hex(bmp->header.reserved2, 8, array_of_bytes);
    write_4byte_hex(bmp->header.data_offset, 10, array_of_bytes);
    write_4byte_hex(bmp->header.header_size, 14, array_of_bytes);
    write_4byte_hex(bmp->header.width, 18, array_of_bytes);
    write_4byte_hex(bmp->header.height, 22, array_of_bytes);
    write_2byte_hex(bmp->header.planes, 26, array_of_bytes);
    write_2byte_hex(bmp->header.bits_per_pixel, 28, array_of_bytes);
    write_4byte_hex(bmp->header.compression_type, 30, array_of_bytes);
    write_4byte_hex(bmp->header.image_data_size, 34, array_of_bytes);
    write_4byte_hex(bmp->header.h_pixels_per_meter, 38, array_of_bytes);
    write_4byte_hex(bmp->header.v_pixels_per_meter, 42, array_of_bytes);
    write_4byte_hex(bmp->header.colors_used, 46, array_of_bytes);
    write_4byte_hex(bmp->header.colors_required, 50, array_of_bytes);
    if (fwrite(array_of_bytes, HEADER_BYTES_SIZE, 1, f) != 1) {
        return BMPv3_IO_ERROR;
    }
    return BMPv3_OK;
}

BMPv3_STATUS BMP_get_error()
{
    return BMP_LAST_ERROR_CODE;
}