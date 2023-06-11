#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"
#include <string.h>
// #include <byteswap.h>

// #define swap32 __bswap_32
// #define swap16 __bswap_16

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
        fclose(f);
        fprintf(stderr,
                "Unsupported pixel %d bit size.\n", pixel_size);
        return NULL;
    }
    fseek(f, 30, SEEK_SET);
    fread(&compression, sizeof(int), 1, f);
    if (compression) {
        fclose(f);
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
    int padding = ((*w*3) % 4)? (4 - (*w*3) % 4): 0;
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

static struct __attribute__((packed, scalar_storage_order("little-endian"))) {
    char file[14];
    struct {
        unsigned int size;
        int width;
        int height;
        unsigned short n_planes;
        unsigned short bits_per_pixel;
        unsigned int compression_method;
        unsigned int image_size;
        int h_resolution;
        int v_resolution;
        unsigned int n_colours_palette;
        unsigned int n_important_colours;
    } info;
} header = {{'B', 'M', 0, },
            {40, 0, 0, 1, 24, 0, 0, 0, 0, 16777216, 0}};


void write_to_bitmap(char *fname, unsigned char *data, int w, int h) {
    FILE *f = fopen(fname, "wb");
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return;
    }
    // Assuming little endian
    int size = 54 + w*h*3;
    int offset = 54;
    memcpy(&header.file[2], &size, 4);
    memcpy(&header.file[10], &offset, 4);
    header.info.size = 40;
    header.info.width = w;
    header.info.height = h;
    header.info.n_planes = 1;
    header.info.bits_per_pixel = 24;
    header.info.compression_method = 0;
    header.info.image_size = w*h*3;
    header.info.h_resolution = 100;
    header.info.v_resolution = 100;
    header.info.n_colours_palette = 256*256*256;
    header.info.n_important_colours = 0;
    int h_file_wrote = fwrite(&header.file, 1, 14, f);
    if (h_file_wrote != 14) {
        fprintf(stderr, "Error\n");
        fclose(f);
        return;
    }
    int h_info_wrote = fwrite(&header.info, 1, header.info.size, f);
    if (h_info_wrote != header.info.size) {
        fprintf(stderr, "Error\n");
        fclose(f);
        return;
    }
    int data_wrote  = fwrite(data, 1, 3*w*h, f);
    if (data_wrote != 3*w*h) {
        fprintf(stderr, "Error\n");
        fclose(f);
        return;
    }
    fclose(f);

}
