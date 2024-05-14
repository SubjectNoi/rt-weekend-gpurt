#ifndef RAY_H_
#define RAY_H_

#include "vec3.hpp"

class ray {
public:
    ray() {}
    ray(const point3& origin, const point3& direction, double time=0.0) : orig(origin), dir(direction), tm(time) {}
    point3 origin() const { return orig; }
    vec3 direction() const { return dir; } 
    double time() const { return tm; }

    point3 at(double t) const {
        return orig + t * dir;
    }
public:
    point3 orig;
    vec3 dir;
    double tm;  // This is ray launch time stamp, just to determine the moving status of objects, nothing to do with tmin/tmax
};

#endif // RAY_H_