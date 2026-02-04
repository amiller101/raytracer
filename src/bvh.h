// Defines axis aligned bounding boxes.

#pragma once

#include "hittable.h"
#include "bounding_box.h"
#include "hittable_list.h"

#include <algorithm>

//node in BVH
class BVH_Node : public hittable {

    public:
    BVH_Node(hittable_list list) : BVH_Node(list.objects, 0, list.objects.size()) {}

    //start = first object included from objects vector
    //end = last object included from objects vector
    BVH_Node(std::vector<shared_ptr<hittable>>& objects, size_t start, size_t end) {
        //build a bounding box with span of the source objects
        bbox = Bounding_Box::empty;
        for (size_t object_index=start; object_index < end; object_index++)
            bbox = Bounding_Box(bbox, objects[object_index]->bounding_box());

        int axis = bbox.longest_axis();

    
        // Compares objects along each axis, where being smaller along an axis 
        // means your minimum value along that axis is smaller.
        // Could also implement using center of interval.
        auto comparator = (axis == 0) ? box_x_compare 
                        : (axis == 1) ? box_y_compare
                                      : box_z_compare;

        size_t object_span = end - start;

        if (object_span == 1) {
            left = right = objects[start];
        }
        else if (object_span == 2)
        {
            left = objects[start];
            right = objects[start+1];
        }//could expand to check more near base-cases
        else
        {
            std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

            auto mid = start + object_span/2;
            left = make_shared<BVH_Node>(objects, start, mid);
            right = make_shared<BVH_Node>(objects, mid, end);
        }

    }

    bool hit(const Ray& r, interval ray_t, hit_record& rec) const override {

        //check if hits this bounding box
        if (!bbox.hit(r, ray_t))
        {
            return false;
        }

        bool hit_left = left->hit(r, ray_t, rec);
        //only check times sooner than when we hit left, if we did.
        bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);

        return (hit_left || hit_right);

    }

    Bounding_Box bounding_box() const override {return bbox;}

    private:
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;
    Bounding_Box bbox;

    // returns false if a is bigger than b for chosen axis.
    // returns true if b is bigger than a for chosen axis.
    static bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index)
    {
        auto a_axis_interval = a->bounding_box().axis_interval(axis_index);
        auto b_axis_interval = b->bounding_box().axis_interval(axis_index);
        
        return (a_axis_interval.min < b_axis_interval.min);
    }

    static bool box_x_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 0);
    }

    static bool box_y_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 1);
    }

    static bool box_z_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
        return box_compare(a, b, 2);
    }

};