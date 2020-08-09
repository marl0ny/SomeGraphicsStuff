#include <stdio.h>
#include <stdlib.h>
#include "read_file.h"


/* 
* Invert the endianess of a single byte.
*/
unsigned char invert_endian(unsigned char byte) {
    unsigned char reversed_byte = 0;
    unsigned char bit_u = 1;
    unsigned char bit_d = 128;
    while (bit_u < bit_d) {
        reversed_byte += bit_d*(byte & bit_u)/bit_u;
        bit_u *= 2;
        bit_d /= 2;
    }
    return reversed_byte;
}


void invert_data(unsigned char *arr, int n) {
    int half_n = (n % 2)? n/2 + 1: n/2;
    for (int i = 0; i < half_n; i++) {
        arr[i] = invert_endian(arr[i]);
        if ((i == half_n - 1) && !(n % 2)) {
            arr[n - i - 1] = invert_endian(arr[n - i - 1]);
        }
        unsigned char temp = arr[i];
        arr[i] = arr[n - i - 1];
        arr[n - i - 1] = temp;
    }
}


char *get_file_contents(char *filename) {
    FILE *f = fopen(filename, "r"); 
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *file_buff = malloc(file_size + 1);
    if (file_buff == NULL) {
        perror("malloc");
        fclose(f);
        return NULL;
    }
    fread(file_buff, file_size, 1, f);
    file_buff[file_size] = '\0';
    fclose(f);
    return file_buff;
}


unsigned char *get_bitmap_contents(char *filename, int *w, int *h) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return NULL;
    }
    int offset;
    short pixel_size;
    int compression;
    fseek(f, 10, SEEK_SET);
    fread(&offset, sizeof(int), 1, f);
    fseek(f, 18, SEEK_SET);
    fread(w, sizeof(int), 1, f);
    fseek(f, 22, SEEK_SET);
    fread(h, sizeof(int), 1, f);
    fseek(f, 28, SEEK_SET);
    fread(&pixel_size, sizeof(short), 1, f);
    if (pixel_size != 24) {
        fprintf(stderr, 
                "Only 24 bits per pixel format "
                "currently supported.");
        return NULL;
    }
    fseek(f, 30, SEEK_SET);
    fread(&compression, sizeof(int), 1, f);
    if (compression) {
        fprintf(stderr, "Compression currently not supported.");
        return NULL;
    }
    fseek(f, offset, SEEK_SET);
    // Note that there is an offset after every row
    // of the image data so that the row byte size is 
    // divisible by 4. 
    // Look at the first example found on the 
    // Wikipedia page for the bmp file format:
    // https://en.wikipedia.org/wiki/BMP_file_format
    int padding = ((*w*3) % 4)? 4 - (*w*3) % 4: 0;
    int d = (*w)*(*h);
    unsigned char *image_data = malloc(
        (d*3 + *h*padding)*sizeof(unsigned char));
    if (image_data == NULL) {
        perror("malloc");
        return NULL;
    }
    fread(image_data, sizeof(unsigned char), (d*3 + *h*padding), f);
    for (int i = 0; i < (d*3 + *h*padding); i+= *w*3 + padding) {
        for (int j = 0; j < *w; j++) {
            unsigned char temp = image_data[i+j*3+2];
            image_data[i+j*3+2] = image_data[i+j*3];
            image_data[i+j*3] = temp;
        }
    }
    return image_data;
}
