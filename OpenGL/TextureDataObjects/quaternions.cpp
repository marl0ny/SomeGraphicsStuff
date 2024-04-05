#include "quaternions.hpp"
#include <cmath>


Quaternion Quaternion::conj() const {
    return {.x=-this->x, .y=-this->y, .z=-this->z, .w=this->w};
}

double Quaternion::length_squared() const {
    return x*x + y*y + z*z + w*w;
}

double Quaternion::length() const {
    return sqrt(this->length_squared());
}

Quaternion Quaternion::normalized() const {
    double norm_factor = this->length();
    return {.x=this->x/norm_factor, .y=this->y/norm_factor,
            .z=this->z/norm_factor, .w=this->w/norm_factor};
}

Quaternion Quaternion::inverse() const {
    return this->conj()/this->length_squared();
}

Quaternion Quaternion::operator+(double a) const {
    return {.x=this->x+a, .y=this->y, .z=this->z, .w=this->w};
}

Quaternion Quaternion::operator-(double a) const {
    return {.x=this->x-a, .y=this->y, .z=this->z, .w=this->w};
}

Quaternion Quaternion::operator*(double a) const {
    return {.x=this->x*a, .y=this->y*a, .z=this->z*a, .w=this->w*a};
}

Quaternion Quaternion::operator/(double a) const {
    return {.x=this->x/a, .y=this->y/a, .z=this->z/a, .w=this->w/a};
}

Quaternion Quaternion::operator+(Quaternion q) const {
    return {.x=this->x+q.x, .y=this->y+q.y, .z=this->z+q.z, .w=this->w+q.w};
}

Quaternion Quaternion::operator-(Quaternion q) const {
    return {.x=this->x-q.x, .y=this->y-q.y, .z=this->z-q.z, .w=this->w-q.w};
}

Quaternion Quaternion::operator*(Quaternion q) const {
    return {
        .x = this->x*q.w + this->w*q.x + this->y*q.z - this->z*q.y,
        .y = this->y*q.w + this->w*q.y + this->z*q.x - this->x*q.z,
        .z = this->z*q.w + this->w*q.z + this->x*q.y - this->y*q.x,
        .w = this->w*q.w - this->x*q.x - this->y*q.y - this->z*q.z,
    };
}

Quaternion Quaternion::operator/(Quaternion q) const {
    return *this*q.inverse();
}

Quaternion operator+(double a, Quaternion q) {
    return q + a;
}

Quaternion operator-(double a, Quaternion q) {
    return {.x=q.x, .y=q.y, .z=q.z, .w=a-q.w};
}

Quaternion operator*(double a, Quaternion q) {
    return q*a;
}

Quaternion operator/(double a, Quaternion q) {
    return a*q.inverse();
}

double Quaternion::operator[](int index) const {
    return (index >= 0 && index < 4)? this->ind[index]: NAN;
}

double &Quaternion::operator[](int index) {
    return this->ind[index];
}

/*Take the exponent of the quaternion. This uses a
formula from the Quaternion wikipedia page:

https://en.wikipedia.org/wiki/Quaternion, 
"Exponential, logarithm, and power functions" section.
*/
Quaternion exp(Quaternion q) {
    double r = sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
    double e_w = exp(q.w);
    return {.x=e_w*q.x*sin(r)/r, .y=e_w*q.y*sin(r)/r, .z=e_w*q.z*sin(r)/r,
            .w=e_w*cos(r)};
}

/*Take the logarithm of a quaternion. This uses a 
formula from the Quaternion Wikipedia page:

https://en.wikipedia.org/wiki/Quaternion, 
"Exponential, logarithm, and power functions" section.
*/
Quaternion log(Quaternion q) {
    double r = sqrt(q.x*q.x + q.y*q.y + q.z*q.z);
    double len_q = q.length();
    double x = q.x, y = q.y, z = q.z, w = q.w;
    return {.x=x*acos(w/len_q)/r, .y=y*acos(w/len_q)/r, .z=z*acos(w/len_q)/r,
            .w=log(len_q)};
}

Quaternion rotator(
    double angle, double x, double y, double z) {
    double norm = sqrt(x*x + y*y + z*z);
    double c = cos(angle/2.0);
    double s = sin(angle/2.0);
    return {{{.x=s*x/norm, .y=s*y/norm, .z=s*z/norm, .w=c}}};
}
