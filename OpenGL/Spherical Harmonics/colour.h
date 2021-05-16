#include <complex>


struct Colour {
    union {
        struct {
            short red, green, blue, alpha;
        };
        struct {
            short r, g, b, a;
        };
    };
};


typedef float real;

template <typename T>
void complex_to_colour(struct Colour *col, std::complex<T> number) {
    real pi = 3.141592653589793;
    real arg_val = std::arg(number);
    int max_col = 255;
    int min_col = 50;
    int col_range = max_col - min_col;
    if (arg_val <= pi/3 && arg_val >= 0) {
        col->red = max_col;
        col->green = min_col + (int)(col_range*arg_val/(pi/3));
        col->blue = min_col;
    } else if (arg_val > pi/3 && arg_val <= 2*pi/3){
        col->red = max_col - (int)(col_range*(arg_val - pi/3)/(pi/3));
        col->green = max_col;
        col->blue = min_col;
    } else if (arg_val > 2*pi/3 && arg_val <= pi){
        col->red = min_col;
        col->green = max_col;
        col->blue = min_col + (int)(col_range*(arg_val - 2*pi/3)/(pi/3));
    } else if (arg_val < 0.0 && arg_val > -pi/3){
        col->red = max_col;
        col->blue = min_col - (int)(col_range*arg_val/(pi/3));
        col->green = min_col;
    } else if (arg_val <= -pi/3 && arg_val > -2*pi/3){
        col->red = max_col + (int)(col_range*(arg_val + pi/3)/(pi/3));
        col->blue = max_col;
        col->green = min_col;
    } else if (arg_val <= -2*pi/3 && arg_val >= -pi){
        col->red = min_col;
        col->blue = max_col;
        col->green = min_col - (int)(col_range*(arg_val + 2*pi/3)/(pi/3));
    }
    else {
        col->red = min_col;
        col->green = max_col;
        col->blue = max_col;
    }
}