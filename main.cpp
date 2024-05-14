#include "utils.hpp"
#include "color.hpp"
#include "hittable_list.hpp"
#include "sphere.hpp"
#include "moving_sphere.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "bvh.hpp"
#include "aarect.hpp"
#include "box.hpp"
#include "constant_medium.hpp"
#include <iostream>
#include <fstream>
//#include <omp.h>

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

color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
    hit_record rec;
    if (depth <= 0) return color(0, 0, 0);
    if (!world.hit(r, 0.0001, infinity, rec)) return background;
    ray scattered;
    color attenuation;
    color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);
    if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
        return emitted;
    }
    return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list two_spheres() {
    hittable_list objects;
    auto checker = make_shared<checker_texture>(color(0.91, 0.17, 0.36), color(0.99, 0.79, 0.33));
    objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
    objects.add(make_shared<sphere>(point3(0,  10, 0), 10, make_shared<lambertian>(checker)));
    return objects;
}

hittable_list two_perlin_spheres() {
    hittable_list objects;

    auto pertext = make_shared<noise_texture>(10);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));
    return objects;
}

hittable_list simple_light() {
    hittable_list objects;
    
    auto material1 = make_shared<dielectric>(1.5);
    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    auto pertext = make_shared<noise_texture>(5);
    objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
    objects.add(make_shared<sphere>(point3(5, 1, 3), 1, material1));
    objects.add(make_shared<sphere>(point3(0, 2, 0), 2, material2));
    objects.add(make_shared<sphere>(point3(0, 2, 5), 2, material3));
    objects.add(make_shared<sphere>(point3(12, 1, 2.5), 1, material1));
    objects.add(make_shared<sphere>(point3(12, 1, 2.5), -0.999, material1));
    objects.add(make_shared<xy_rect>(6, 8, 0, 2, 5.5, material3));
    auto difflight = make_shared<diffuse_light>(color(5, 5, 5));
    // objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));
    // objects.add(make_shared<xy_rect>(3, 5, 1, 3, 4, difflight));
    objects.add(make_shared<sphere>(point3(8, 0.3, 4), 0.3, difflight));
    objects.add(make_shared<xy_rect>(-10, 10, 0, 20, -4, difflight));
    
    return objects;
}

hittable_list cornell_box() {
    hittable_list objects;

    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(8, 8, 8));
    auto mirror = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    auto glass = make_shared<dielectric>(1.1);
    auto img_tex = make_shared<image_texture>("/Users/zihanliu/workspace/rt-weekend-gpurt/Tex.jpg");
    auto img_mat = make_shared<lambertian>(img_tex);

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(152.5, 402.5, 152.5, 402.5, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, img_mat));

    // objects.add(make_shared<box>(point3(130, 0.01, 65), point3(295, 165, 230), glass));
    // objects.add(make_shared<box>(point3(265, 0, 295), point3(430, 330, 460), mirror));
    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(180, 360, 180), glass);
    box1 = make_shared<rotate_y>(box1, 30);
    box1 = make_shared<translate>(box1, vec3(265, 0.01, 295));

    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(180, 180, 180), mirror);
    box2 = make_shared<rotate_y>(box2, -15);
    box2 = make_shared<translate>(box2, vec3(130, 0.01, 65));

    objects.add(box1);
    objects.add(box2);

    return objects;
}

hittable_list smoke_box() {
    hittable_list objects;
    auto red = make_shared<lambertian>(color(0.65, 0.05, 0.05));
    auto white = make_shared<lambertian>(color(0.73, 0.73, 0.73));
    auto blue = make_shared<lambertian>(color(0.08, 0.13, 0.72));
    auto green = make_shared<lambertian>(color(0.12, 0.45, 0.15));
    auto light = make_shared<diffuse_light>(color(8, 8, 8));    
    auto img_tex = make_shared<image_texture>("/Users/zihanliu/workspace/rt-weekend-gpurt/Tex2.jpg");
    auto img_mat = make_shared<lambertian>(img_tex);
    auto glass = make_shared<dielectric>(1.2);

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(152.5, 402.5, 152.5, 402.5, 554.9, light));
    objects.add(make_shared<yz_rect>(350, 500, 152.5, 402.5, 554.9, light));
    objects.add(make_shared<yz_rect>(350, 500, 152.5, 402.5, 0.1, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, img_mat));
    
    shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(180, 180, 180), glass);
    box2 = make_shared<rotate_y>(box2, -15);
    box2 = make_shared<translate>(box2, vec3(130, 0.01, 65));
    objects.add(box2);
    shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(180, 240, 180), white);
    box1 = make_shared<rotate_y>(box1, 30);
    box1 = make_shared<translate>(box1, vec3(265, 0.01, 295));
    objects.add(make_shared<constant_medium>(box1, 0.005, color(1, 1, 1)));

    shared_ptr<hittable> sphere1 = make_shared<sphere>(point3(120, 360, 120), 90, white);
    objects.add(make_shared<constant_medium>(sphere1, 0.02, color(0, 0, 0)));
    return objects;
}

hittable_list random_scene() {
    hittable_list world;
    // auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    auto checker = make_shared<checker_texture>(color(0.91, 0.17, 0.36), color(0.99, 0.79, 0.33));
    world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_material = random_double();
            point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;
                if (choose_material < 0.5) {
                    auto albedo = color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0, 0.5), 0);
                    world.add(make_shared<moving_sphere>(center, center2, 0.0, 1.0, 0.2, sphere_material));
                }
                else if (choose_material < 0.75) {
                    auto albedo = color::random(0.5, 1.0);
                    auto fuzz = random_double(0.0, 0.25);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
                else {
                    sphere_material = make_shared<dielectric>(1.5, 1.0, 0.78, 0.86);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
    // world.add(make_shared<sphere>(point3(0, 1, 0), -0.99, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    return world;
}

int main(int argc, char** argv) {
    auto aspect_ratio = 16.0 / 9.0;
    int image_width = 1920;
    int image_height = static_cast<int>(image_width / aspect_ratio);
    int samples_per_pixel = 200;
    int max_depth = 50;

    double ***img = new double** [image_height];
    for (int j = 0; j < image_height; j++) img[j] = new double* [image_width];
    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            img[j][i] = new double[3];
        }
    }

    // // World
    // auto world = random_scene();
    // hittable_list bvh_world;
    // bvh_world.add(make_shared<bvh_node>(world, 0, 1));


    // // Camera
    // point3 look_from(13, 2, 3);
    // point3 look_at(0, 0, 0);
    // vec3 vup(0, 1, 0);
    // auto dist_to_focus = 10.0;
    // auto aperture = 0.1;
    // camera cam(look_from, look_at, vup, 30, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    hittable_list world;
    point3 lookfrom;
    point3 lookat;
    auto vfov = 40.0;
    auto aperture = 0.0;
    color background(0, 0, 0);

    switch (7) {
        case 1:
            world = random_scene();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            aperture = 0.1;
            break;
        case 2:
            world = two_spheres();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            break;
        case 3:
            world = two_perlin_spheres();
            background = color(0.70, 0.80, 1.00);
            lookfrom = point3(13, 2, 3);
            lookat = point3(0, 0, 0);
            vfov = 20.0;
            break;
        case 4:
            break;
        case 5:
            world = simple_light();
            lookfrom = point3(26, 3, 2);
            lookat = point3(0, 2, 0);
            background = color(0, 0, 0);
            vfov = 20.0;
            break;
        default:
        case 6:
            world = cornell_box();
            aspect_ratio = 1.0;
            image_width = 800;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 500;
            background = color(0, 0, 0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
        case 7:
            world = smoke_box();
            aspect_ratio = 1.0;
            image_width = 1000;
            image_height = static_cast<int>(image_width / aspect_ratio);
            samples_per_pixel = 500;
            background = color(0, 0, 0);
            lookfrom = point3(278, 278, -800);
            lookat = point3(278, 278, 0);
            vfov = 40.0;
            break;
    }    
    hittable_list bvh_world;
    bvh_world.add(make_shared<bvh_node>(world, 0, 1));

    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    color **pixel_colors;
    pixel_colors = new color* [image_height];
    for (int j = 0; j < image_height; j++) pixel_colors[j] = new color [image_width];
    for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
            pixel_colors[j][i] = color(0, 0, 0);
        }
    }
    for (int s = 0; s < samples_per_pixel; s++) {
        // #pragma omp parallel for
        for (int j = image_height - 1; j >= 0; j--) {
            for (int i = 0; i < image_width; i++) {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_colors[j][i] += ray_color(r, background, bvh_world, max_depth);
                auto pc = get_color(pixel_colors[j][i], s);
                img[j][i][0] = pc[0];
                img[j][i][1] = pc[1];
                img[j][i][2] = pc[2];
            }
        }
        std::ofstream fout("/Users/zihanliu/workspace/rt-weekend-gpurt/render_output/img_" + std::to_string(s) + ".ppm", std::ios::out);
        fout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
        for (int j = image_height - 1; j >= 0; j--) {
            for (int i = 0; i < image_width; i++) {
                fout << static_cast<int>(256 * clamp(img[j][i][0], 0.0, 0.999)) << ' '
                     << static_cast<int>(256 * clamp(img[j][i][1], 0.0, 0.999)) << ' '
                     << static_cast<int>(256 * clamp(img[j][i][2], 0.0, 0.999)) << '\n';
            }
        }
        fout.close();
        std::cout << s << std::endl;
        // #pragma omp barrier
    }
    // for (int j = image_height - 1; j >= 0; --j) {
    //     for (int i = 0; i < image_width; ++i) {
    //         std::cout << static_cast<int>(256 * clamp(img[j][i][0], 0.0, 0.999)) << ' '
    //                   << static_cast<int>(256 * clamp(img[j][i][1], 0.0, 0.999)) << ' '
    //                   << static_cast<int>(256 * clamp(img[j][i][2], 0.0, 0.999)) << '\n';
    //     }
    // }
    return 0;
}
