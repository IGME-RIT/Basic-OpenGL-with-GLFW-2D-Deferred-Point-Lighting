/*
Title: 2D Normal Mapping
File Name: spriteBatcher.cpp
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

#include "spriteBatcher.h"

SpriteBatcher::SpriteBatcher(glm::vec2 screenSize)
{
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2dUVColor), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2dUVColor), (void*)sizeof(glm::vec2));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex2dUVColor), (void*)sizeof(glm::vec4));
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    SetScreenSize(screenSize);

    m_material = nullptr;
}

SpriteBatcher::~SpriteBatcher()
{
    if (m_material != nullptr)
    {
        m_material->DecRefCount();
    }
}



void SpriteBatcher::Draw(glm::vec4 destRect, glm::vec4 sourceRect, glm::vec4 color, Material* material)
{
    // Use Materials instead of textures
    if (m_material != material)
    {
        // Draw everything in the current buffer.
        Flush();
        
        
        // Make sure the material exists so long as we are holding on to it.
        material->IncRefCount();
        if (m_material != nullptr)
        {
            m_material->DecRefCount();
        }
        // Switch material
        m_material = material;
    }

    // Create vertices for the given draw request and add them to the buffer.e
    m_vertexBuffer.push_back(Vertex2dUVColor(glm::vec2(destRect.x, destRect.y), glm::vec2(sourceRect.x, sourceRect.y), color));
    m_vertexBuffer.push_back(Vertex2dUVColor(glm::vec2(destRect.x + destRect.z, destRect.y), glm::vec2(sourceRect.z, sourceRect.y), color));
    m_vertexBuffer.push_back(Vertex2dUVColor(glm::vec2(destRect.x, destRect.y + destRect.w), glm::vec2(sourceRect.x, sourceRect.w), color));
    m_vertexBuffer.push_back(Vertex2dUVColor(glm::vec2(destRect.x + destRect.z, destRect.y), glm::vec2(sourceRect.z, sourceRect.y), color));
    m_vertexBuffer.push_back(Vertex2dUVColor(glm::vec2(destRect.x, destRect.y + destRect.w), glm::vec2(sourceRect.x, sourceRect.w), color));
    m_vertexBuffer.push_back(Vertex2dUVColor(glm::vec2(destRect.x + destRect.z, destRect.y + destRect.w), glm::vec2(sourceRect.z, sourceRect.w), color));
}

void SpriteBatcher::Flush()
{
    // If there's a false alarm, don't draw anything
    // (this will always happen on the first texture)
    if (m_vertexBuffer.size() == 0 || m_material == nullptr)
    {
        return;
    }

    // Set the screen transform matrix
    m_material->SetMatrix((char*)"screenTransform", glm::mat4(m_screenTransform));
    // bind material
    m_material->Bind();

    // Copy our vertex buffer into the actual vertex buffer.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex2dUVColor) * m_vertexBuffer.size(), &m_vertexBuffer[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Draw all indices in the index buffer
    glDrawArrays(GL_TRIANGLES, 0, m_vertexBuffer.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    m_material->Unbind();

    m_vertexBuffer.clear();
}


void SpriteBatcher::SetScreenSize(glm::vec2 screenSize)
{
    // Since opengl normally renders with the screen being -1 to 1, and I want it from 0 to screen size,
    // I have to multiply everything by this scaling and translation matrix
    m_screenTransform[0][0] = 2 / screenSize.x;
    m_screenTransform[1][1] = 2 / screenSize.y;
    m_screenTransform[2][0] = -1;
    m_screenTransform[2][1] = -1;
}
