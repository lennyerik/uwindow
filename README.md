# μWindow
**THIS PROJECT IS A WORK IN PROGRESS!**

μWindow (or uWindow / microWindow) is a minimal (under 500 lines of C code) [Wayland](https://wayland.freedesktop.org/) [OpenGL](https://www.opengl.org/) window library.
You tell it to open a window and it gives you an OpenGL context to go and have fun with!

Think of it as a very minimal [GLFW](https://www.glfw.org/) specifically for Wayland and OpenGL.


## Compiling and Running
To build the library, just clone the repo and run make:

    git clone https://github.com/lennyerik/uwindow
    make

If you want to run any of the examples, just change into a directory within `examples` and build it the same way.
For example, if you want to compile and run the `simple` example:

    cd examples/simple
    make
    ./simple


