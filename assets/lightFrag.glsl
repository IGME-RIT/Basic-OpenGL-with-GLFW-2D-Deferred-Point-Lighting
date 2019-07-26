/*
Title: 2D Normal Mapping
File Name: fragment.glsl
Copyright ? 2016
Author: David Erbelding
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

in vec2 uv;
in vec4 color;

uniform sampler2D texColorPNG;
uniform sampler2D texNormalScreen;

void main(void)
{
	// Get the normal direction from the normal texture
	// this is a pixel from the screenshot of the last shader program
	// where all circles have the normal map texture
	vec4 normal = texelFetch(texNormalScreen, ivec2(gl_FragCoord), 0);

	// this applies the circle RGB texture to the light
	vec4 test = texture(texColorPNG, uv);
	
	// The light is a square by default, because that is
	// the shape of all 2D sprites. By getting the color
	// of a texture applied to the light, we can ignore black
	// pixels, which turns our square into a circle.
	
	// After that, we can also ignore any black pixels from
	// the screenshot of the image from the previously rendered
	// image, because there is no geometry there. This assures
	// that lighting is only calculated on pixels within the 
	// range of the light, and pixels that have geometry in the
	// first render pass
	
	if(normal.r > 0.0f && test.r > 0.0f)
	{
		// make sure it's normalized
		vec3 norm = normalize(vec3(normal * 2 - 1)) * normal.w;

		// Calculate direction of the surface to the light
		vec2 lightVector = uv * 2 - 1;
		vec3 surfaceToLight = vec3(-lightVector, .1f);
	
		// Get the diffuse value using the light and normal vectors
		float diffuse = clamp(dot(normalize(surfaceToLight), norm), 0, 1);

		// Light strength is related to how close the object is to the light
		float d = length(lightVector);

		// Attenuation uses the quadratic equation ax^2 + bx + c
		// Since we divide 1 by that formula, the value of lights will never reach 0 with distance.
		// This is a problem, since we are only rendering them on a square part of the screen.
		// To fix this problem we subtract the total by whatever the number will be when x is 1 (in this case .2)
		vec4 attenCoeff = vec4(4, 1, 0, 0.2f);
		float attenuation = clamp(1 / (d * d * attenCoeff.x + d * attenCoeff.y + attenCoeff.z) - attenCoeff.w, 0, 1);

		// Output the final light color
		gl_FragColor = color * diffuse * attenuation;
	}
	else
	{
		// if the pixel was within the light's sprite square,
		// but outside the radius of the light, or if the color
		// of the screenshot was zero (meaning there is no geometry there)
		// don't do any lighting calculations for this pixel, just return zero
		
		// If we don't do this, then we COULD go through the lighting 
		// calculations for this pixel anyways, but we would end up
		// getting zero anyways
		
		// In a 3D scene, we will use sphere objects for point lights,
		// so the shape of the light will already be a circle by default,
		// rather than being a square like this sprite example
	
		gl_FragColor = vec4(0);
	}
}