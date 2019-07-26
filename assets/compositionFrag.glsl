/*
Title: 2D Normal Mapping
File Name: fragment.glsl
Copyright ? 2016, 2019
Author: David Erbelding, Niko Procopi
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#version 400 core

// screenshot of 1st render pass (color)
uniform sampler2D texColorScreen;

// screenshot of 2nd render pass (lights)
uniform sampler2D texLightScreen;

void main(void)
{
	// get the size of the screenSize
	// size of colorScreen, lightScreen, and
	// the current image being rendered should be the same
	vec2 screenSize = textureSize(texColorScreen, 0);
	
	// convert it to int
	ivec2 screenSizeInt = ivec2(screenSize);
	
	// if we are in the bottom 25% of the screen
	if
	  (
		(gl_FragCoord.x < screenSize.x * 0.25) && 
		(gl_FragCoord.y < screenSize.y * 0.25)
	  )
	{
		// get the uv coordinates of this screen-space
		ivec2 uv = ivec2(gl_FragCoord); 
		
		// the range of UV will be 0 - 0.25, so multiply
		// by 4 to get 0 - 1.0, to draw full image in the 
		// corner
		uv.x *= 4;
		uv.y *= 4;
		
		// get a color from the textured circles
		gl_FragColor = texelFetch(texColorScreen, ivec2(uv), 0);
	}
		
	// if we are in the left-hand side of the screen,
	// not in the bottom corner of the screen,
	// but between 0.25 and 0.50 of the height
	else if
	  (
		(gl_FragCoord.x < screenSize.x * 0.25) && 
		(gl_FragCoord.y < screenSize.y * 0.5)
	  )
	{
		// get the uv coordinates again
		ivec2 uv = ivec2(gl_FragCoord);

		// the uv.x will range 0 to 0.25
		// the uv.y will range 0.25 to 0.50
		
		// same as before
		uv.x *= 4;
		
		// subtract 1/4th the size, 
		// to get range from 0 to 0.25
		uv.y -= screenSizeInt.y / 4;
		
		// multiply by 4 to get 0 to 1.0
		uv.y *= 4;
	
		// get the color of the screen from the light screenshot
		gl_FragColor = texelFetch(texLightScreen, ivec2(uv), 0);
	}
	
	// if we are not in a small portion of the screen
	// which is 87% of the entire screen (7 / 8)
	else
	{
		// Multiply color and light to get our final value!
		gl_FragColor = texelFetch(texColorScreen, ivec2(gl_FragCoord), 0);
		gl_FragColor *= texelFetch(texLightScreen, ivec2(gl_FragCoord), 0);
	}
}