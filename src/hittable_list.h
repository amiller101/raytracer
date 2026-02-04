#pragma once
#include "utility.h"
#include "Hittable.h"
#include "bounding_box.h"
#include <vector>

// Storage method for several hittable objects in the scene. Has it's own bounding box.
class hittable_list : public hittable{
public:
    //list of abstract pointers to hittable objects
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(shared_ptr<hittable> object) {
        objects.push_back(object);
        bbox = Bounding_Box(bbox, object->bounding_box());
    }

    //returns true if hit something and stores the object that is hit first into the hit_record rec.
    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override {

        bool hit_anything = false;
        hit_record temp_rec;
        auto closest_thus_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_thus_far), temp_rec)){
                hit_anything = true;
                closest_thus_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    Bounding_Box bounding_box() const override { return bbox;}

    private:
    Bounding_Box bbox;
};
