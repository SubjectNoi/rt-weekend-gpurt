#include "utils.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include <iostream>
#include <omp.h>

double hit_sphere(const point3& center, double radius, const ray& r) {
    vec3 oc = r.origin() - center;
    auto a = r.direction().length_squared();
    auto half_b = dot(oc, r.direction());
    auto c = oc.length_squared() - radius * radius;
    auto delta = half_b * half_b - a * c;
    if (delta < 0) {
        return -1.0;
    }
    else {
        return (-half_b - sqrt(delta)) / a;  // Get the close hit point, since the far one is unvisible.
    }
}

color ray_color(const ray& r, const hittable& world, int depth) {
    hit_record rec;
    if (depth <= 0) return color(0, 0, 0);
    if (world.hit(r, 0.0001, infinity, rec)) {
        // point3 target = rec.p + rec.normal + random_in_unit_sphere();
        // point3 target = rec.p + rec.normal + random_unit_vector();
        // point3 target = rec.p + rec.normal + random_in_hemisphere(rec.normal);
        // return 0.5 * ray_color(ray(rec.p, target - rec.p), world, depth - 1); // 0.5 is absorb rate
        ray scattered;
        color attenuation; // Absorbtion rate of R,G,B
        if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
            return attenuation * ray_color(scattered, world, depth - 1);
        }
        return color(0, 0, 0);
    }
    vec3 unit_direction = unit_vector(r.direction());
    auto t = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
    hittable_list world;
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

    // for (int a = -11; a < 11; a++) {
    //     for (int b = -11; b < 11; b++) {
    //         auto choose_material = random_double();
    //         point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
    //         if ((center - point3(4, 0.2, 0)).length() > 0.9) {
    //             shared_ptr<material> sphere_material;
    //             if (choose_material < 0.5) {
    //                 auto albedo = color::random();
    //                 sphere_material = make_shared<lambertian>(albedo);
    //                 world.add(make_shared<sphere>(center, 0.2, sphere_material));
    //             }
    //             else if (choose_material < 0.75) {
    //                 auto albedo = color::random(0.5, 1.0);
    //                 auto fuzz = random_double(0.0, 0.5);
    //                 sphere_material = make_shared<metal>(albedo, fuzz);
    //                 world.add(make_shared<sphere>(center, 0.2, sphere_material));
    //             }
    //             else {
    //                 sphere_material = make_shared<dielectric>(1.5);
    //                 if (random_double() < 0.5) {
    //                     world.add(make_shared<sphere>(center, -0.19, sphere_material));
    //                 }
    //                 world.add(make_shared<sphere>(center, 0.2, sphere_material));
    //             }
    //         }
    //     }
    // }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

int main(int argc, char** argv) {
    const auto aspect_ratio = 16.0 / 9.0;
    const int image_width = 1600;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 50;
    const int max_depth = 50;

    double ***img = new double** [image_height];
    for (int j = 0; j < image_height; j++) img[j] = new double* [image_width];
    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            img[j][i] = new double[3];
        }
    }

    auto world = random_scene();

    point3 look_from(13, 2, 3);
    point3 look_at(0, 0, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.1;
    camera cam(look_from, look_at, vup, 30, aspect_ratio, aperture, dist_to_focus);
    omp_set_num_threads(64);
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    // #pragma omp parallel for
    // for (int j = image_height - 1; j >= 0; --j) {
    //     for (int i = 0; i < image_width; ++i) {
    //         color pixel_color(0, 0, 0);
    //         for (int s = 0; s < samples_per_pixel; s++) {
    //             auto u = (i + random_double()) / (image_width - 1);
    //             auto v = (j + random_double()) / (image_height - 1);
    //             ray r = cam.get_ray(u, v);
    //             pixel_color += ray_color(r, world, max_depth);
    //         }
    //         // write_color(std::cout, pixel_color, samples_per_pixel);
    //         auto pc = get_color(pixel_color, samples_per_pixel);
    //         img[j][i][0] = pc[0];
    //         img[j][i][1] = pc[1];
    //         img[j][i][2] = pc[2];
    //     }
    // }
    color **pixel_colors;
    pixel_colors = new color* [image_height];
    for (int j = 0; j < image_height; j++) pixel_colors[j] = new color [image_width];
    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            pixel_colors[j][i] = color(0, 0, 0);
        }
    }
    for (int s = 0; s < samples_per_pixel; s++) {
        #pragma omp parallel for
        for (int j = image_height - 1; j >= 0; j--) {
            for (int i = 0; i < image_width; i++) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_colors[j][i] += ray_color(r, world, max_depth);
                auto pc = get_color(pixel_colors[j][i], s);
                img[j][i][0] = pc[0];
                img[j][i][1] = pc[1];
                img[j][i][2] = pc[2];
            }
        }
        #pragma omp barrier
    }
    for (int j = image_height - 1; j >= 0; --j) {
        for (int i = 0; i < image_width; ++i) {
            std::cout << static_cast<int>(256 * clamp(img[j][i][0], 0.0, 0.999)) << ' '
                      << static_cast<int>(256 * clamp(img[j][i][1], 0.0, 0.999)) << ' '
                      << static_cast<int>(256 * clamp(img[j][i][2], 0.0, 0.999)) << '\n';
        }
    }
    return 0;
}
