#ifndef __EMSCRIPTEN__
#include <png.h>
#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>

#define header_size 8


/*static char *s_buf;
static int s_buf_size;

static int
read_chunk_callback(png_struct *read_ptr, png_unknown_chunk *chunk) {
    char name[5];
    for (int i = 0; (name[i] = chunk->name[i]); i++);
    const png_byte *data = chunk->data;
    size_t size = chunk->size;
    printf("%d\n", size);
    printf("%s\n", name);
    for (int i = 0; i < size; i++) {
        s_buf[i] = data[i];
    }
    s_buf += size;
    return size;
    }*/

int read_png(const char *fname, unsigned char *buf, int buf_size) {
    // s_buf = buf;
    // s_buf_size = buf_size;
    FILE *f = fopen(fname, "rb");
    if (!f) {
        perror("fopen");
        return -1;
    }
    /*char header[header_size] = {'\0',};
    if (fread(header, 1, header_size, f) != header_size) {
        fprintf(stderr, "read_png: unable to read header of file.\n");
        fclose(f);
        return -1;
    }
    if (png_sig_cmp(header, 0, header_size)) {
        fprintf(stderr, "read_png: %s is not a png file.\n", fname);
        fclose(f);
        return -1;
        }*/
    png_struct *read_ptr
        = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                 NULL, NULL, NULL);
    if (!read_ptr) {
        fprintf(stderr,
                "read_png: png_create_read_struct returned NULL\n");
        return -1;
    }

    png_info *info_ptr = png_create_info_struct(read_ptr);
    if (!info_ptr) {
        fprintf(stderr,
                "read_png: png_create_info_struct returned NULL\n");
        png_destroy_read_struct(&read_ptr, NULL, NULL);
        return -1;
    }
    if (setjmp(png_jmpbuf(read_ptr))) {
        png_destroy_read_struct(&read_ptr, &info_ptr, NULL);
        fclose(f);
        return -1;
    }

    png_init_io(read_ptr, f);
    // png_set_sig_bytes(read_ptr, header_size);
    png_read_png(read_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
    png_byte **row_pointers;
    unsigned int width, height;
    int bit_depth;
    int colour_type, interlace_method;
    int compression_type, filter_method;
    png_get_IHDR(read_ptr, info_ptr, &width, &height, &bit_depth, &colour_type,
                 &interlace_method, &compression_type, &filter_method);
    int channel_length = png_get_channels(read_ptr, info_ptr);
    if (bit_depth != 8) {
        fclose(f);
        fprintf(stderr,
                "read_png: unsupported bit depth size of %d.\n", bit_depth);
        png_read_end(read_ptr, info_ptr);
        return -1;
    }
    if (width*height*channel_length > buf_size) {
        fclose(f);
        fprintf(stderr, "read_png: buffer not big enough.\n");
        png_read_end(read_ptr, info_ptr);
        return -1;
    }
    printf("width: %d, height: %d\n"
           "bit_depth: %d\n"
           "colour_type: %d\n"
           "interlace_method: %d\n"
           "compression_type: %d\n"
           "filter_method: %d\n",
           width, height, bit_depth, colour_type,
           interlace_method, compression_type, filter_method);
    row_pointers = png_get_rows(read_ptr, info_ptr);
    for (int i = 0; i < height; i++) {
        for (int k = 0; k < width*channel_length; k++)
            buf[i*width*channel_length + k] = row_pointers[height-i-1][k];
    }
    /*  png_unknown_chunk chunk;
    png_set_read_user_chunk_fn(read_ptr, &chunk, read_chunk_callback);
    fclose(f);*/
    png_read_end(read_ptr, NULL);
    fclose(f);
    return 0;
}
#endif