#include <stdio.h>
#include <stdlib.h>
#include "rgb8_file.h"


unsigned char *get_rgb8_contents(char *filename, int *w, int *h) {
    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        perror("fopen");
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    fread(w, sizeof(int), 1, f);
    fseek(f, 4, SEEK_SET);
    fread(h, sizeof(int), 1, f);
    unsigned char *image_data = malloc(*w*(*h)*3*sizeof(unsigned char));
    if (image_data == NULL) {
        perror("malloc");
        return NULL;
    }
    fseek(f, 8, SEEK_SET);
    fread(image_data, sizeof(unsigned char), *w*(*h)*3, f);
    return image_data;
}
