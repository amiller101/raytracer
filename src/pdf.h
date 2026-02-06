#pragma once

#include "hittable_list.h"
#include "onb.h"

class pdf {
  public:
    virtual ~pdf() {}

    virtual double value(const Vec3& direction) const = 0;
    virtual Vec3 generate() const = 0;
};

class sphere_pdf : public pdf {
  public:
    sphere_pdf() {}

    double value(const Vec3& direction) const override {
        return 1 / (4*pi);
    }

    Vec3 generate() const override {
        return random_unit_vector();
    }

};

class cosine_pdf : public pdf {
  public:
    cosine_pdf(const Vec3& normal) : uvw(normal) {}

    double value(const Vec3& direction) const override {
        auto cosine_theta =  dot(uvw.w(), direction);
        return std::fmax(0, cosine_theta / pi);
    }

    Vec3 generate() const override {
        return uvw.transform(random_cosine_direction());
    }

  private:
    onb uvw;
};

class hittable_pdf : public pdf {
  public:
    hittable_pdf(const hittable& objects, const point3& origin)
     : objects(objects), origin(origin)
    {}

    double value(const Vec3& direction) const override {
        return objects.pdf_value(origin, direction);
    }

    Vec3 generate() const override {
        return objects.random(origin);
    }

  private:
    const hittable& objects;
    point3 origin;
};

class mixture_pdf : public pdf {
  public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    double value(const Vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    Vec3 generate() const override {
        if (random_double() < 0.5)
        { return p[0]->generate(); }
        else
        { return p[1]->generate(); }
    }

  private:
    shared_ptr<pdf> p[2];
};