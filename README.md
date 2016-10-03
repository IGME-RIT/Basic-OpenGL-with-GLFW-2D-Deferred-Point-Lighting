# Basic OpenGL with GLFW: 2D Deferred Point Lighting

Forward rendering models can become difficult to manage for larger numbers of lights.
This demo combines knowledge from the point lighting and rendering to textures examples.
A Deferred lighting system uses 3 passes to greatly increase the number of lights we can render at once:

1) Texture and normal information are rendered to two diferent textures.
2) The normal information is used to render lights to a third texture.
3) Texture and light information are combined and rendered to the backbuffer.

# Setup

You will need to have CMake installed on your computer, and properly added to your path.
In order to setup, run the following in a shell, then open the project in your preferred editor.
Windows setup has been configured for use with Visual Studio.

Windows:
```
cd path/to/folder
setup.cmd
```
Linux:
```
cd path/to/folder
./setup
```
