// Defines triangle, smooth triangle, and triangle mesh geometry.

#pragma once

#include "utility.h"
#include "hittable.h"
#include <vector>

class Triangle : public hittable
{
    public:
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c, shared_ptr<material> mat) : a(a), b(b), c(c), mat(mat) {
        normal = cross(b - a, c - a);

        interval interval_x = interval(std::min(std::min(a.x, b.x), c.x), std::max(std::max(a.x, b.x), c.x));
        interval interval_y = interval(std::min(std::min(a.y, b.y), c.y), std::max(std::max(a.y, b.y), c.y));
        interval interval_z = interval(std::min(std::min(a.z, b.z), c.z), std::max(std::max(a.z, b.z), c.z));

        bbox = Bounding_Box(
            interval_x,
            interval_y,
            interval_z
        );
    }

    //check if hit with plane, if yes then calculate barycentric coords and check if pos, if yes then hit at intersection with plane (store in rec)
    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override
    {
        //construct plane
        //a = point, normal = normal of plane

        //check for ray moving along the plane
        if (cmpfloat(dot(r.direction, normal), 0))
        {
            return false;
        }
        
        // find intersection with plane
        // t = (dot((plane_point - ray_origin), plane_normal) / dot(ray_direction, plane_normal))
        auto collision_time = dot(a - r.origin, normal) / dot(r.direction, normal);
        if (!ray_t.contains(collision_time))
            return false;
        auto p = r.at(collision_time);

        // Calculate barycentric coordinates
        auto alpha = dot(normal, cross(c - b, p - b)) / normal.length_squared();
        auto beta = dot(normal, cross(a - c, p - c)) / normal.length_squared();
        auto upsilon = dot(normal, cross(b - a, p - a)) / normal.length_squared();

        //if any negative barycentric coord, then misses the triangle.
        if (alpha < 0 || beta < 0 || upsilon < 0)
        {
            return false;
        }

        rec.t = collision_time;
        rec.collision = p;
        rec.set_face_normal(r, unit_vector(normal));
        rec.mat = mat;
        set_uv_coords_triangle(rec.u, rec.v, alpha, beta, upsilon);

        return true;
    }

    //set u and v by converting the 3D cartesian point into 2D uv coordinates on the triangle's surface.
    // alpha, beta, upsilon are the triangles barycentric coordinates for the point in question.
    static void set_uv_coords_triangle(double& u, double& v, const double& alpha, const double& beta, const double& upsilon)
    {
        //for the trangles vertices a, b, c
        auto a_u = 0.0; auto a_v = 0.0;
        auto b_u = 1.0; auto b_v = 1.0;
        auto c_u = 0.0; auto c_v = 1.0;


        u = alpha*a_u + beta*b_u + upsilon*c_u;
        v = alpha*a_v + beta*b_v + upsilon*c_v;
    }
    
    Bounding_Box bounding_box() const override { return bbox;}

    private:
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 normal;
    shared_ptr<material> mat;
    Bounding_Box bbox;
};

    //smooth shaded triangle with linear interpolation of normals
class Smooth_Triangle : public hittable
{
    public:
    Smooth_Triangle(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& a_n, const Vec3& b_n, const Vec3& c_n, shared_ptr<material> mat) 
    : a(a), b(b), c(c), a_n(a_n), b_n(b_n), c_n(c_n), mat(mat) {
        normal = cross(b - a, c - a);
        
        interval interval_x = interval(std::min(std::min(a.x, b.x), c.x), std::max(std::max(a.x, b.x), c.x));
        interval interval_y = interval(std::min(std::min(a.y, b.y), c.y), std::max(std::max(a.y, b.y), c.y));
        interval interval_z = interval(std::min(std::min(a.z, b.z), c.z), std::max(std::max(a.z, b.z), c.z));

        bbox = Bounding_Box(
            interval_x,
            interval_y,
            interval_z
        );
    }

    //check if hit with plane, if yes then calculate barycentric coords and check if pos, if yes then hit at intersection with plane (store in rec)
    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override
    {

        //check for ray moving along the plane
        if (cmpfloat(dot(r.direction, normal), 0))
        {
            return false;
        }
        
        // find intersection with plane
        // t = (dot((plane_point - ray_origin), plane_normal) / dot(ray_direction, plane_normal))
        auto collision_time = dot(a - r.origin, normal) / dot(r.direction, normal);
        if (!ray_t.contains(collision_time))
            return false;
        auto p = r.at(collision_time);

        // Calculate barycentric coordinates
        auto alpha = dot(normal, cross(c - b, p - b)) / normal.length_squared();
        auto beta = dot(normal, cross(a - c, p - c)) / normal.length_squared();
        auto upsilon = dot(normal, cross(b - a, p - a)) / normal.length_squared();

        //if any negative barycentric coord, then misses the triangle.
        if (alpha < 0 || beta < 0 || upsilon < 0)
        {
            return false;
        }

        //Calculate smoothed normal for this hit based on barycentric interpolation of vertex normals
        Vec3 smooth_normal = get_smooth_normal(alpha, beta, upsilon);

        rec.t = collision_time;
        rec.collision = p;
        rec.set_face_normal(r, unit_vector(smooth_normal));
        rec.mat = mat;
        set_uv_coords_triangle(rec.u, rec.v, alpha, beta, upsilon);

        return true;
    }

    //set u and v by converting the 3D cartesian point into 2D uv coordinates on the triangle's surface.
    // alpha, beta, upsilon are the triangles barycentric coordinates for the point in question.
    static void set_uv_coords_triangle(double& u, double& v, const double& alpha, const double& beta, const double& upsilon)
    {
        //for the trangles vertices a, b, c
        auto a_u = 0.0; auto a_v = 0.0;
        auto b_u = 1.0; auto b_v = 1.0;
        auto c_u = 0.0; auto c_v = 1.0;


        u = alpha*a_u + beta*b_u + upsilon*c_u;
        v = alpha*a_v + beta*b_v + upsilon*c_v;
    }

    Vec3 get_smooth_normal (const double& alpha, const double& beta, const double& upsilon) const
    {
        Vec3 smooth = alpha*a_n + beta*b_n + upsilon*c_n;
        return smooth;
    }
    
    Bounding_Box bounding_box() const override { return bbox;}

    private:
    Vec3 a;
    Vec3 b;
    Vec3 c;
    Vec3 a_n;
    Vec3 b_n;
    Vec3 c_n;
    Vec3 normal;
    shared_ptr<material> mat;
    Bounding_Box bbox;
};



inline shared_ptr<hittable_list> triangle_mesh(const point3& Q, std::vector<shared_ptr<Triangle>> triangles, shared_ptr<material> mat)
{
    auto fragments = make_shared<hittable_list>();
    return fragments;
};
