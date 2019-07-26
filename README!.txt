Documentation Author: Niko Procopi 2019

This tutorial was designed for Visual Studio 2017 / 2019
If the solution does not compile, retarget the solution
to a different version of the Windows SDK. If you do not
have any version of the Windows SDK, it can be installed
from the Visual Studio Installer Tool

Welcome to the 2D Deferred Rendering Tutorial for Point Lights!
Prerequesites: Render-To-Texture, 2D Point Lights (2D Sprite-Batch section)

This tutorial will be LONG.
Deferred Rendering is used in games like Grand Theft Auto V
and it one of the best (if not the #1 best) method of
optimizing a scene with multiple lights.

Line 024: Definiton of Terms
Line 046: What the program does
Line 076: How the program works 
Line 108: How to write and use shader program #1
Line 144: How to write and use shader program #2
Line 231: How to write and use shader program #3
Line 270: How to improve

[Definition of Terms]

Up until now, we've been using a method of lighting called: Forward Lighting
Forward Lighting requires one shader program; the program renders the 
geometry and colors the pixels depending on the geometry in the scene.

The problem with forward lighting is that every lighting calculation
is done on every pixel, for every light. That means, if you have 20
lights, then the lighting algorithm:
	normalize(vec3(texelFetch(norm, ivec2(uv), 0)) * 2 - 1);
	clamp(dot(normalize(surfaceToLight), normal), 0, 1);
	clamp(1 / (d * d * attenCoeff.x + d * attenCoeff.y + attenCoeff.z), 0, 1);
	(lightColor * diffuse * attenuation + ambientLight);
Will be done 20 times (once per light), on every pixel of the screen, even
if the light doesn't touch the pixel when the calculation is done. That
is a lot of wasted processing.

Deferred Rendering is an algorithm that makes it so each light is only
processed on the pixels that it touches. If there are lights in the scene
that don't touch any pixels, then those lights are automatically ignored, 
and not processed in the pixel shader.

[What the program does]

Run the OpenGL program, you'll see two rectangles in the corner,
and you'll see the final image covering the rest of the screen.
This OpenGL program has three shader (GLSL) programs, one for 
each image.

The first shader draws the scene without any lights, which 
gives us the bottom-left image, whicih shows all the RGB 
circle textures. This image is rendered to a texture.
The first shader also renders the scene with the normal map
textures. This image is also rendered to texture.

The second shader draws the small rectangle above the RGB
circle textures, which shows the color of the lighting at
each pixel. This shader takes the texture from the first shader 
that renders the scene with normal map textures. 
The key point here is that this image was created
while only processing the lighting calculations on the pixels
that each light touches. The blue light was only processed on
pixels within the radius of the blue light's center, and the
same applies for red and green, only processed on pixels within
the radius. This image was rendered to texture.

The third shader program is the one that finally sends an image
to the screen. The third shader takes the two textures (the first
two rendered images), multiplies their color values together, and
then the final scene is rendered. Additionally, the third shader
handles drawing the previous two textures to the screen in the corner.

[How it works]

When we draw something, vertices are turned into pixels through
the rasterizer. The Rasterizer determines which pixels on the screen
will have polygons, and which pixels wont. After the rasterizer, pixels
are only processed by pixel shader are only processed if the rasterizer
detects geometry on that pixel. The pixel shader does NOT need to process
every pixel on the screen, it can execute on only some pixels, or no pixels.

When we run the 2nd shader program, we pass the rendered texture that
is the entire scene rendered with the color of the normal map textures.
We then draw geometry "in the shape of the light". That means, we draw 
a circle of geometry where each light is, which will go to the rasterizer,
and then the pixel shader will only process lighting on pixels that are
in the radius of the light, because the geometry is in the same shape
and size of the light. If the geometry is too large, then extra processing
will be done on pixels that are outside the light's radius. If the geometry
is too small, then pixels within the light's radius won't have lighting
calculations done on them.

One light is processed per draw call. In one draw call, we draw one circle
of geometry, and set the color to red. This will process the lighting calculations
for the red light, only on the pixels that the red light touches, because the pixel
shader will only be activated for those pixels, because the rasterizer determined
that those pixels should be activated, because of the geometry in the vertex shader
which is in the shape of the light. Then, in seperate draw calls, we do the green
light and the blue light.

Just like any other GPU program, all draw calls are combined into a final image.
All of our lighting draw calls combine into a "light map" that shows which pixels
to be lit.

[How to write and use Shader Program #1]

Main.cpp line 115
First we load the PNG textures, which are each one circle,
one of them hold color values, the other holds normals
	Texture* colorPNG = new Texture((char*)"../assets/texture.png");
	Texture* normalPNG = new Texture((char*)"../assets/normal.png");

We have to create the first shader program, which renders the scene
with color textures, and normal textures, then takes a screenshot of
both fully-rendered scenes. We will draw 48 circles with this material.

Main.cpp line 120
On the C++ side, We load the shaders
    ShaderProgram* spriteRenderProgram = new ShaderProgram();
    spriteRenderProgram->AttachShader(new Shader("../Assets/vertex.glsl", GL_VERTEX_SHADER));
    spriteRenderProgram->AttachShader(new Shader("../Assets/spriteFrag.glsl", GL_FRAGMENT_SHADER));

We then give it the apply the program to a material, and increment the material count
	spriteMat->SetTexture((char*)"texColorPNG", colorPNG);
    spriteMat->SetTexture((char*)"texNormalPNG",normalPNG);
	
Main.cpp line 205:
We use this material to draw 48 circles in the scene

vertex.glsl
The vertex shader for this is simple, just apply a world matrix
to the geometry. Then send the position, UV, and color to the rasterizer.

spriteFrag.glsl
The fragment shader for this is also simple, but different than usual.
Rather than using gl_FragColor, we will use an array of glFragData,
which lets us render to multpile textures at once, not just one.
	gl_FragData[0] = texelFetch(texColorPNG, ivec2(uv), 0) * color;
	gl_FragData[1] = texelFetch(texNormalPNG, ivec2(uv), 0);

[How to write and use Shader Program #2]

This is the most complicated shader program of all.
This program is responsible for doing lighting calculations,
where each light is only calculated on pixels that the particular
light touches, which absolutely maximizes optimization.

We will be using this program to draw "sprites", but in this
case, each sprite represents a light. We draw a circle sprite,
becuse the light is in the shape of a circle. Earlier in this
tutorial, it was explained why this was important.

Main.cpp line 133 - 143
Load the shaders:
    ShaderProgram* lightRenderProgram = new ShaderProgram();
    lightRenderProgram->AttachShader(new Shader("../Assets/vertex.glsl", GL_VERTEX_SHADER));
    lightRenderProgram->AttachShader(new Shader("../Assets/lightFrag.glsl", GL_FRAGMENT_SHADER));
	
Make the material:
    Material* lightMat = new Material(lightRenderProgram);
	lightMat->IncRefCount();
	
Set the texture:
	lightMat->SetTexture((char*)"texColorPNG", colorPNG);
    lightMat->SetTexture((char*)"texNormalScreen", screenNormal);
	
We give the lights the PNG texture, so that we can compare the 
circle texture to the square light sprite. If the point-light itself
is a circle, then we can ignore pixels outside the circle. We use the 
color of the PNG texture to determine if a pixel is within the sprite,
but outside the circle, and then we ignore that pixel
	
We give the screenshot of the scene from the first shader program; 
this is the screenshot that was textured with normal maps. We use that
to determine the color of the light at each pixel, and intensity of 
the light at each pixel.
	
Main.cpp line: 249 - 263
These light sprites are much larger than the sprites that are drawn in the
previous material, this sprite represents the size of the each light,
the color of each light, the position, etc

vertex.glsl (same as first pass)
The vertex shader for this is simple, just apply a world matrix
to the geometry. Then send the position, UV, and color to the rasterizer.

lightFrag.glsl
we take in the UV of the light sprite, and the color of the light sprite
in vec2 uv;
in vec4 color;

We take the texture PNG, and the screenshot of the 1st pass with normal textures:
uniform sampler2D texColorPNG;
uniform sampler2D texNormalScreen;

We get the color of the PNG texture applied to the sprite,
we do NOT render the color of the PNG texture on the sprite,
we just check to see what the color WOULD be if we had
	vec4 test = texture(texColorPNG, uv);
	
If this color is black, then we have a pixel that is inside
the square-light-sprite, but outside the radius of the point light
that the sprite represents. In 3D tutorials, we will use a 
sphere, so that the shape is alreayd a circle.

We get the color of the pixel from the normal map screenshot
If this pixel is black, then there is not a pixel here to be lit,
so we ignore the pixel here too
	vec4 normal = texelFetch(texNormalScreen, ivec2(gl_FragCoord), 0);

confirm that there is a pixel here that should be lit
	if(normal.r > 0.0f && test.r > 0.0f)
	{
		...
	}
	
The code inside this if-statement is EXACTLY the same as the 
prerequesite tutorial for Forward Rendering with normal mapping,
so we don't need to explain here how it works
	
if there is nothing to be lit, don't run lighting claculations,
just set the pixel color to zero
	else
	{
		gl_FragColor = vec4(0); 
	}
	
[How to write and use Shader Program #3]
	
This shader program is resonsible for drawing the final image to the screen,
which is not only the finished scene, but also the two screenshots that
draw in teh lower-left corner of the window. We only need to draw this
once per frame, because it is the final image.

Main.cpp lines 143 - 153
Make the shader program:
    ShaderProgram* compositionProgram = new ShaderProgram();
    compositionProgram->AttachShader(new Shader("../Assets/fullScreenVert.glsl", GL_VERTEX_SHADER));
    compositionProgram->AttachShader(new Shader("../Assets/compositionFrag.glsl", GL_FRAGMENT_SHADER));

Make the material:
    Material* compositionMat = new Material(compositionProgram);
	compositionMat->IncRefCount();
	
Set the textures:
	compositionMat->SetTexture((char*)"texColorScreen", screenColor);
    compositionMat->SetTexture((char*)"texLightScreen", screenLighting);

We give it the screenshot of the scene, rendered with textures, (1st pass),
and we give it the screenshot of the lighting calculations (2nd pass)
	
Main.cpp lines 278 - 295
Render one trianlge that covers the full screen,
exactly the same as the prerequesite render-to-texture and blurry tutorals,
so it does not to be explained again here
	
fullscreenVert.glsl is exactly the same as the fullscreen veretx shader
in previous prerequesite tutorials like blurry, and render-to-texture.

compositionFrag.glsl:
Literally, in this shader, there are more lines of comments
than there are lines of codes, the comments explain everything.

Congratulations, you're done!
	
How to Improve:	
Try 3D Deferred Rendering for Point Lights