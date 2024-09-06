#pragma once

#include <Common/Math/Math.h>

namespace Cosmos::Renderer
{
    struct Vertex
    {
        enum Component
        {
            POSITION = 0,
            NORMAL,
            UV,
            JOINT,
            WEIGHT,
            COLOR
        };

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::uvec4 joint;
        glm::vec4 weight;
        glm::vec4 color;

        // checks if current vertex is the same as another
        inline bool operator==(const Vertex& other) const
        {
            return position == other.position
                && normal == other.normal
                && uv == other.uv
                && joint == other.joint
                && weight == other.weight
                && color == other.color;
        }
    };
}