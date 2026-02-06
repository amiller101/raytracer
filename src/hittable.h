#pragma once
#include "utility.h"
#include "bounding_box.h"

class material;

// A convenient data structure for storing ray-hittable intersection information.
class hit_record
{
    public:
    Vec3 collision;
    Vec3 normal;
    shared_ptr<material> mat;
    double t;
    bool front_face;
    double u;
    double v;


    void set_face_normal(const Ray& r, const Vec3& outward_normal)
    {
        //if positive, then vectors coincide direction and ray is leaving the object (from within)
        front_face = dot(r.direction, outward_normal) < 0;   
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// Represents an object in the scene for which we'd like to check ray intersections.
class hittable
{
    public:
    virtual ~hittable() = default;

    //with check for hits over time interval [t_min, t_max], store any hits into the hit_records data array||object
    virtual bool hit(const Ray& r, interval ray_t, hit_record& rec) const = 0;

    virtual Bounding_Box bounding_box() const = 0;

    virtual double pdf_value(const point3& origin, const Vec3& direction) const {
        return 0.0;
    }

    // A random point on the hittable
    virtual Vec3 random(const point3& origin) const {
        return Vec3(1,0,0);
    }
};

// Creates a hittable in the scene at a given translation away from the scene's origin.
class translate : public hittable {
    public:

    translate(shared_ptr<hittable> object, const Vec3& translation) : object(object), translation(translation) {
        bbox = object->bounding_box() + translation;
    }

    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override
    {
        auto offset_ray = Ray(r.origin - translation, r.direction, r.time);
    

        if(!object->hit(offset_ray, ray_t, rec))
        {
            return false;
        }

        rec.collision += translation;

        return true;
    }

    Bounding_Box bounding_box() const override { return bbox; }

    private:
    shared_ptr<hittable> object;
    Vec3 translation;
    Bounding_Box bbox;
};