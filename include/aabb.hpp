#ifndef AABB_H_
#define AABB_H_

#include "utils.hpp"

class aabb {
public:
    aabb() {}
    aabb(const point3& a, const point3& b) {
        minimum = a;
        maximum = b;
    }
    point3 min() const { return minimum; }
    point3 max() const { return maximum; }

    bool hit(const ray& r, double t_min, double t_max) const {
        for (int i = 0; i < 3; i++) {
            auto t0 = fmin((minimum[i] - r.origin()[i]) / r.direction()[i], (maximum[i] - r.origin()[i]) / r.direction()[i]);
            auto t1 = fmax((minimum[i] - r.origin()[i]) / r.direction()[i], (maximum[i] - r.origin()[i]) / r.direction()[i]);
            t_min = fmax(t0, t_min);
            t_max = fmin(t1, t_max);
            if (t_max - t_min < -1e-10) {
                return false;
            }
        }
        return true;
    }
public:
    point3 minimum;
    point3 maximum;
};

aabb surrounding_box(aabb box0, aabb box1) {
    point3 small(fmin(box0.min().x(), box1.min().x()), fmin(box0.min().y(), box1.min().y()), fmin(box0.min().z(), box1.min().z()));
    point3   big(fmax(box0.max().x(), box1.max().x()), fmax(box0.max().y(), box1.max().y()), fmax(box0.max().z(), box1.max().z()));
    return aabb(small, big);
}

#endif