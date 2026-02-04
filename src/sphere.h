// Defines sphere geometry.

#pragma once

#include "utility.h"
#include <functional>
#include <vector>
#include <algorithm>
#include "hittable.h"

//std::fmax() & std::fmin() are C++ standard functions

class Sphere : public hittable
{
    using PosFunc = point3 (*)(double t, const point3& origin);
    PosFunc pos_func;

    static point3 static_position(double /*t*/, const point3& origin)
    {
        return origin;
    }
    
    Vec3 cur_pos(double t) const {
        return pos_func(t, center);
    }

    public:
    // Sphere may be instantiated with a position function that determines where it's center is rendered at for an input t and t_0 = origin.
    // This could be fairly easily expanded to other objects, just add pos_func to hittable and have each child class implement the "center" translation differently
    // in the hit function.
    Sphere(const Vec3& center, double radius, shared_ptr<material> mat) : center(center), radius(std::fmax(0, radius)), pos_func(static_position), mat(mat) {
        Vec3 maxima = center + Vec3(radius, radius, radius);
        Vec3 minima = center - Vec3(radius, radius, radius);
        bbox = Bounding_Box(minima, maxima);
    }
    
    // Constructor for moving spheres
    Sphere(const Vec3& center, double radius, PosFunc pos_func, shared_ptr<material> mat)
        : center(center), radius(std::fmax(0, radius)), pos_func(pos_func), mat(mat) {
        bbox = computeBoundingBoxForMovingSphere(pos_func, center, radius, 0.0, 1.0);
    }
    //solves quadratic formula for time t and store's hit data in rec.
    //Uses b = -2h to simplify quadratic formula.
    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override
    {
        point3 current_center = cur_pos(r.time);
        Vec3 oc = current_center - r.origin;
        auto a = r.direction.length_squared();
        auto h = dot(r.direction, oc);
        auto c = oc.length_squared() - radius*radius;
        auto discriminant = h*h - a*c;

        if (discriminant < 0) {
            return false;
        }

        auto sqrtd = std::sqrt(discriminant);

        //find the nearest root in the given time range
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root)) {
                return false;
            }   
        }

        rec.t = root;
        rec.collision = r.at(rec.t);
        //div by radius normalizes
        Vec3 outward_normal = (rec.collision - current_center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        //outward_normal = p relative to the center of the sphere.
        set_uv_coords_sphere(outward_normal, rec.u, rec.v);

        return true;
    }

    //set u and v by converting the 3D cartesian point p into 2D uv coordinates on the sphere-wrapping surface.
    static void set_uv_coords_sphere(const point3& p, double& u, double& v)
    {
        auto theta = acos(-p.y);
        auto phi = atan2(-p.z, p.x) + pi;
        u = phi/(2*pi);
        v = theta/pi;
    }

    Bounding_Box bounding_box() const override { return bbox;}
    
    private:

    Vec3 center;
    double radius;
    shared_ptr<material> mat;
    Bounding_Box bbox;



    // --- Adaptive sampling helpers ---
    // These helpers compute tight per-axis minima/maxima for an arbitrary
    // position function `pos_func(t, origin)` over the time interval
    // [time0, time1]. Recursively sample the
    // midpoint and compare it to the linear interpolation of the endpoints.
    // If the midpoint deviates by more than `tolerance`, subdivide further.

    // Recursively sample one axis. `axis_eval` returns the axis coordinate
    // for a given time. The function appends sampled interior values into
    // `out_samples` when needed.
    static void sampleAxisAdaptiveRecursive(
        const std::function<double(double)>& axis_eval,
        double time_a, double time_b,
        double value_a, double value_b,
        double tolerance,
        int remaining_depth,
        std::vector<double>& out_samples)
    {
        if (remaining_depth <= 0) {
            double time_mid = 0.5 * (time_a + time_b);
            out_samples.push_back(axis_eval(time_mid));
            return;
        }

        double time_mid = 0.5 * (time_a + time_b);
        double value_mid = axis_eval(time_mid);

        // Linear prediction of the midpoint from the endpoints
        double linear_mid = 0.5 * (value_a + value_b);

        // If the actual midpoint is close enough to the linear prediction,
        // accept it and stop subdividing this interval.
        if (std::abs(value_mid - linear_mid) <= tolerance) {
            out_samples.push_back(value_mid);
            return;
        }

        // Otherwise, subdivide left and right halves.
        sampleAxisAdaptiveRecursive(axis_eval, time_a, time_mid, value_a, value_mid, tolerance, remaining_depth - 1, out_samples);
        sampleAxisAdaptiveRecursive(axis_eval, time_mid, time_b, value_mid, value_b, tolerance, remaining_depth - 1, out_samples);
    }

    // Compute numeric min/max for a single axis by adaptive sampling.
    static std::pair<double,double> computeAxisRangeAdaptive(
        const std::function<double(double)>& axis_eval,
        double time0, double time1,
        double tolerance = 1e-4,
        int max_depth = 16)
    {
        double value0 = axis_eval(time0);
        double value1 = axis_eval(time1);

        std::vector<double> samples;
        samples.reserve(32);
        // always include endpoints
        samples.push_back(value0);
        samples.push_back(value1);

        sampleAxisAdaptiveRecursive(axis_eval, time0, time1, value0, value1, tolerance, max_depth, samples);

        auto mm = std::minmax_element(samples.begin(), samples.end());
        return std::make_pair(*mm.first, *mm.second);
    }

    // Compute an axis-aligned bounding box that contains the moving sphere
    // for all times in [time0, time1]. This function evaluates each axis via
    // the adaptive sampler above and then expands the found ranges by the
    // sphere radius to get the final AABB.
    static Bounding_Box computeBoundingBoxForMovingSphere(
        PosFunc position_func,
        const point3& center_origin,
        double radius,
        double time0,
        double time1,
        double tolerance = 1e-4,
        int max_depth = 16)
    {
        // Axis evaluators using the provided position function
        auto eval_x = [&](double t){ return position_func(t, center_origin).x; };
        auto eval_y = [&](double t){ return position_func(t, center_origin).y; };
        auto eval_z = [&](double t){ return position_func(t, center_origin).z; };

        auto xr = computeAxisRangeAdaptive(eval_x, time0, time1, tolerance, max_depth);
        auto yr = computeAxisRangeAdaptive(eval_y, time0, time1, tolerance, max_depth);
        auto zr = computeAxisRangeAdaptive(eval_z, time0, time1, tolerance, max_depth);

        point3 minima(xr.first - radius, yr.first - radius, zr.first - radius);
        point3 maxima(xr.second + radius, yr.second + radius, zr.second + radius);

        return Bounding_Box(minima, maxima);
    }

};
