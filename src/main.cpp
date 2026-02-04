// Creates scenes and call's Camera to render them.

#include "utility.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "texture.h"
#include "triangle.h"
#include "camera.h"
#include "bvh.h"
#include "quad.h"
#include "obj_mesh.h"
#include "volume.h"

void bouncing_spheres() {
    
    hittable_list world;
    
    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));
    auto checker_material = make_shared<lambertian>(checker);
    world.add(make_shared<Sphere>(point3(0,-1000,0), 1000, checker_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = random_vector() * random_vector();
                    sphere_material = make_shared<lambertian>(albedo);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = random_vector(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<specular>(albedo, fuzz);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<Sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<Sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<specular>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<Sphere>(point3(4, 1, 0), 1.0, material3));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 20;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.direction   = point3(0,0,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
    

}


void checkered_spheres() {
    hittable_list world;

    auto checker = make_shared<checker_texture>(0.32, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<Sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<Sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.direction   = point3(0,0,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);


}


void earth() {
    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<Sphere>(point3(1,0,0), 2, earth_surface);

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.position = point3(0,0,12);
    cam.direction   = point3(0,0,0);
    cam.up      = Vec3(0,1,0);
    cam.background        = color(0.70, 0.80, 1.00);

    cam.defocus_angle = 0;

    cam.render(hittable_list(globe));
}

void triangles() {
    hittable_list world;

    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    //world.add(make_shared<Sphere>(point3(0,0, -4), 0.5, make_shared<lambertian>(color(1, 0, 0))));
    world.add(make_shared<Triangle>(point3(-0.5, -0.5, -3.0), point3(0.5, -0.5, -6.0), point3(0.0, 0.5, -4.0), earth_surface));

    world.add(make_shared<Sphere>(point3(0,-1, -4), 0.5, make_shared<lambertian>(color(0, 1, 0))));
    world.add(make_shared<Sphere>(point3(1,1, -4), 0.5, make_shared<lambertian>(color(0, 0, 1))));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0.70, 0.80, 1.00);

    cam.vfov     = 40;
    cam.position = point3(0,0,0);
    cam.direction  = point3(0,0,-1);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}


void quads() {
    hittable_list world;

    // Materials
    auto material3 = make_shared<specular>(color(0.8, 0.85, 0.8), 0.0);
    auto material4 = make_shared<specular>(color(0.8, 0.85, 0.8), 0);
    //auto per_tex = make_shared<lambertian>(make_shared<marble_texture>(4));


    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);

    auto light_tex = make_shared<emissive>(color(4, 4, 4));

    auto disturbance = [](double t, const point3& origin) {
        return point3(0, -1.5*t, 0);
    };

    // Quads
    world.add(make_shared<quad>(point3(-3,-2, 5), Vec3(0, 0,-4), Vec3(0, 4, 0), material3));
    world.add(make_shared<quad>(point3(-2,-2, 0), Vec3(4, 0, 0), Vec3(0, 4, 0), material3));
    world.add(make_shared<quad>(point3( 3,-2, 1), Vec3(0, 0, 4), Vec3(0, 4, 0), material3));
    world.add(make_shared<quad>(point3(-2, 3, 1), Vec3(4, 0, 0), Vec3(0, 0, 4), material3));
    world.add(make_shared<quad>(point3(-2,-3, 5), Vec3(4, 0, 0), Vec3(0, 0,-4), material3));
    //world.add(make_shared<Sphere>(point3(0, 0, 0), 1.5, material4));
    //world.add(make_shared<Sphere>(point3(-2.0, 0, 0), 0.5, light_tex));

    auto glass1 = make_shared<Sphere>(point3(0,0,0), 1.8, make_shared<dielectric>(1.8));
    auto glass2 = make_shared<Sphere>(point3(0,0,0), 1.6, make_shared<dielectric>(1.6));
    auto glass3 = make_shared<Sphere>(point3(0,0,0), 1.4, make_shared<dielectric>(1.4));
    auto glass4 = make_shared<Sphere>(point3(0,0,0), 1.2, make_shared<dielectric>(1.2));
    auto glass5 = make_shared<Sphere>(point3(0,0,0), 1.0, make_shared<dielectric>(1.0));

    auto metal = make_shared<Sphere>(point3(0,0,0), 0.6, make_shared<specular>(color(1,1,1), 0.0));

    world.add(glass1);
    world.add(glass2);
    world.add(glass3);
    world.add(glass4);
    world.add(glass5);
    world.add(metal);

    Camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 1000;
    cam.samples_per_pixel = 200;//100
    cam.max_depth         = 50;//50
    cam.background        = color(0.70, 0.80, 1.00);
    cam.set_cubemap("cube_maps/Earth");


    cam.vfov     = 30; //165
    cam.position = point3(0,0,9); //4
    cam.direction   = point3(0,0,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void basic_lights() {
    hittable_list world;

    auto earth_texture = make_shared<image_texture>("earthmap.jpg");
    //auto earth_surface = make_shared<lambertian>(earth_texture);
    auto earth_surface = make_shared<lambertian>(earth_texture);


    world.add(make_shared<Sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(color(1, 0, 0))));
    world.add(make_shared<Sphere>(point3(0,2,0), 2, earth_surface));

    auto difflight = make_shared<emissive>(color(5,5,4));
    //world.add(make_shared<Sphere>(point3(0,7,0), 2, difflight));
    world.add(make_shared<quad>(point3(3,1,-2), Vec3(2,0,0), Vec3(0,2,0), difflight));
    world.add(make_shared<Sphere>(point3(0,5.5,0), 1, difflight));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 20;
    cam.position = point3(26,3,6);
    cam.direction   = point3(0,2,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

void cornell_box() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<emissive>(color(15, 15, 15));

    world.add(make_shared<quad>(point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(343, 554, 332), Vec3(-130,0,0), Vec3(0,0,-105), light));
    world.add(make_shared<quad>(point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), Vec3(-555,0,0), Vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));
    world.add(box(point3(130, 0, 65), point3(295, 165, 230), white));
    world.add(box(point3(265, 0, 295), point3(430, 330, 460), white));

    Camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 200; //64
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.position = point3(278, 278, -800);
    cam.direction   = point3(278, 278, 0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}



void first_model() {
    hittable_list world;

    auto smooth_die = load_obj_mesh("models/dice-obj/dicea_LOD3.obj", true);
    auto die = load_obj_mesh("models/dice-obj/dicea_LOD3.obj", false);

    //world.add(die);

    world.add(make_shared<Sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(color(1, 0, 0))));
    world.add(make_shared<translate>(smooth_die, Vec3(0, 0.5, 0)));
    world.add(make_shared<translate>(die, Vec3(-1, 0.5, 1.2)));

    auto difflight = make_shared<emissive>(color(5,5,4));
    //world.add(make_shared<Sphere>(point3(0,7,0), 2, difflight));
    world.add(make_shared<quad>(point3(3,1,-2), Vec3(2,0,0), Vec3(0,2,0), difflight));
    world.add(make_shared<Sphere>(point3(0,5.5,0), 1, difflight));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 13;
    cam.position = point3(5,5, 13);
    cam.direction   = point3(0,1,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}


void perlin() {
    hittable_list world;

    auto per_tex = make_shared<marble_texture>(4);
    //world.add(make_shared<Sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(per_tex)));
    world.add(make_shared<Sphere>(point3(0,2,0), 2, make_shared<lambertian>(per_tex)));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.direction   = point3(0,0,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background        = color(0.70, 0.80, 1.00);


    cam.render(world);
}

void cube_map() {
    hittable_list world;
    
    auto glass = make_shared<Sphere>(point3(0,1,0), 2, make_shared<dielectric>(1.5));
    auto metal = make_shared<Sphere>(point3(0,1,0), 1, make_shared<specular>(color(1,1,1), 0.0));

    //translations
    //auto glass_Lshifted = make_shared<translate>(glass, Vec3(0, 0, 3));
    //auto metal_Lshifted = make_shared<translate>(metal, Vec3(0, 0, 3));

    //auto glass_Rshifted = make_shared<translate>(glass, Vec3(0, 0, -3));
    //auto metal_Rshifted = make_shared<translate>(metal, Vec3(0, 0, -3));

    world.add(glass);
    //world.add(glass_Lshifted);
    //world.add(glass_Rshifted);
    world.add(metal);
    //world.add(metal_Lshifted);
    //world.add(metal_Rshifted);


    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 3000; //400
    cam.samples_per_pixel = 500; //100
    cam.max_depth         = 150; //50

    cam.vfov     = 35;
    cam.position = point3(13,2,3);
    cam.direction   = point3(0,0,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background        = color(0.70, 0.80, 1.00);
    cam.set_cubemap("cube_maps/Park2");

    cam.render(world);
}

 
void pretty_sphere() {
    hittable_list world;

    //auto per_tex = make_shared<marble_texture>(4);
    //world.add(make_shared<Sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(per_tex)));
    world.add(make_shared<Sphere>(point3(0,2,0), 2, make_shared<specular>(color(1,1,1), 0.0)));

    Camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 400;
    cam.samples_per_pixel = 100;
    cam.max_depth         = 50;

    cam.vfov     = 20;
    cam.position = point3(13,2,3);
    cam.direction   = point3(0,2,0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;
    cam.background        = color(0.70, 0.80, 1.00);
    cam.set_cubemap("cube_maps/Earth");

    cam.render(world);
}


void cornell_smoke() {
    hittable_list world;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<emissive>(color(7, 7, 7));

    world.add(make_shared<quad>(point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(113,554,127), Vec3(330,0,0), Vec3(0,0,305), light));
    world.add(make_shared<quad>(point3(0,555,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));

    shared_ptr<hittable> box1 = box(point3(0,0,0), point3(165,330,165), white);
    box1 = make_shared<translate>(box1, Vec3(265,0,295));

    shared_ptr<hittable> box2 = box(point3(0,0,0), point3(165,165,165), white);
    box2 = make_shared<translate>(box2, Vec3(130,0,65));

    world.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    world.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    Camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 600;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.position = point3(278, 278, -800);
    cam.direction   = point3(278, 278, 0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}


// Abstract marble scene with Perlin noise and complex lighting
void marble_gallery() {
    hittable_list world;

    auto marble_tex = make_shared<marble_texture>(5);
    auto marble_mat = make_shared<lambertian>(marble_tex);
    
    auto white = make_shared<lambertian>(color(.95, .95, .95));
    auto dark_gray = make_shared<lambertian>(color(.15, .15, .15));
    auto gold = make_shared<specular>(color(1.0, 0.85, 0.0), 0.1);
    
    // Marble spheres of varying sizes
    world.add(make_shared<Sphere>(point3(0, 1.5, 0), 1.5, marble_mat));
    world.add(make_shared<Sphere>(point3(-3.5, 0.6, -2), 0.6, gold));
    world.add(make_shared<Sphere>(point3(3, 1, 1), 1.0, marble_mat));
    world.add(make_shared<Sphere>(point3(0, 0.3, -4), 0.3, gold));
    
    // Floor with checkerboard
    auto floor_checker = make_shared<checker_texture>(0.5, color(0.2, 0.2, 0.2), color(0.9, 0.9, 0.9));
    world.add(make_shared<Sphere>(point3(0, -1000.3, 0), 1000, make_shared<lambertian>(floor_checker)));
    
    // Quad walls with different materials
    world.add(make_shared<quad>(point3(-6, -0.5, -6), Vec3(12, 0, 0), Vec3(0, 6, 0), dark_gray));
    world.add(make_shared<quad>(point3(-6, -0.5, 6), Vec3(12, 0, 0), Vec3(0, 6, 0), white));
    
    // Multiple light sources
    auto light1 = make_shared<emissive>(color(3, 3, 2.5));
    auto light2 = make_shared<emissive>(color(2.5, 2, 3));
    world.add(make_shared<Sphere>(point3(-2, 4, -3), 0.5, light1));
    world.add(make_shared<Sphere>(point3(2.5, 3.5, 2), 0.4, light2));
    world.add(make_shared<quad>(point3(-4, 5, -5), Vec3(4, 0, 0), Vec3(0, 0, 3), make_shared<emissive>(color(2, 2, 2))));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 150;
    cam.max_depth         = 50;
    cam.background        = color(0.1, 0.1, 0.15);

    cam.vfov     = 45;
    cam.position = point3(8, 4, 6);
    cam.direction   = point3(0, 0.5, 0);
    cam.up      = Vec3(0, 1, 0);

    cam.defocus_angle = 0.6;
    cam.focus_dist    = 10.0;

    cam.render(world);
}


// Motion blur showcase: animated spheres with trails
void motion_blur_symphony() {
    hittable_list world;

    // Define position functions for moving spheres
    auto sine_wave = [](double t, const point3& origin) {
        return point3(origin.x + 4 * sin(2 * pi * t), origin.y, origin.z);
    };

    auto sine_wave_offset = [](double t, const point3& origin) {
        return point3(origin.x + 4 * sin(2 * pi * t), origin.y + 0.3, origin.z - 0.5);
    };

    auto circle_path = [](double t, const point3& origin) {
        return point3(origin.x + 2.5 * cos(2 * pi * t), origin.y + 0.5 * sin(2 * pi * t), origin.z);
    };

    auto circle_path_offset = [](double t, const point3& origin) {
        return point3(origin.x + 2.5 * cos(2 * pi * t), origin.y + 0.5 * sin(2 * pi * t), origin.z + 0.4);
    };

    auto figure_eight = [](double t, const point3& origin) {
        double angle = 2 * pi * t;
        return point3(origin.x + 3 * sin(angle), origin.y + 1.5 * sin(angle) * cos(angle), origin.z);
    };

    auto figure_eight_offset = [](double t, const point3& origin) {
        double angle = 2 * pi * t;
        return point3(origin.x + 3 * sin(angle) - 0.3, origin.y + 1.5 * sin(angle) * cos(angle), origin.z + 0.3);
    };

    auto helix = [](double t, const point3& origin) {
        double angle = 8 * pi * t;
        return point3(origin.x + 2 * cos(angle), origin.y + 3 * t, origin.z + 2 * sin(angle));
    };

    auto helix_offset = [](double t, const point3& origin) {
        double angle = 8 * pi * t;
        return point3(origin.x + 2 * cos(angle) + 0.4, origin.y + 3 * t, origin.z + 2 * sin(angle) - 0.4);
    };

    // Floor
    auto floor_mat = make_shared<lambertian>(color(0.3, 0.3, 0.4));
    world.add(make_shared<Sphere>(point3(0, -1000.5, 0), 1000, floor_mat));

    // Moving spheres with different trajectories and materials
    auto red_glass = make_shared<dielectric>(1.5);
    auto bright_red = make_shared<emissive>(color(1.5, 0.3, 0.3));
    
    auto blue_metal = make_shared<specular>(color(0.2, 0.4, 1.0), 0.05);
    auto bright_blue = make_shared<emissive>(color(0.4, 0.7, 1.5));
    
    auto green_diffuse = make_shared<lambertian>(color(0.2, 0.9, 0.3));
    auto bright_green = make_shared<emissive>(color(0.5, 1.2, 0.5));
    
    auto gold_metal = make_shared<specular>(color(1.0, 0.84, 0.0), 0.08);
    auto bright_gold = make_shared<emissive>(color(1.5, 1.2, 0.2));

    // Sine wave: mix of refractive and emissive
    world.add(make_shared<Sphere>(point3(0, 1, -2), 0.4, sine_wave, red_glass));
    //world.add(make_shared<Sphere>(point3(0, 1, -2), 0.35, sine_wave_offset, bright_red));

    // Circle path: mix of metal and emissive
    world.add(make_shared<Sphere>(point3(-2, 2, 0), 0.35, circle_path, blue_metal));
    //world.add(make_shared<Sphere>(point3(-2, 2, 0), 0.3, circle_path_offset, bright_blue));

    // Figure eight: diffuse and emissive
    world.add(make_shared<Sphere>(point3(3, 1.5, -1), 0.3, figure_eight, green_diffuse));
    //world.add(make_shared<Sphere>(point3(3, 1.5, -1), 0.25, figure_eight_offset, bright_green));

    // Helix: metal and emissive
    world.add(make_shared<Sphere>(point3(0, 0.5, 3), 0.25, helix, gold_metal));
    //world.add(make_shared<Sphere>(point3(0, 0.5, 3), 0.2, helix_offset, bright_gold));

    // Stationary reference spheres
    world.add(make_shared<Sphere>(point3(-4, 0.3, -3), 0.3, make_shared<lambertian>(color(1, 0, 0))));
    world.add(make_shared<Sphere>(point3(4, 0.3, -3), 0.3, make_shared<lambertian>(color(0, 1, 0))));

    // Lighting
    auto light = make_shared<emissive>(color(4, 4, 4));
    world.add(make_shared<quad>(point3(-3, 5, -4), Vec3(6, 0, 0), Vec3(0, 0, 8), light));
    world.add(make_shared<Sphere>(point3(0, 4, 2), 0.6, make_shared<emissive>(color(2.5, 2, 3))));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1200;
    cam.samples_per_pixel = 200;  // More samples to capture motion blur
    cam.max_depth         = 50;
    cam.background        = color(0.05, 0.05, 0.1);

    cam.vfov     = 45;
    cam.position = point3(10, 3, 5);
    cam.direction   = point3(0, 1, 0);
    cam.up      = Vec3(0, 1, 0);

    cam.defocus_angle = 0;  // No DOF to focus on motion blur

    cam.render(world);
}


// Complex reflective/refractive scene with depth of field
void crystal_garden() {
    hittable_list world;

    auto clear_glass = make_shared<dielectric>(1.5);
    auto frosted_glass = make_shared<dielectric>(1.3);
    auto mirror = make_shared<specular>(color(0.95, 0.95, 0.95), 0.0);
    auto ground_mat = make_shared<lambertian>(color(0.4, 0.5, 0.3));
    
    // Ground
    world.add(make_shared<Sphere>(point3(0, -1000.2, 0), 1000, ground_mat));

    // Glass pyramids/spheres of varying sizes and optical properties
    world.add(make_shared<Sphere>(point3(-3, 1, -2), 0.8, clear_glass));
    world.add(make_shared<Sphere>(point3(2, 1.5, 1), 0.6, frosted_glass));
    world.add(make_shared<Sphere>(point3(0, 0.7, -4), 0.7, clear_glass));
    world.add(make_shared<Sphere>(point3(1, 0.4, 3), 0.4, mirror));
    world.add(make_shared<Sphere>(point3(-2, 0.3, 3.5), 0.3, clear_glass));

    // Colored glass spheres
    world.add(make_shared<Sphere>(point3(3, 1.2, -0.5), 0.5, make_shared<lambertian>(color(1, 0.2, 0.2))));
    world.add(make_shared<Sphere>(point3(-4, 0.5, 0), 0.5, make_shared<lambertian>(color(0.2, 1, 0.3))));
    world.add(make_shared<Sphere>(point3(-1, 0.6, -3), 0.4, make_shared<lambertian>(color(0.3, 0.5, 1))));

    // Reflective quad structures
    auto dark_mirror = make_shared<specular>(color(0.7, 0.7, 0.7), 0.05);
    world.add(make_shared<quad>(point3(-5, 0, -6), Vec3(10, 0, 0), Vec3(0, 3, 0), dark_mirror));
    world.add(make_shared<quad>(point3(-6, 0, -6), Vec3(0, 0, 12), Vec3(0, 3, 0), dark_mirror));

    // Complex lighting setup
    auto soft_light1 = make_shared<emissive>(color(3.5, 3, 2.5));
    auto soft_light2 = make_shared<emissive>(color(2.5, 2.8, 3.5));
    world.add(make_shared<Sphere>(point3(-3, 4, -1), 0.6, soft_light1));
    world.add(make_shared<Sphere>(point3(2, 3.5, 3), 0.5, soft_light2));
    world.add(make_shared<quad>(point3(-2, 4.5, -3), Vec3(4, 0, 0), Vec3(0, 0, 2), make_shared<emissive>(color(2, 2, 2))));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1000;
    cam.samples_per_pixel = 250;
    cam.max_depth         = 80;  // More bounces for refractions
    cam.background        = color(0.15, 0.15, 0.2);

    cam.vfov     = 40;
    cam.position = point3(8, 3.5, 6);
    cam.direction   = point3(-0.5, 0.3, 0);
    cam.up      = Vec3(0, 1, 0);

    cam.defocus_angle = 1.5;  // Strong depth of field
    cam.focus_dist    = 8.0;

    cam.render(world);
}


// Volumetric fog with suspended particles and complex geometry
void foggy_cathedral() {
    hittable_list world;

    auto white = make_shared<lambertian>(color(.9, .9, .9));
    auto dark = make_shared<lambertian>(color(.1, .1, .1));
    auto marble = make_shared<marble_texture>(3);
    auto marble_mat = make_shared<lambertian>(marble);
    
    // Cathedral-like structure
    world.add(make_shared<quad>(point3(0, -0.5, -10), Vec3(10, 0, 0), Vec3(0, 8, 0), white));
    world.add(make_shared<quad>(point3(-5, -0.5, -10), Vec3(0, 0, 20), Vec3(0, 8, 0), dark));
    world.add(make_shared<quad>(point3(5, -0.5, -10), Vec3(0, 0, 20), Vec3(0, 8, 0), dark));
    world.add(make_shared<quad>(point3(-5, -0.5, 10), Vec3(10, 0, 0), Vec3(0, 8, 0), white));

    // Marble columns
    world.add(make_shared<Sphere>(point3(-3, -0.3, 0), 0.4, marble_mat));
    world.add(make_shared<Sphere>(point3(3, -0.3, 0), 0.4, marble_mat));
    world.add(make_shared<Sphere>(point3(0, -0.3, -5), 0.4, marble_mat));

    // Volumetric boxes (fog)
    shared_ptr<hittable> fog_vol1 = box(point3(-4, 0, -8), point3(-1, 4, -2), white);
    shared_ptr<hittable> fog_vol2 = box(point3(1, 0, 0), point3(4, 4, 8), white);
    
    world.add(make_shared<constant_medium>(fog_vol1, 0.008, color(0.9, 0.9, 1.0)));
    world.add(make_shared<constant_medium>(fog_vol2, 0.015, color(1.0, 0.8, 0.6)));

    // Spheres floating in fog
    world.add(make_shared<Sphere>(point3(-2.5, 2, -5), 0.3, make_shared<lambertian>(color(1, 0.2, 0.2))));
    world.add(make_shared<Sphere>(point3(2.5, 2.5, 4), 0.35, make_shared<specular>(color(0.8, 0.8, 0.8), 0.1)));

    // Dramatic lighting from above
    auto volumetric_light = make_shared<emissive>(color(4, 3.5, 3));
    world.add(make_shared<quad>(point3(-4, 7.5, -8), Vec3(8, 0, 0), Vec3(0, 0, 10), volumetric_light));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 800;
    cam.samples_per_pixel = 200;
    cam.max_depth         = 50;
    cam.background        = color(0.05, 0.05, 0.1);

    cam.vfov     = 55;
    cam.position = point3(0, 2, 10);
    cam.direction   = point3(0, 1.5, 0);
    cam.up      = Vec3(0, 1, 0);

    cam.defocus_angle = 0;

    cam.render(world);
}


// Noise-based terrain with complex reflections
void noisy_landscape() {
    hittable_list world;

    auto noise_ground = make_shared<noise_texture>(2.5);
    auto marble_sky = make_shared<marble_texture>(4);
    
    // Large perlin noise terrain
    world.add(make_shared<Sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(noise_ground)));

    // Scattered reflective and refractive objects
    world.add(make_shared<Sphere>(point3(-4, 0.5, -3), 0.5, make_shared<specular>(color(1, 1, 1), 0.02)));
    world.add(make_shared<Sphere>(point3(4, 0.5, -2), 0.5, make_shared<dielectric>(1.4)));
    world.add(make_shared<Sphere>(point3(0, 0.3, 3), 0.3, make_shared<lambertian>(color(0.9, 0.3, 0.3))));
    
    // Marble sculptural elements
    world.add(make_shared<Sphere>(point3(-2, 1.5, 0), 0.8, make_shared<lambertian>(marble_sky)));
    world.add(make_shared<Sphere>(point3(2, 1.2, 1), 0.6, make_shared<lambertian>(marble_sky)));

    // Quads at various angles
    auto textured_quad = make_shared<lambertian>(noise_ground);
    world.add(make_shared<quad>(point3(-5, -0.5, -5), Vec3(10, 0, 0), Vec3(0, 0, 10), textured_quad));
    world.add(make_shared<quad>(point3(-5, -0.5, -5), Vec3(0, 3, 0), Vec3(0, 0, 10), make_shared<lambertian>(color(0.2, 0.3, 0.5))));

    // Ambient and focused lighting
    auto glow1 = make_shared<emissive>(color(2.5, 2, 1.5));
    auto glow2 = make_shared<emissive>(color(1.5, 2, 2.5));
    world.add(make_shared<Sphere>(point3(-4, 4, -2), 0.7, glow1));
    world.add(make_shared<Sphere>(point3(3, 3, 2), 0.5, glow2));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 900;
    cam.samples_per_pixel = 180;
    cam.max_depth         = 60;
    cam.background        = color(0.1, 0.12, 0.15);

    cam.vfov     = 50;
    cam.position = point3(8, 3, 8);
    cam.direction   = point3(0, 0.5, 0);
    cam.up      = Vec3(0, 1, 0);

    cam.defocus_angle = 1.2;
    cam.focus_dist    = 12.0;

    cam.render(world);
}


// Glossy metallic objects with complex refractions and reflections
void metallic_showcase() {
    hittable_list world;

    auto polished_copper = make_shared<specular>(color(0.85, 0.5, 0.15), 0.03);
    auto brushed_aluminum = make_shared<specular>(color(0.88, 0.88, 0.85), 0.08);
    auto shiny_brass = make_shared<specular>(color(0.95, 0.80, 0.20), 0.05);
    auto chrome = make_shared<specular>(color(0.98, 0.98, 0.95), 0.0);
    auto ground = make_shared<lambertian>(color(0.3, 0.35, 0.4));

    // Ground
    auto floor_checker = make_shared<checker_texture>(1.0, color(0.2, 0.2, 0.2), color(0.7, 0.7, 0.75));
    world.add(make_shared<Sphere>(point3(0, -1000.4, 0), 1000, make_shared<lambertian>(floor_checker)));

    // Metallic spheres of varying finishes
    world.add(make_shared<Sphere>(point3(-5, 1.5, -2), 1.5, polished_copper));
    world.add(make_shared<Sphere>(point3(0, 1, 0), 1.0, brushed_aluminum));
    world.add(make_shared<Sphere>(point3(4, 1.2, 1), 0.8, shiny_brass));
    world.add(make_shared<Sphere>(point3(-2, 0.6, 3), 0.6, chrome));
    world.add(make_shared<Sphere>(point3(2, 0.4, -3), 0.4, polished_copper));

    // Glass accent spheres
    world.add(make_shared<Sphere>(point3(0, 2.5, -2), 0.4, make_shared<dielectric>(1.5)));
    world.add(make_shared<Sphere>(point3(3.5, 0.5, 2.5), 0.3, make_shared<dielectric>(1.4)));

    // Reflective surfaces
    auto brushed_steel = make_shared<specular>(color(0.7, 0.72, 0.75), 0.12);
    world.add(make_shared<quad>(point3(-6, -0.3, -4), Vec3(12, 0, 0), Vec3(0, 5, 0), brushed_steel));
    world.add(make_shared<quad>(point3(-6, -0.3, 6), Vec3(12, 0, 0), Vec3(0, 5, 0), brushed_steel));

    // Lighting with multiple sources at different heights
    auto bright_light = make_shared<emissive>(color(4.5, 4, 3.5));
    auto warm_light = make_shared<emissive>(color(3, 2.5, 1.5));
    auto cool_light = make_shared<emissive>(color(1.5, 2.5, 3.5));
    
    world.add(make_shared<Sphere>(point3(-4, 5, 0), 0.6, bright_light));
    world.add(make_shared<Sphere>(point3(3, 4, 2), 0.5, warm_light));
    world.add(make_shared<Sphere>(point3(0, 3, -3), 0.4, cool_light));
    world.add(make_shared<quad>(point3(-3, 5.5, -3), Vec3(6, 0, 0), Vec3(0, 0, 3), make_shared<emissive>(color(2, 2, 2))));

    world = hittable_list(make_shared<BVH_Node>(world));

    Camera cam;
    cam.aspect_ratio      = 16.0 / 9.0;
    cam.image_width       = 1000;
    cam.samples_per_pixel = 220;
    cam.max_depth         = 70;
    cam.background        = color(0.12, 0.15, 0.18);

    cam.vfov     = 48;
    cam.position = point3(10, 4, 8);
    cam.direction   = point3(0, 0.8, 0);
    cam.up      = Vec3(0, 1, 0);

    cam.defocus_angle = 0.8;
    cam.focus_dist    = 12.0;

    cam.render(world);
}

void final_scene() {
    hittable_list boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(box(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    hittable_list world;

    world.add(make_shared<BVH_Node>(boxes1));

    auto light = make_shared<emissive>(color(7, 7, 7));
    world.add(make_shared<quad>(point3(123,554,147), Vec3(300,0,0), Vec3(0,0,265), light));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + Vec3(30,0,0);
    auto sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    world.add(make_shared<Sphere>(center1, 50, sphere_material));

    world.add(make_shared<Sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    world.add(make_shared<Sphere>(
        point3(0, 150, 145), 50, make_shared<specular>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<Sphere>(point3(360,150,145), 70, make_shared<dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<Sphere>(point3(0,0,0), 5000, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
    world.add(make_shared<Sphere>(point3(400,200,400), 100, emat));
    auto pertext = make_shared<noise_texture>(0.2);
    world.add(make_shared<Sphere>(point3(220,280,300), 80, make_shared<lambertian>(pertext)));

    hittable_list boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<Sphere>(random_vector(0,165), 10, white));
    }

    world.add(make_shared<translate>(
            make_shared<BVH_Node>(boxes2),
            Vec3(-100,270,395)
        )
    );

    Camera cam;

    cam.aspect_ratio      = 1.0;
    cam.image_width       = 400;
    cam.samples_per_pixel =  100;
    cam.max_depth         =  50;
    cam.background        = color(0,0,0);

    cam.vfov     = 40;
    cam.position = point3(478, 278, -600);
    cam.direction   = point3(278, 278, 0);
    cam.up      = Vec3(0,1,0);

    cam.defocus_angle = 0;

    cam.render(world);
}

int main()
{
    switch(5) {
        case 1: bouncing_spheres(); break;
        case 2: checkered_spheres(); break;
        case 3: earth(); break;
        case 4: triangles(); break;
        case 5: quads(); break;
        case 6: basic_lights(); break;
        case 7: cornell_box(); break;
        case 8: first_model(); break;
        case 9: perlin(); break;
        case 10: cube_map(); break;
        case 11: cornell_smoke(); break;
        case 12: marble_gallery(); break;
        case 13: motion_blur_symphony(); break;
        case 14: crystal_garden(); break;
        case 15: foggy_cathedral(); break;
        case 16: noisy_landscape(); break;
        case 17: metallic_showcase(); break;
        case 18: final_scene(); break;
    }
}
