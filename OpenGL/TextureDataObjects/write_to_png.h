#ifdef __cplusplus
extern "C" {
#endif

# ifndef _WRITE_TO_PNG_
# define _WRITE_TO_PNG_

int write_rgb8_png(const char *fname, 
                    unsigned char *data, int width, int height);

# endif

#ifdef __cplusplus
}
#endif