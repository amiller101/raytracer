
# How To Use
## Build Project 
Build in VScode, everything is already setup properly with CMakeLists.txt
## Run in terminal:
    build/Raytracer.exe > results/image.ppm
    the image is saved at results/image.ppm
    Open with irfanview




# Implementation details
See 'Raytracer.pdf' for overview.

The following is a few extra notes.
## Diffuse Materials
Currently using true lambertian. This is done with random angle reflections away from the surface, weighted more towards the normal of ray incidence. This weighting is acheived by sending rays from the point of incidence to a random point on a unit sphere centered at the end of the unit-normal ray. This generally provides more physically-based results than sending the ray to a random point of a hemisphere centered at the point of ray incidence, but both techniques may be useful in varying scenarios.

Choose to always scatter light for lambertian materials, instead of only with probability of (1 - reflectance).

## Gamma Correction
We compute the gamma (or brightness) of a color linearly in RGB, but as humans we percieve color's brightness on a logarithmic scale. We "correct" the gamma of our colors before outputting them so that we percieve (127,127,127) as half as bright as (255,255,255). 



# Create Eckart Young approximation of last rendered image (Not functional yet):
    build/Eckart_Young.exe < results/image.ppm
    the image is saved at results/svd_image.ppm

To use Eckart_Young, must add include folder and Eigen library.
