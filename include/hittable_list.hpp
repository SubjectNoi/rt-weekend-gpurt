#ifndef HITTABLE_LIST_H_
#define HITTABLE_LIST_H_

#include "hittable.hpp"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable {
public:
    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }
    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(object); }
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
    virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;
public:
    std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record tmp;
    bool hit_anything = false;
    auto closest_so_far = t_max;
    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, tmp)) {
            hit_anything = true;
            closest_so_far = tmp.t;
            rec = tmp;
        }
    }
    return hit_anything;
}

bool hittable_list::bounding_box(double time0, double time1, aabb& output_box) const {
    if (objects.empty()) return false;
    aabb tmp_box;
    bool first_box = true;

    for (const auto& object : objects) {
        if (!object->bounding_box(time0, time1, tmp_box)) return false;
        output_box = first_box ? tmp_box : surrounding_box(tmp_box, output_box);
        first_box = false;
    }
    return true;
}

#endif // HITTABLE_LIST_H_