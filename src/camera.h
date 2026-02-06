// Defines camera behavior and the rendering procedure.

#pragma once

#include "hittable.h"
#include "material.h"
#include <omp.h> 
#include "cube_map.h"
#include "pdf.h"

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
    

    void render(const hittable& world, const hittable& lights){
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
                for (int s_j = 0; s_j < sqrt_spp; s_j++) {
                    for (int s_i = 0; s_i < sqrt_spp; s_i++) {
                        Ray r = get_ray(i, j, s_i, s_j);
                        pixel_color += ray_color(r, max_depth, world, lights);
                    }
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
    int image_height;       // Rendered image height
    double pixel_samples_scale;  // Color scale factor for a sum of pixel samples
    int sqrt_spp; // sqrt of samples per pixel
    double recip_sqrt_spp; // 1/sqrt_spp

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

        sqrt_spp = int(std::sqrt(samples_per_pixel));
        pixel_samples_scale = 1.0 / (sqrt_spp * sqrt_spp);
        recip_sqrt_spp = 1.0 / sqrt_spp;

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
    // and directed at randomly sampled point around the pixel location i, j for stratified square with topleft corner at (s_i,s_j)
    Ray get_ray(int i, int j, int s_i, int s_j) const{
        auto offset = sample_square_stratified(s_i, s_j);
        auto pixel_sample = pixel00_loc + ((i + offset.x) * pixel_delta_u) + ((j + offset.y) * pixel_delta_v);

        auto ray_origin = (defocus_angle <= 0) ? position: defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        auto ray_time = random_double();

        return Ray(ray_origin, ray_direction, ray_time);

    }

    Vec3 sample_square_stratified(int s_i, int s_j) const {
        // Returns the vector to a random point in the square sub-pixel specified by grid
        // indices s_i and s_j, for an idealized unit square pixel [-.5,-.5] to [+.5,+.5].

        auto px = ((s_i + random_double()) * recip_sqrt_spp) - 0.5;
        auto py = ((s_j + random_double()) * recip_sqrt_spp) - 0.5;

        return Vec3(px, py, 0);
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

    color ray_color(const Ray& r, int depth, const hittable& world, const hittable& lights) const {
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
        double pdf_val;
        color emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.collision);

        //if we don't scatter, return just emission.
        if (!rec.mat->scatter(r, rec, attenuation, scattered, pdf_val))
        {
            return emission;
        }

        //if we scattered, follow next ray
        auto light_pdf = make_shared<hittable_pdf>(lights, rec.collision);
        auto surface_pdf = make_shared<cosine_pdf>(rec.normal);
        mixture_pdf mixed_pdf(light_pdf, surface_pdf);

        scattered = Ray(rec.collision, mixed_pdf.generate(), r.time);
        pdf_val = mixed_pdf.value(scattered.direction);

        double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

        color sample_color = ray_color(scattered, depth-1, world, lights);
        color scatter = (attenuation * scattering_pdf * sample_color) / pdf_val;
        return emission + scatter;
    }
    
};