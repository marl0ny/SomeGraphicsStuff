#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"


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
    if (pixel_size != 24 
        // && pixel_size != 32
        ) {
        fprintf(stderr, 
                "Unsupported pixel %d bit size.\n", pixel_size);
        return NULL;
    }
    fseek(f, 30, SEEK_SET);
    fread(&compression, sizeof(int), 1, f);
    if (compression) {
        fprintf(stderr, "Compression not supported.\n");
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
    fclose(f);
    return image_data;
}
