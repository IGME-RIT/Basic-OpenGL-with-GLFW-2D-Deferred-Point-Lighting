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

uniform sampler2D texNormal;

void main(void)
{
	// Get the normal direction from the normal texture and make sure it's normalized
	vec4 normal = texelFetch(texNormal, ivec2(gl_FragCoord), 0);
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