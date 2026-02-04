// Defines albedo (color) of an object.

#pragma once
#include "hittable.h"
#include "image.h"
#include <vector>
#include "perlin.h"

class texture {
    public:
    virtual ~texture() = default;

    virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {
    public:
    solid_color(const color& albedo) : alebdo(albedo) {}
    solid_color(double r, double g, double b) : solid_color(color(r, g, b)) {}

    color value(double u, double v, const point3& p) const override {
        return alebdo;
    }

    private:
    color alebdo;
};

class checker_texture : public texture {

    public:
    checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd)
    : inv_scale(1.0 / scale), even(even), odd(odd) {}
    checker_texture(double scale, const color& even_color, const color& odd_color)
    : checker_texture(scale, make_shared<solid_color>(even_color), make_shared<solid_color>(odd_color)) {}

    color value(double u, double v, const point3& p) const override {
        //take floor to convert to integers.
        int x = p.x > 0 ? int(inv_scale*p.x) : int(inv_scale*p.x - 1);
        int y = p.y > 0 ? int(inv_scale*p.y) : int(inv_scale*p.y - 1);
        int z = p.z > 0 ? int(inv_scale*p.z) : int(inv_scale*p.z - 1);

        int mod_sum = (x + y + z) % 2;

        return mod_sum ? odd->value(u, v, p) : even->value(u, v, p);
    }

    private:
    double inv_scale;
    shared_ptr<texture> even;
    shared_ptr<texture> odd;
};

// UV maps an image onto the object.
class image_texture : public texture {
    public:
    image_texture(const char* image_filename) : image(image_filename) {}

    color value(double u, double v, const point3& p) const override {
        return value(u, v);
    }

    color value(double u, double v) const{
        //return cyan if invalid image size
        if (image.height() <= 0 || image.width() <= 0)
        {
            return color(0, 1, 1);
        }

        //ensure input texture coordinates are in [0, 1]
        //and flip v axis for top_left -> bottom_right image reading
        u = interval(0, 1).clamp(u);
        v = 1.0 - interval(0, 1).clamp(v);

        //convert real-valued uv coordinates to discrete image texels
        int i = u * image.width();
        int j = v * image.height();
        auto color_bytes = image.pixel_data(i, j);

        auto color_scale = 1.0 / 255.0;        
        return color(color_bytes[0]*color_scale, color_bytes[1]*color_scale, color_bytes[2]*color_scale);
    }
    
    private:
    Image image;

};

class noise_texture : public texture {
    public:
    noise_texture(double frequency) : frequency(frequency) {}

    //Here we must map the [-1, 1] noise value to [0, 1], hence the addition and division.
    color value(double u, double v, const point3& p) const override {
        return color(1, 1, 1) * 0.5 * (1 + noise_generator.noise(p * frequency));
    }
    

    private:
    perlin noise_generator;
    double frequency;
};


class marble_texture : public texture {
    public:
    marble_texture(double frequency) : frequency(frequency) {}

  
    //The sin() domain is bent by noise.
    //This is referred to and domain warping, and creates
    //the folding structure in marble.
    color value(double u, double v, const point3& p) const override {
        return color(0.5, 0.5, 0.5) * (1 + sin(frequency * p.z + 10 * noise_generator.turbulence(p, 7)));
    }
    
    
    private:
    perlin noise_generator;
    double frequency;
};