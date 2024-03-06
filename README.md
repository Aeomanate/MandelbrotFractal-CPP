# MandelbrotFractal-CPP
Mandelbrot fractal calculations on C++

### Used tools 
* MSYS-MinGW-w64 v12
* SFML 2.6.1
* GMP 6.3.0-1

### How to run
1. Pull the project
2. Install a GMP into your compiler 
3. Compile & run 

### Usage
* Mouse wheel to zoom cartesian_borders area (or LMB/RBM)
* Side mouse buttons to change zoom rectangle
* Num+ and Num- (or space/n) to change count iterations for compute the fractal_image 

### ToDo
* Continuous zoom is making the image noisy. 
It can be solved by introduce a higher precision calculations with GMP.
  * The GMP very slow out-of-box.
