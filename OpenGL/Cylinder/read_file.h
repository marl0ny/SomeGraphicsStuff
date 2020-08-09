
# ifndef _READ_FILE
# define _READ_FILE


/*
* Get the contents of a text file given its filename.
*/
char *get_file_contents(char *filename);


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


# endif