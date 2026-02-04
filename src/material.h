// Materials define how light bounces upon intersection with an object.

#pragma once

#include "hittable.h"
#include "texture.h"

class material {
    public:
    virtual ~material() = default;

    //input: incoming ray, hit record data including the collision normal (defines reflectance behavior)
    //output: color of the material hit, reflected ray
    virtual bool scatter(const Ray& ray_in, const hit_record& rec, color& attenuation, Ray& scattered) const {
      return false;
    }

    //If un-implemented, does not emit.
    virtual color emitted(double u, double v, const point3& p) const {
      return color(0,0,0);
    }

};

class lambertian : public material{
  public:
    lambertian(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    lambertian(shared_ptr<texture> tex) : tex(tex) {}

    //Returns true because it always reflects. 
    bool scatter(const Ray& r_in, const hit_record& rec, color& attenuation, Ray& scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        //Catch edge-case scatter direction
        if (scatter_direction.near_zero())
        {
          scatter_direction = rec.normal;
        }

        scattered = Ray(rec.collision, scatter_direction, r_in.time);
        attenuation = tex->value(rec.u, rec.v, rec.collision);
        return true;
    }

  private:
    shared_ptr<texture> tex;
};

class specular :public material {
  public:
    specular(const color& albedo, double fuzz) : tex(make_shared<solid_color>(albedo)), fuzz(fuzz < 1 ? fuzz : 1) {}
    specular(shared_ptr<texture> tex, double fuzz) : tex(tex), fuzz(fuzz < 1 ? fuzz : 1) {}


    bool scatter(const Ray& r_in, const hit_record& rec, color& attenuation, Ray& scattered) const override {
        //calculate reflection
        Vec3 reflected = reflect(r_in.direction, rec.normal);
        //add fuzziness
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = Ray(rec.collision, reflected, r_in.time);
        attenuation = tex->value(rec.u, rec.v, rec.collision);
        //ignore ray if fuzziness offest sends it through the object of original ray incidence.
        return (dot(scattered.direction, rec.normal) > 0);
    }

  private:
    shared_ptr<texture> tex;
    double fuzz;
};

class dielectric : public material {
  public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const Ray& r_in, const hit_record& rec, color& attenuation, Ray& scattered) const override {
      attenuation = color(1.0, 1.0, 1.0);
      double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

      Vec3 unit_direction = unit_vector(r_in.direction);
      //check for total internal reflection
      double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
      double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

      bool cannot_refract = ri * sin_theta > 1.0;
      Vec3 direction;

      if (cannot_refract || random_double() < reflectance(cos_theta, ri))
      {
        direction = reflect(unit_direction, rec.normal);
      }
      else
      {
        direction = refract(unit_direction, rec.normal, ri);
      }
      scattered = Ray(rec.collision, direction, r_in.time);
      return true;
    }
  private:
    // Refractive index in vacuum/air, or the ratio of the material's refractive index over refractive index of the encasing media.
    double refraction_index;
    
    //Uses Schlick's approximation for reflectance
    static double reflectance(double cos_angle_incidence, double refractive_indices_ratio) {
      auto t = (1 - refractive_indices_ratio) / (1 + refractive_indices_ratio);
      t *= t;
      return t + (1-t)*std::pow((1 - cos_angle_incidence),5);
    }
  
};


class emissive : public material {
  public:
    emissive(const color& emission) : tex(make_shared<solid_color>(emission)) {}
    emissive(shared_ptr<texture> tex) : tex(tex) {}

    //always absorbs rays. 
    color emitted(double u, double v, const point3& p) const override {
      return tex->value(u, v, p);
    }

  private:
    shared_ptr<texture> tex;
};


class isotropic : public material{
  public:
    isotropic(const color& albedo) : tex(make_shared<solid_color>(albedo)) {}
    isotropic(shared_ptr<texture> tex) : tex(tex) {}

    bool scatter(const Ray& r_in, const hit_record& rec, color& attenuation, Ray& scattered) const override {
      scattered = Ray(rec.collision, random_unit_vector(), r_in.time);
      attenuation = tex->value(rec.u, rec.v, rec.collision);
      return true;
    }

  private:
    shared_ptr<texture> tex;
};