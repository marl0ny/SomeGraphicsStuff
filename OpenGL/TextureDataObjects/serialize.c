#include "serialize.h"
#include "frames_stacks.h"

/* TODO: Complete this! */
static int byte_size(int format) {
    switch(format) {
    case GL_RGBA32F: case GL_RGBA32I: case GL_RGBA32UI:
        return 16;
    case GL_RGB32F: case GL_RGB32I: case GL_RGB32UI:
        return 12;
    case GL_RG32F: case GL_RG32I: case GL_RG32UI:
    case GL_RGBA16F: case GL_RGBA16I: case GL_RGBA16UI:
        return 8;
    case GL_RGB16F: case GL_RGB16I: case GL_RGB16UI:
        return 6;
    case GL_R32F: case GL_R32I: case GL_R32UI: case GL_RGBA8:
    case GL_RG16F: case GL_RG16I: case GL_RG16UI:
        return 4;
    case GL_RGB: case GL_RGB8UI: case GL_RGB8I:
        return 3;
    case GL_RG8: case GL_RG8UI: case GL_R16I: case GL_R16UI: case GL_R16F:
        return 2; // 16 bits
    case GL_R8: case GL_R8UI:
        return 1;
    }
    return -1;
}

struct {
    int format;
    int width;
    int height;
    int generate_mipmap;
    int wrap_s;
    int wrap_t;
    int min_filter;
    int mag_filter;
} header;


void serialize(const char *fname, int quad_id,
               const struct TextureParams *tex_params) {
    FILE *f_ptr = fopen(fname, "wb");
    if (!f_ptr) {
        perror("fopen");
        return;
    }
    header.format = tex_params->format;
    header.width = tex_params->width;
    header.height = tex_params->height;
    header.generate_mipmap = tex_params->generate_mipmap;
    header.wrap_s = tex_params->wrap_s;
    header.wrap_t = tex_params->wrap_t;
    header.min_filter = tex_params->min_filter;
    header.mag_filter = tex_params->mag_filter;
    int array_size = byte_size(header.format)*header.width*header.height;
    char *array = malloc(byte_size(header.format)
                                         *header.width*header.height);
    get_quad_array(quad_id, tex_params, array);
    int h_file_wrote = fwrite(&header, 1, sizeof(header), f_ptr);
    if (h_file_wrote != sizeof(header)) {
        fprintf(stderr, "serialize: unable to write header.\n");
        fclose(f_ptr);
        return;
    }
    int data_wrote = fwrite(array, 1, array_size, f_ptr);
    if (data_wrote != array_size) {
        fprintf(stderr, "serialize: unable to write data.\n");
        fclose(f_ptr);
        return;
    }
    fclose(f_ptr);
    free(array);
}

int deserialize(const char *fname,
                struct TextureParams *tex_params) {
    FILE *f_ptr = fopen(fname, "rb");
    if (!f_ptr) {
        perror("fopen");
        fclose(f_ptr);
        return -1;
    }
    if (fread(&header,
        1, sizeof(header), f_ptr) != sizeof(header)) {
        fprintf(stderr, "deserialize: unable to read header.\n");
        return -1;
    }
    if (header.width < 0 || header.height < 0) {
        fprintf(stderr, "deserialize: invalid dimensions.\n");
    }
    int array_size = header.width*header.height*byte_size(header.format);
    char *array = malloc(array_size);
    if (fread(array, 1, array_size, f_ptr) != array_size) {
        fprintf(stderr, "deserialize: unable to write array.\n");
        return -1;
    }
    tex_params->format = header.format;
    tex_params->generate_mipmap = header.generate_mipmap;
    tex_params->width = header.width;
    tex_params->height = header.height;
    tex_params->min_filter = header.min_filter;
    tex_params->mag_filter = header.mag_filter;
    tex_params->wrap_s = header.wrap_s;
    tex_params->wrap_t = header.wrap_t;
    frame_id new_frame = activate_frame(tex_params);
    quad_substitute_array(new_frame, tex_params, array);
    fclose(f_ptr);
    return new_frame;
}