/*
Title: 2D Normal Mapping
File Name: main.cpp
Copyright � 2016
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



#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include "FreeImage.h"
#include <vector>
#include "shader.h"
#include "spriteBatcher.h"
#include <iostream>

SpriteBatcher* spriteBatcher;
glm::vec2 mousePosition;
glm::vec2 viewportDimensions = glm::vec2(1280, 960);


// The texture we will be rendering to. It will match the dimensions of the screen.
Texture* screenColor;
Texture* screenNormal;
Texture* screenLighting;


// Window resize callback
void resizeCallback(GLFWwindow* window, int width, int height)
{
    viewportDimensions = glm::vec2(width, height);
	glViewport(0, 0, width, height);
    spriteBatcher->SetScreenSize(viewportDimensions);

    // Resize the fullscreen texture.
    screenColor->Resize(width, height);
    screenNormal->Resize(width, height);
    screenLighting->Resize(width, height);
}

// This will get called when the mouse moves.
void mouseMoveCallback(GLFWwindow *window, GLdouble mouseX, GLdouble mouseY)
{
    mousePosition = glm::vec2(mouseX, mouseY);
}

int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Initialize window
	GLFWwindow* window = glfwCreateWindow(
		viewportDimensions.x, viewportDimensions.y, 
		"2D Deferred Lighting", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	//set resize callback
	glfwSetFramebufferSizeCallback(window, resizeCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);

	// Initializes the glew library
	glewInit();


    // We need 3 different render textures for this, we'll create them here.
    // (This constructor for our texture class takes a width and a height, and creates an empty texture for us)
    screenColor = new Texture(viewportDimensions.x, viewportDimensions.y);
    screenNormal = new Texture(viewportDimensions.x, viewportDimensions.y);
    screenLighting = new Texture(viewportDimensions.x, viewportDimensions.y);


    // Create and bind the framebuffer for our sprite data.
    // This is the first buffer we render to, and contains the textures and normals of all the sprites
    GLuint spriteFrameBuffer;
    glGenFramebuffers(1, &spriteFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, spriteFrameBuffer);
    // Attach both the color and normal textures to the sprite frame buffer.
    // The sprite buffer will render to both of them in parallel.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColor->GetGLTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, screenNormal->GetGLTexture(), 0);


    // Create and bind the framebuffer for our light data.
    // This is the second buffer we render to. It will contain all of the lighting information.
    GLuint lightFrameBuffer;
    glGenFramebuffers(1, &lightFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, lightFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenLighting->GetGLTexture(), 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // After we render to both of these, we will combine them and render to the backbuffer.

	// Load textures from PNG
	Texture* colorPNG = new Texture((char*)"../assets/texture.png");
	Texture* normalPNG = new Texture((char*)"../assets/normal.png");

    // Create the material used to render sprites to our buffers
    ShaderProgram* spriteRenderProgram = new ShaderProgram();
    spriteRenderProgram->AttachShader(new Shader("../Assets/vertex.glsl", GL_VERTEX_SHADER));
    spriteRenderProgram->AttachShader(new Shader("../Assets/spriteFrag.glsl", GL_FRAGMENT_SHADER));
    
	// create material
	Material* spriteMat = new Material(spriteRenderProgram);
	spriteMat->IncRefCount();
    
	// set textures
	spriteMat->SetTexture((char*)"texColorPNG", colorPNG);
    spriteMat->SetTexture((char*)"texNormalPNG",normalPNG);

    // Create the material used to render lighting 
    // Note: it uses the same vertex shader, because it does the same thing in that step.
    ShaderProgram* lightRenderProgram = new ShaderProgram();
    lightRenderProgram->AttachShader(new Shader("../Assets/vertex.glsl", GL_VERTEX_SHADER));
    lightRenderProgram->AttachShader(new Shader("../Assets/lightFrag.glsl", GL_FRAGMENT_SHADER));

    Material* lightMat = new Material(lightRenderProgram);
	lightMat->IncRefCount();
	
	lightMat->SetTexture((char*)"texColorPNG", colorPNG);
    lightMat->SetTexture((char*)"texNormalScreen", screenNormal);

    // Create the material used to combine color and lighting to the backbuffer
    ShaderProgram* compositionProgram = new ShaderProgram();
    compositionProgram->AttachShader(new Shader("../Assets/fullScreenVert.glsl", GL_VERTEX_SHADER));
    compositionProgram->AttachShader(new Shader("../Assets/compositionFrag.glsl", GL_FRAGMENT_SHADER));

    Material* compositionMat = new Material(compositionProgram);
	compositionMat->IncRefCount();
    
	compositionMat->SetTexture((char*)"texColorScreen", screenColor);
    compositionMat->SetTexture((char*)"texLightScreen", screenLighting);


    // Create a spriteBatcher the same size as the screen
    spriteBatcher = new SpriteBatcher(glm::vec2(800, 600));

    float frames = 0;
    float secCounter = 0;
    float totalTime = 0;

	// Main Loop
	while (!glfwWindowShouldClose(window))
	{
        // Calculate delta time and frame rate
        float dt = glfwGetTime();
        frames++;
        secCounter += dt;
        totalTime += dt;
        if (secCounter > 1.f)
        {
            std::string title = "2D Deferred Lighting FPS: " + std::to_string(frames);
            glfwSetWindowTitle(window, title.c_str());
            secCounter = 0;
            frames = 0;
        }
        glfwSetTime(0);

        // Convert the mouse position on screen to world coordinates
        glm::vec2 direction = mousePosition;
        direction.y = viewportDimensions.y - direction.y;
        glm::vec3 direction3d = glm::vec3(direction, 5.f);


        // Select the framebuffer we want to render to.
        glBindFramebuffer(GL_FRAMEBUFFER, spriteFrameBuffer);
        GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(2, buffers);

        // Clear it.
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 0.0);

        // Enable alpha blending
        glEnable(GL_BLEND);
        // Change the blend settings
        // The first value is multiplied the the source (color being written).
        // The second value is multiplied by the destination (color already there).
        // They are then added together.
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


        // Loop and queue 48 sprites to be drawn
		// This uses SpriteMat to draw all sprites
		for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                spriteBatcher->Draw(
					glm::vec4(i * 100 , j * 100, 200, 200), 
					glm::vec4(0, 0, 1024, 1024), 
					glm::vec4(1, 1, 1, 1), 
					spriteMat);
            }
        }

        // Draw sprites
        spriteBatcher->Flush();


        // Select the framebuffer we want to render to.
        glBindFramebuffer(GL_FRAMEBUFFER, lightFrameBuffer);

        // Clear it.
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 0.0);

        // Enable additive alpha blending
        glBlendFunc(GL_ONE, GL_ONE);


        // With a forward renderer, we would have to send all the lights in an array.
        // Then for each pixel we'd have to loop over all the lights and apply them.
        // The time complexity would then be p * l, where p is the number of pixels and l is the number of lights.
        // With a resolution of 1920 by 1080, or even larger, and 20+ lights, this can become ridiculous.

        // Instead, we can render our lights similarly to the way we would render anything else.
        // Since the fragment shader only runs on the pixels within our "sprite", we can render many more lights than before.
        // Now our complexity would be sum(l(p)), where p is each pixel, and l is the number of lights on that pixel.
        // In this situation, it's important to note that we could still make everything extremely slow.
        // All you'd have to do is make each light cover the entire screen, and we're back to where we started!



        // 3 lights moving around
        // This uses lightMat to draw each light
		spriteBatcher->Draw(
            glm::vec4(200 + sin(totalTime) * 200, 100 + cos(totalTime) * 200, 400, 400),
            glm::vec4(0, 0, 1, 1),
            glm::vec4(1, 0, 0, 1), lightMat);

        spriteBatcher->Draw(
            glm::vec4(200, 100 - cos(totalTime) * 200, 400, 400),
            glm::vec4(0, 0, 1, 1),
            glm::vec4(0, 1, 0, 1), lightMat);

        spriteBatcher->Draw(
            glm::vec4(200 - sin(totalTime) * 200, 100, 400, 400),
            glm::vec4(0, 0, 1, 1),
            glm::vec4(0, 0, 1, 1), lightMat);
        
        // Uncomment this part to draw 19200 lights!
        // It might be a little slow, but it's faster than doing them one by one!
        // The slowest part here is actually just buffering all of this data.

        /*for (int i = 0; i < 160; i++)
        {
            for (int j = 0; j < 120; j++)
            {
                spriteBatcher->Draw(glm::vec4(i * 8, j * 8, 16, 16), glm::vec4(0, 0, 1, 1), glm::vec4(1), lightMat);
            }
        }*/

        // Render those lights!
        spriteBatcher->Flush();


        // Now, combine the sprite colors with the lights
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Clear it.
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0, 0.0, 0.0, 0.0);

        // Disable alpha blending (we don't need it)
        glDisable(GL_BLEND);

        // Bind the material to combine them
        compositionMat->Bind();

        // Draw three "vertices" as a triangle.
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Unbind
        compositionMat->Unbind();

		// Swap the backbuffer to the front.
		glfwSwapBuffers(window);

		// Poll input and window events.
		glfwPollEvents();

	}

    // delete all graphics memory...

    glDeleteFramebuffers(1, &spriteFrameBuffer);
    glDeleteFramebuffers(1, &lightFrameBuffer);

    spriteMat->DecRefCount();
    lightMat->DecRefCount();
    compositionMat->DecRefCount();

    delete spriteBatcher;

	// Free GLFW memory.
	glfwTerminate();


	// End of Program.
	return 0;
}
