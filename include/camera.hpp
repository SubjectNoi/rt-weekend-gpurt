#ifndef CAMERA_H_
#define CAMERA_H_

#include "utils.hpp"

enum PROJECTION_TYPE {
    PROJECTION_PARALLEL = 0,
    PROJECTION_ORTHO = 1,
    PROJECTION_NUM = 2,
};

class camera {
public:
    camera(point3 lookfrom, point3 lookat, vec3 vup, double vfov, double aspect_ratio, double aperture, double focus_dist, double _time0=0, double _time1=0) {
        auto theta = degrees_to_radians(vfov);
        auto h = tan(theta / 2.0);
        auto viewport_height = 2.0 * h;
        auto viewport_width = aspect_ratio * viewport_height;

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);
        
        origin = lookfrom;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;
        len_radius = aperture / 2.0;
        type = PROJECTION_ORTHO;
        time0 = _time0;
        time1 = _time1;
    }

    ray get_ray(double s, double t) const {
        if (type == PROJECTION_ORTHO) {
            vec3 rd = len_radius * random_in_unit_disc();
            vec3 offset = u * rd.x() + v * rd.y();
            return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, random_double(time0, time1));
        }
        else if (type == PROJECTION_PARALLEL) {
            point3 p(lower_left_corner + s * horizontal + t * vertical);
            return ray(point3(p.x(), p.y(), 0), vec3(0, 0, -1));
        }
    }
private:
    point3 origin;
    point3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    PROJECTION_TYPE type;

    vec3 u, v, w;
    double len_radius;
    
    double time0, time1;
};

#endif