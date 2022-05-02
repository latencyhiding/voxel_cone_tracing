# Voxel Cone Tracing

An implementation of voxel cone tracing based on "Interactive Indirect Illumination Using Voxel Cone Tracing" by Cyril Crassin et al (https://research.nvidia.com/sites/default/files/publications/GIVoxels-pg2011-authors.pdf).

![alt text](https://github.com/GreatBlambo/voxel_cone_tracing/blob/master/tckr9Pn.png)

Requires OpenGL 4.5, CMake 3.2

After cloning, do:
```
git submodule update --init --recursive
```
To pull each submodule.

To build:
``` 
mkdir build
cd build && cmake ..
make
cd ..
build/Voxel_Cone_Tracing
```
