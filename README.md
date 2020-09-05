# MandelbrotFractal-CPP
Mandelbrot fractal on C++

I made the C++ verion because the Python implementation is very slow. 

I had notised that continuous zoom was making the image of fractal be noisy (and I donnow how fix it).

I had tryed to use GMP but it way wery slow (or I not understand how to use it in correct way).


### Used instruments 
* mingw32-i686-8.1.0-release-posix-dwarf-rt_v6-rev0
* SFML 2.5.1
* CLion

### How to run
1. Download the appropriate SFML and compiler for this lib 
2. Import project from sources in CLion
3. Change paths in CMakeLists.txt 
4. Put into SFML root directory the cmake folder 
5. Compile & run 

### Usage
* Mouse wheel to zoom cartesian area (or LMB/RBM)
* Side mouse buttons to change zoom rectangle
* Num+ and Num- (or space/n) to change count iterations for compute the fractal 
