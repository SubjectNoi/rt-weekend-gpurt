#ifndef RAY_H_
#define RAY_H_

#include "vec3.hpp"

class ray {
public:
    ray() {}
    ray(const point3& origin, const point3& direction) : orig(origin), dir(direction) {}
    point3 origin() const { return orig; }
    vec3 direction() const { return dir; } 

    point3 at(double t) const {
        return orig + t * dir;
    }
public:
    point3 orig;
    vec3 dir;
};

#endif // RAY_H_