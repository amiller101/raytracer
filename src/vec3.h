#pragma once

class Vec3 {
public:

    // union allows two structures to occupy the same memory, effectively creating a set of alias variables.
    union {
        //for geometry
        struct { double x, y, z; };
        //for color
        struct { double r, g, b; };
    };

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    //the following is a series of operator definitions and overloads for the Vec3 class
    //Vec3 is passed in as a const ref to Vec3 so that we don't have to create a copy of the variable each time (wastes space and time),
    //yet we enforce using it AS IF it were just a copy of Vec3 (cannot alter).

    //for read/write indexing
    double& operator[](int i)
    {
        switch (i) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw "invalid vector indexing";
        }
    }

    //for read-only indexing
    const double& operator[](int i) const
    {
        switch (i) {
        case 0: return x;
        case 1: return y;
        case 2: return z;
        default: throw "invalid vector indexing";
        }
    }
    
    //In-place Addition
    Vec3& operator+=(const Vec3 &summand) {
        this->x += summand.x;
        this->y += summand.y;
        this->z += summand.z;
        return *this;
    }

    //In-place Scalar multiplication
    Vec3& operator*=(const double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    //In-place Vector entry-wise multiplication
    Vec3& operator*=(const Vec3& v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    //In-place subtraction
    Vec3& operator-=(const Vec3& subtrahend) {
        this->x -= subtrahend.x;
        this->y -= subtrahend.y;
        this->z -= subtrahend.z;
        return *this;
    }

    //In-place scalar division
    Vec3& operator/=(double scalar) { 
        return *this *= 1 / scalar; 
    }

    //Returns true if near zero in all dimensions
    bool near_zero()
    {
        auto threshold = 1e-8;
        return ((std::fabs(x) < threshold) && (std::fabs(y) < threshold) && (std::fabs(z) < threshold));
    }

    double length() const { return std::sqrt(length_squared()); }
    double length_squared() const { return x*x + y*y + z*z; }

};

// Std output
inline std::ostream& operator<<(std::ostream &out, const Vec3 &v) {
    return out << v.x << " " << v.y << " " << v.z;
}

// Addition
inline Vec3 operator+(const Vec3& a, const Vec3& b) {
    return Vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

// Subtraction
inline Vec3 operator-(const Vec3& a, const Vec3& b) {
    return Vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// Negation
inline Vec3 operator-(const Vec3& v) {
    return Vec3(-v.x, -v.y, -v.z);
}

// Scalar multiplication (vec * scalar)
inline Vec3 operator*(const Vec3& v, double scalar) {
    return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

// Scalar multiplication (scalar * vec)
inline Vec3 operator*(double scalar, const Vec3& v) {
    return Vec3(v.x * scalar, v.y * scalar, v.z * scalar);
}

// Entry-wise multiplication (vec * vec)
inline Vec3 operator*(const Vec3& v, const Vec3& u) {
    return Vec3(v.x * u.x, v.y * u.y, v.z * u.z);
}

// Scalar division
inline Vec3 operator/(const Vec3& v, double scalar) {
    return v * (1 / scalar);
}

// Dot product
inline double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

//Cross product
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y,
                a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x);
}

// Unit vector
inline Vec3 unit_vector(const Vec3& v) {
    return v / v.length();
}

//returns random vector with canonical {in [0,1)} entries
inline Vec3 random_vector() 
{
    return Vec3(random_double(), random_double(), random_double());
}

//returns random vector in [min, max)
inline Vec3 random_vector(double min, double max) 
{
    return Vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

//returns random unit vector
inline Vec3 random_unit_vector() {
    while (true) {
        auto p = random_vector(-1,1);
        auto lensq = p.length_squared();
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}

//returns random vector in unit disk
inline Vec3 random_in_unit_disk() {
    while (true) {
        auto p = Vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() < 1)
        {
            return p;
        }
    }
}


//returns a random unit vector on the hemisphere according to the input normal
inline Vec3 random_on_hemisphere(const Vec3& normal) {
    Vec3 on_unit_sphere = random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

//returns symetric reflection of the input vector about the input normal vector
inline Vec3 reflect(const Vec3& in, const Vec3& normal) {
    return Vec3(in - (2 * normal * dot(in, normal)));
}

//returns the refracted ray resulting from the unit vector uv passing through a material with given refractive index ratio
//and with given normal of ray incidence.
//refractive_indices_ration = refractive index of original medium / '' of new medium
inline Vec3 refract(const Vec3& uv, const Vec3& normal, double refractive_indices_ratio) {
    auto cos_theta = std::fmin(dot(-uv, normal), 1.0);
    Vec3 r_out_perp = refractive_indices_ratio * (uv + cos_theta*normal);
    Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * normal;
    return r_out_perp + r_out_parallel;
}

inline Vec3 random_cosine_direction() {
    auto r1 = random_double();
    auto r2 = random_double();

    auto phi = 2*pi*r1;

    auto z = std::sqrt(1-r2);
    auto x = std::cos(phi) * std::sqrt(r2);
    auto y = std::sin(phi) * std::sqrt(r2);

    return Vec3(x, y, z);
}