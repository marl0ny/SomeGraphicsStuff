#ifdef __cplusplus
extern "C" {
#endif

# ifndef _READ_BITMAP_FILE_
# define _READ_BITMAP_FILE_


/*
* Load a bitmap file.
*
* ---------------------------------------------------------
* References:
*
* Wikipedia contributors. (2020). BMP file format.
* In Wikipedia, The Free Encyclopedia.
* https://en.wikipedia.org/wiki/Hue
*/
unsigned char *get_bitmap_contents(char *filename, int *w, int *h);

void write_to_bitmap(char *fname, unsigned char *data, int w, int h);

# endif

#ifdef __cplusplus
}
#endif
