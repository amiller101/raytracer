// Defines camera behavior and the rendering procedure.

#pragma once

#include "hittable.h"
#include "material.h"
#include <omp.h> 
#include "cube_map.h"

class Camera{
    public:
    double aspect_ratio = 1.0;
    int image_width = 100;
    int samples_per_pixel = 10;
    int max_depth = 10; //maximum number of ray bounces into scene
    float vfov = 90;
    point3 position = point3(0, 0, 0);
    Vec3 direction = Vec3(0, 0, -1);
    Vec3 up = Vec3(0, 1, 0);
    color background;
    Cube_Map cubemap;
    bool has_cubemap = false;

    double defocus_angle = 0; //Variation angle of rays from camera center for a single pixel
    double focus_dist = 10; //Distance from the camera position to the focus plane
    

    void render(const hittable& world){
        initialize();

        std::vector<color> frame_buffer(image_width * image_height);

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        int pixels_completed = 0;

        //maps each pixel to a ray with origin at that pixel and with a direction
        //given by the unit vector from focal_length behind
        #pragma omp parallel for collapse(2) schedule(dynamic, 1)
        for (int j = 0; j < image_height; j++) {

            for (int i = 0; i < image_width; i++) {
                color pixel_color(0,0,0);
                //for each pixel/point, render as an average of randomly chosen nearby points.
                for (int sample = 0; sample < samples_per_pixel; sample++) {
                    Ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }   
                frame_buffer[j * image_width + i] = pixel_samples_scale * pixel_color;

                #pragma omp critical
                {
                    pixels_completed++;
                    if (pixels_completed % image_width == 0) {
                        int scanlines_remaining = image_height - (pixels_completed / image_width);
                        std::clog << "\rScanlines remaining: " << scanlines_remaining << ' ' << std::flush;
                    }
                }
            }
        }


        for (int j = 0; j < image_height; j++)
        {
            for (int i = 0; i < image_width; i++)
            {
                write_color(std::cout, frame_buffer[j * image_width + i]);
            }
        }

        std::clog << "\rDone.                 \n";
    }

    void set_cubemap(const char* image_filename)
    {
        cubemap = Cube_Map(image_filename);
        has_cubemap = true;
    }


    private:
    int image_height;       //Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    
    point3 pixel00_loc;     //Location of pixel 0, 0
    Vec3 pixel_delta_u;     //Offset to pixel to the right
    Vec3 pixel_delta_v;     //Offset to pixel below
    Vec3 u, v, w;           //Cameras position basis vectors
    Vec3 defocus_disk_u;    //Defocus disk horizontal radius
    Vec3 defocus_disk_v;    //Defocus disk vertical radius

    void initialize()
    {
        image_height = int (image_width / aspect_ratio);
        //ensure height > 1.
        image_height = (image_height < 1) ? 1 : image_height;

        pixel_samples_scale = 1.0 / samples_per_pixel;

        //relative basis for camera 
        // <x, y, z> : <u, v, w>
        w = unit_vector(position - direction);
        u = unit_vector(cross(up, w));
        v = cross(w, u);      

        //viewport dimensions
        auto theta = degrees_to_radians(vfov);
        // tan(theta/2) = h/focal_dist...therefore:       
        auto viewport_height = 2 * tan(theta/2)*focus_dist;
        auto viewport_width = viewport_height * (double(image_width)/image_height);

        // Calculate the vectors across the horizontal and down the vertical viewport edges.
        auto viewport_u = u * viewport_width;
        auto viewport_v = -v * viewport_height;

        // Calculate the horizontal and vertical delta vectors from pixel to pixel.
        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        // Calculate the location of the upper left pixel.
        auto viewport_upper_left = position - (focus_dist * w) - viewport_u/2 - viewport_v/2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Calculate the camera defocus disk basis vectors
        auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;

    }

    // Construct a camera ray originating from the defocus disk at the origin
    // and directed at randomly sampled point around the pixel location i, j
    Ray get_ray(int i, int j) const{
        auto offset = sample_square();
        auto pixel_sample = pixel00_loc + ((i + offset.x) * pixel_delta_u) + ((j + offset.y) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? position: defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return Ray(ray_origin, ray_direction, ray_time);

    }

    // Returns a random point in the camera defocus disk
    point3 defocus_disk_sample() const {
        auto p = random_in_unit_disk();
        return position + (p.x * defocus_disk_u) + (p.y * defocus_disk_v);
    }

    //returns a vector to a random offset point in the [-0.5, -0.5] - [0.5, 0.5] unit square.
    Vec3 sample_square() const {
        return Vec3(random_double() - 0.5, random_double() - 0.5, 0);
    }

    color ray_color(const Ray& r, int depth, const hittable& world){
        //if exceeded bounce limit, gather no light.
        if (depth <= 0)
        {
            return color(0, 0, 0);
        }

        hit_record rec;

        //if we hit nothing, return the background or enviroment (cube map)
        if (!(world.hit(r, interval(0.001, infinity), rec)))
        {
            if (!has_cubemap) {
                return background;
            }
            return cubemap.value(r.direction);
        }

        //if we did hit something...
        color attenuation;
        Ray scattered;
        color emission = rec.mat->emitted(rec.u, rec.v, rec.collision);

        //if we scattered, follow next ray
        if (rec.mat->scatter(r, rec, attenuation, scattered))
        {
            return emission + attenuation * ray_color(scattered, depth-1, world);
        }
        //otherwise return just emission
        return emission;
    }
    
};