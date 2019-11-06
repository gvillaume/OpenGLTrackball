# OpenGLTrackball
A virtual trackball for rotating a 3D object

Implemented in OpenGL, the virtual trackball uses mouse input to track the cursor's position on the screen and rotate a 3D object when
the left mouse button is held down.

The program uses the screen coordinates of the cursor position and projects them as vectors on the trackball. Using two vectors, the
last position and current position, it calculates the rotation angle and rotation axis which are applied to the object on a forced redraw.

I also implemented the ability to translate the object left, right, up, and down using the respective arrow keys, and scale the object
using the mouse wheel.
