#ifndef HITTABLE_H_
#define HITTABLE_H_

#include "ray.hpp"
#include "utils.hpp"

class material;

struct hit_record {
    point3 p;       // Hit point
    vec3 normal;    // Normal of hit point
    shared_ptr <material> mat_ptr;
    double t;       // Hit time

    bool front_face;

    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif // HITTABLE_H_