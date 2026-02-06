#pragma once

#include "texture.h"
#include "hittable.h"

// Defines a background for the scene based on 6 images, a plane for each axis-sign pair.
class Cube_Map {

    public:

    Cube_Map() {}

    //Must pass in a folder name in 'cube_maps' with 6 image files titled:
    //posx, posy, posz, negx, negy, negz
    Cube_Map(const char* cubemap_foldername)
    : pos_x(make_shared<image_texture>((std::string(cubemap_foldername) + "/posx.jpg").c_str())),
      pos_y(make_shared<image_texture>((std::string(cubemap_foldername) + "/posy.jpg").c_str())),
      pos_z(make_shared<image_texture>((std::string(cubemap_foldername) + "/posz.jpg").c_str())),
      neg_x(make_shared<image_texture>((std::string(cubemap_foldername) + "/negx.jpg").c_str())),
      neg_y(make_shared<image_texture>((std::string(cubemap_foldername) + "/negy.jpg").c_str())),
      neg_z(make_shared<image_texture>((std::string(cubemap_foldername) + "/negz.jpg").c_str())) {}


    color value(const Vec3& dir) const
    {
        //locally unitize
        Vec3 direction = unit_vector(dir);

        double abs_x = std::abs(direction.x);
        double abs_y = std::abs(direction.y);
        double abs_z = std::abs(direction.z);
        
        double max_axis = std::max(std::max(abs_x, abs_y), abs_z);
        double u, v;
        shared_ptr<image_texture> hit_face;

        if (cmpfloat(max_axis, abs_x)){
            if (direction.x > 0){
                u = -direction.z;
                v = direction.y;
                hit_face = pos_x;
            }
            else {
                u = direction.z;
                v = direction.y;
                hit_face = neg_x;
            }
        }
        else if (cmpfloat(max_axis, abs_y)) {
            if (direction.y > 0){
                u =  direction.x;
                v = -direction.z;
                hit_face = pos_y;
            }
            else {
                u = direction.x;
                v = direction.z;
                hit_face = neg_y;
            }
        }
        else {
            if (direction.z > 0){
                u = direction.x;
                v = direction.y;
                hit_face = pos_z;
            }
            else {
                u = -direction.x;
                v =  direction.y;
                hit_face = neg_z;
            }
        }
        
        u = 0.5*(u/max_axis + 1.0);
        v = 0.5*(v/max_axis + 1.0);
        return hit_face->value(u, v);
    }

    private:
    shared_ptr<image_texture> pos_x, neg_x, pos_y, neg_y, pos_z, neg_z;
};