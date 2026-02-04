// Defines constant density particle-cloud volumes

#pragma once

#include "hittable.h"
#include "texture.h"
#include "material.h"

//Currently only valid for shapes whose exiting rays will never enter again (before hitting something else).
//e.g. a taurus would not be possible.
class constant_medium : public hittable {
    public:
    constant_medium(shared_ptr<hittable> boundary, double density, shared_ptr<texture> tex) 
    : boundary(boundary), neg_inv_density(-1/density), phase_function(make_shared<isotropic>(tex)) {}

    constant_medium(shared_ptr<hittable> boundary, double density, const color& albedo) 
    : boundary(boundary), neg_inv_density(-1/density), phase_function(make_shared<isotropic>(albedo)) {}

    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override {
        hit_record enter, exit;

        //if ray never enters, return false (misses)
        if (!boundary->hit(r, interval::universe, enter))
            return false;

        //if ray never exits, return false (never finds light)
        if (!boundary->hit(r, interval(enter.t + 0.0001, infinity), exit))
            return false;  

        //contrain to time bounds
        if (enter.t < ray_t.min) enter.t = ray_t.min;
        if (exit.t > ray_t.max) exit.t = ray_t.max;

        //return false if impossible time condition
        if (enter.t >= exit.t)
            return false;

        if (enter.t < 0)
            enter.t = 0;
        
        auto ray_length = r.direction.length();
        auto distance_in_boundary = (exit.t - enter.t) * ray_length;
        //how long must you be in the boundary to hit? Random for each ray
        auto hit_distance = neg_inv_density * log(random_double());

        if (hit_distance > distance_in_boundary)
            return false;

        //if we scattered while inside
        rec.t = enter.t + (hit_distance/ray_length);
        rec.collision = r.at(rec.t);

        //Not used in further calculations, arbitrary.
        rec.normal = Vec3(0,0,1); 
        rec.front_face = true;

        rec.mat = phase_function;

        return true;
    }

    Bounding_Box bounding_box() const override {return boundary->bounding_box();}

    private:
    shared_ptr<hittable> boundary;
    double neg_inv_density;
    shared_ptr<material> phase_function;
};