#ifndef _FFT_
#define _FFT_


template <typename T>
void inplace_fft(T *z, int n);

template <typename T>
void inplace_ifft(T *z, int n);


#endif
