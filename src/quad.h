// Defines quad geometry

#pragma once

#include "hittable.h"

class quad: public hittable {
    public:

    quad(const point3& Q, const Vec3 u, const Vec3 v, shared_ptr<material> mat) : Q(Q), u(u), v(v), mat(mat) {
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot (n, n);
        area = n.length();

        set_bounding_box();

    }

    virtual void set_bounding_box() {
        auto bbox_diag1 = Bounding_Box(Q, Q + u + v);
        auto bbox_diag2 = Bounding_Box(Q + u, Q + v);
        bbox = Bounding_Box(bbox_diag1, bbox_diag2);
    }
    Bounding_Box bounding_box() const override {return bbox;}

    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override {
        //if ray is tangent to the bounding plane, miss
        if (std::fabs(dot(r.direction, normal)) < 1e-8)
        {
            return false;
        }

        auto t = (D - dot(normal,r.origin))/dot(normal, r.direction);

        if (!ray_t.contains(t))
        {
            return false;
        }

        auto intersection = r.at(t);

        //create quad coord system with basis vectors uv
        auto relative_point = intersection - Q;
        auto alpha = dot(w, cross(relative_point, v));
        auto beta = dot(w, cross(u, relative_point));

        if(!is_interior(alpha, beta, rec))
            return false;

        rec.collision = intersection;
        rec.t = t;
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

    //given the hit point in plane coords, return false is it is
    //outside the primitive bounded by vectors a and b.
    //otherwise set UV coords and return true.
    virtual bool is_interior(double a, double b, hit_record& rec) const {
        interval unit_interval = interval(0, 1);
        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

    double pdf_value(const point3& origin, const Vec3& direction) const override {
        hit_record rec;
        if (!this->hit(Ray(origin, direction), interval(0.001, infinity), rec))
        { return 0;}

        auto distance_squared = rec.t * rec.t * direction.length_squared();
        auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

        return distance_squared / (cosine * area);
    }

    Vec3 random(const point3& origin) const override {
        auto point = Q + (random_double() * u) + (random_double() * v);
        return point - origin;
    }

    private:
    point3 Q;
    Vec3 u;
    Vec3 v;
    Vec3 w;
    Vec3 normal;
    shared_ptr<material> mat;
    Bounding_Box bbox;
    double D;
    double area;
};

inline shared_ptr<hittable_list> box(const point3& a, const point3& b, shared_ptr<material> mat)
{
    auto sides = make_shared<hittable_list>();

    //find min/max of each coord
    Vec3 max = Vec3(MAX(a.x, b.x), MAX(a.y, b.y), MAX(a.z, b.z));
    Vec3 min = Vec3(MIN(a.x, b.x), MIN(a.y, b.y), MIN(a.z, b.z));

    //find variance of each coord
    Vec3 dx = Vec3(max.x - min.x, 0, 0);
    Vec3 dy = Vec3(0, max.y - min.y, 0);
    Vec3 dz = Vec3(0, 0, max.z - min.z);

    //create sides

    // front and back
    sides->add(make_shared<quad>(point3(min.x, min.y, min.z), dx, dy, mat));
    sides->add(make_shared<quad>(point3(min.x, min.y, max.z), dx, dy, mat));
    // left and right
    sides->add(make_shared<quad>(point3(min.x, min.y, min.z), dz, dy, mat));
    sides->add(make_shared<quad>(point3(max.x, min.y, min.z), dz, dy, mat));
    // top and bottom
    sides->add(make_shared<quad>(point3(min.x, min.y, min.z), dx, dz, mat));
    sides->add(make_shared<quad>(point3(min.x, max.y, min.z), dx, dz, mat));

    return sides;
};