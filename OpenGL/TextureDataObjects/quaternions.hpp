#ifndef _QUATERNIONS_
#define _QUATERNIONS_

struct Quaternion {
    union {
        struct {
            double x, y, z, w;
        };
        struct {
            double i, j, k, real;
        };
        struct {
            double ind[4];
        };
    };
    Quaternion conj() const;
    double length_squared() const;
    double length() const;
    Quaternion normalized() const;
    Quaternion inverse() const;
    Quaternion operator+(double a) const;
    Quaternion operator-(double a) const;
    Quaternion operator*(double a) const;
    Quaternion operator/(double a) const;
    Quaternion operator+(Quaternion q) const;
    Quaternion operator-(Quaternion q) const;
    Quaternion operator*(Quaternion q) const;
    Quaternion operator/(Quaternion q) const;
    double operator[](int index) const;
    double &operator[](int index);
};

Quaternion operator+(double a, Quaternion q);

Quaternion operator-(double a, Quaternion q);

Quaternion operator*(double a, Quaternion q);

Quaternion operator/(double a, Quaternion q);

Quaternion exp(Quaternion q);

Quaternion log(Quaternion q);

Quaternion rotator(double angle, 
                   double x, double y, double z);

#endif