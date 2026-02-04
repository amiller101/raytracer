// Random noise generated texture

#pragma once
#include "utility.h"

class perlin {

    public:

    perlin() {
        for (int i = 0; i < point_count; i++)
        {
            randvec[i] = random_unit_vector();
        }

        //set up random permutation arrays
        perlin_generate_perm(perm_x);
        perlin_generate_perm(perm_y);
        perlin_generate_perm(perm_z);
    }

    //returns a omni-directional noise value to apply to the point p.
    double noise(const point3& p) const {
        //fractional coordinates
        auto u = p.x - floor(p.x);
        auto v = p.y - floor(p.y);
        auto w = p.z - floor(p.z);

        //minimum corner of the cube spanned by the fractional coordinates
        int i = floor(p.x);
        int j = floor(p.y);
        int k = floor(p.z);

        Vec3 corners[2][2][2];

        //Set random value for each (8) corner of the unit cube containing p
        for (int x_corner = 0; x_corner < 2; x_corner++)
        {
            for (int y_corner = 0; y_corner < 2; y_corner++)
            {
                for (int z_corner = 0; z_corner < 2; z_corner++) 
                {
                    corners[x_corner][y_corner][z_corner] = 
                    randvec [
                        perm_x[(i + x_corner) & 255] ^ 
                        perm_y[(j + y_corner) & 255] ^ 
                        perm_z[(k + z_corner) & 255]
                    ]; 
                }
            }
        }
        
        //return: 
        //combination of set dimension permutations for 3D deterministic randomness
        return trillinear_interpolation(corners, u, v , w);
    }

    double turbulence(const point3& p, int depth) const {
        auto sum = 0.0;
        auto temp_p = p;
        auto weight = 1.0;

        for (int i = 0; i < depth; i++) {
            //add this octave
            sum += weight * noise(temp_p);
            //halve amplitude
            weight *= 0.5;
            //double frequency
            temp_p *= 2;
        }
        return fabs(sum);
    }

    private:
    static const int point_count = 256;
    Vec3 randvec[point_count];
    int perm_x[point_count];
    int perm_y[point_count];
    int perm_z[point_count];

    //creates a random permutation array of values [0,255]
    static void perlin_generate_perm(int* p) {
        for (int i = 0; i < point_count; i++)
        {
            p[i] = i;
        }

        permute(p, point_count);
    }

    //randomly swaps index values in the integer array p of size 'size'.
    static void permute(int* p, int size)
    {
        for (int i = size - 1; i > 0; i--)
        {
            int target = random_int(0, i);
            int tmp = p[i];
            p[i] = p[target];
            p[target] = tmp;
        }
    }

    //return weighted average of the corner values of the unit cube defined by corners,
    // at the point <u, v, w> in [0, 1] within the cube.
    static double trillinear_interpolation(Vec3 corners[2][2][2], double u, double v, double w)
    {
        //round off the interpolation with a Hermite cubic (c spline)
        u = u*u*(3-2*u);
        v = v*v*(3-2*v);
        w = w*w*(3-2*w);

        auto sum = 0.0;

        // For each corner dimension weighted according to u, v, or w,
        // Set the same dimension in the opposite corner equal to  1 - (u, v, or w).
        // Do all of our calculations in the unit cubic,
        // then perform a weighted projection onto each of the corner vectors.
        for (int corner_x = 0; corner_x < 2; corner_x++) {
            for (int corner_y = 0; corner_y < 2; corner_y++) {
                for (int corner_z = 0; corner_z < 2; corner_z++) { 
                    Vec3 weight_v(u - corner_x, v - corner_y, w - corner_z);
                    sum += (corner_x*u + (1-corner_x)*(1-u))
                        * (corner_y*v + (1-corner_y)*(1-v))
                        * (corner_z*w + (1-corner_z)*(1-w))
                        * dot(corners[corner_x][corner_y][corner_z], weight_v);
                }
            }
        }
        return sum;
    }
};