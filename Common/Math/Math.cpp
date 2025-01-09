#include "Math.h"

namespace Cosmos
{
    bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
    {
        glm::mat4 localMatrix(transform);
        using T = float;

        // normalize the matrix
        if (glm::epsilonEqual(localMatrix[3][3], static_cast<T>(0), glm::epsilon<T>()))
            return false;

        // handle perspective
        bool row0 = glm::epsilonNotEqual(localMatrix[0][3], 0.0f, glm::epsilon<T>());
        bool row1 = glm::epsilonNotEqual(localMatrix[1][3], 0.0f, glm::epsilon<T>());
        bool row2 = glm::epsilonNotEqual(localMatrix[2][3], 0.0f, glm::epsilon<T>());

        if (row0 || row1 || row2)
        {
            localMatrix[0][3] = localMatrix[1][3] = localMatrix[2][3] = static_cast<T>(0);
            localMatrix[3][3] = static_cast<T>(1);
        }

        // handle translation
        translation = glm::vec3(localMatrix[3]);
        localMatrix[3] = glm::vec4(0, 0, 0, localMatrix[3].w);

        // handle scale
        glm::vec3 row[3];
        for (glm::length_t i = 0; i < 3; ++i)
            for (glm::length_t j = 0; j < 3; ++j)
                row[i][j] = localMatrix[i][j];

        // compute scale factor and normalize rows
        scale.x = glm::length(row[0]);
        row[0] = glm::detail::scale(row[0], static_cast<T>(1));
        scale.y = glm::length(row[1]);
        row[1] = glm::detail::scale(row[1], static_cast<T>(1));
        scale.z = glm::length(row[2]);
        row[2] = glm::detail::scale(row[2], static_cast<T>(1));

        // at this point, the matrix (in rows[]) is orthonormal.
        // check for a coordinate system flip. If the determinant is -1, then negate the matrix and the scaling factors.
#if 0
        glm::vec3 Pdum3 = glm::cross(row[1], row[2]); // v3Cross(row[1], row[2], Pdum3);
        if (glm::dot(row[0], Pdum3) < 0)
        {
            for (glm::length_t i = 0; i < 3; i++)
            {
                scale[i] *= static_cast<T>(-1);
                row[i] *= static_cast<T>(-1);
            }
        }
#endif
        rotation.y = asin(-row[0][2]);

        if (cos(rotation.y) != 0)
        {
            rotation.x = atan2(row[1][2], row[2][2]);
            rotation.z = atan2(row[0][1], row[0][0]);
        }

        else
        {
            rotation.x = atan2(-row[2][0], row[1][1]);
            rotation.z = 0;
        }

        return true;
    }

    float GetAverageRadius(const glm::mat4& transform, const glm::vec3& scale)
    {
        float scale_x = transform[0][0];
        float scale_y = transform[1][1];
        float scale_z = transform[2][2];

        // calculate average scale (for uniform scaling approximation)
        float average_scale = (scale_x + scale_y + scale_z) / 3.0f;

        // calculate maximum dimension of the original model
        float max_dimension = std::max(std::max(scale.x, scale.y), scale.z);

        // approximate radius of the scaled model
        return average_scale * max_dimension;
    }

    void RayCast(glm::vec2 mousePos, float winWidth, float winHeight, const glm::mat4& projection, const glm::mat4& view, glm::vec3& origin, glm::vec3& dir)
    {
        // map to Normalized Device Coordinates (NDC)
        glm::vec2 ndc;
        ndc.x = (2.0f * mousePos.x) / winWidth - 1.0f;
        ndc.y = 1.0f - (2.0f * mousePos.y) / winHeight; // vulkan flips Y
        ndc.y *= -1.0f;
    
        // create clip space coordinates
        glm::vec4 clipCoords(ndc.x, ndc.y, -1.0f, 1.0f);
    
        // transform to view space
        glm::vec4 viewCoords = glm::inverse(projection) * clipCoords;
        viewCoords.z = -1.0f; // forward direction
        viewCoords.w = 0.0f;
    
        // transform to world space
        glm::vec4 worldCoords = glm::inverse(view) * viewCoords;
    
        // normalize the direction
        dir = glm::normalize(glm::vec3(worldCoords));
    
        // get ray origin
        origin = glm::vec3(glm::inverse(view)[3]); // camera position
    }

    void RayCastViewport(glm::vec2 mousePos, glm::vec2 viewportSize, glm::vec2 viewportOffset, float winWidth, float winHeight, const glm::mat4& projection, const glm::mat4& view, glm::vec3& origin, glm::vec3& dir)
    {
        // calculate relative mouse position in the viewport
        glm::vec2 relativeMousePos;
        relativeMousePos.x = mousePos.x - viewportOffset.x;
        relativeMousePos.y = mousePos.y - viewportOffset.y;

        // check if the mouse is inside the viewport
        if (relativeMousePos.x < 0 || relativeMousePos.x > viewportSize.x ||
            relativeMousePos.y < 0 || relativeMousePos.y > viewportSize.y) {
            origin = glm::vec3(0.0f, 0.0f, 0.0f);
            dir = glm::vec3(0.0f, 0.0f, 0.0f);
            return;
        }

        // map to Normalized Device Coordinates (NDC)
        glm::vec2 ndc;
        ndc.x = (2.0f * relativeMousePos.x) / viewportSize.x - 1.0f;
        ndc.y = 1.0f - (2.0f * relativeMousePos.y) / viewportSize.y; // Flip Y
        ndc.y *= -1.0f; // Vulkan's inverted Y NDC

        // clip space coordinates
        glm::vec4 clipCoords(ndc.x, ndc.y, -1.0f, 1.0f);

        // transform to view space
        glm::vec4 viewCoords = glm::inverse(projection) * clipCoords;
        viewCoords.z = -1.0f; // forward direction
        viewCoords.w = 0.0f;

        // transform to world space
        glm::vec4 worldCoords = glm::inverse(view) * viewCoords;

        // normalize the direction
        dir = glm::normalize(glm::vec3(worldCoords));

        // get ray origin
        origin = glm::vec3(glm::inverse(view)[3]); // camera position
    }

    bool RayCollide(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& aabbMin, const glm::vec3& aabbMax, const glm::mat4& modelMatrix, float& distance)
    {
        float tMin = 0.0f;
        float tMax = 100000.0f;

        glm::vec3 OBBposition_worldspace(modelMatrix[3].x, modelMatrix[3].y, modelMatrix[3].z);

        glm::vec3 delta = OBBposition_worldspace - origin;

        // Test intersection with the 2 planes perpendicular to the OBB's X axis
        {
            glm::vec3 xaxis(modelMatrix[0].x, modelMatrix[0].y, modelMatrix[0].z);
            float e = glm::dot(xaxis, delta);
            float f = glm::dot(direction, xaxis);

            if (fabs(f) > 0.001f) { // Standard case

                float t1 = (e + aabbMin.x) / f; // Intersection with the "left" plane
                float t2 = (e + aabbMax.x) / f; // Intersection with the "right" plane
                // t1 and t2 now contain distances betwen ray origin and ray-plane intersections

                // We want t1 to represent the nearest intersection, 
                // so if it's not the case, invert t1 and t2
                if (t1 > t2) {
                    float w = t1; t1 = t2; t2 = w; // swap t1 and t2
                }

                // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
                if (t2 < tMax)
                    tMax = t2;
                // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
                if (t1 > tMin)
                    tMin = t1;

                // And here's the trick :
                // If "far" is closer than "near", then there is NO intersection.
                // See the images in the tutorials for the visual explanation.
                if (tMax < tMin)
                    return false;

            }
            else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
                if (-e + aabbMin.x > 0.0f || -e + aabbMax.x < 0.0f)
                    return false;
            }
        }


        // Test intersection with the 2 planes perpendicular to the OBB's Y axis
        // Exactly the same thing than above.
        {
            glm::vec3 yaxis(modelMatrix[1].x, modelMatrix[1].y, modelMatrix[1].z);
            float e = glm::dot(yaxis, delta);
            float f = glm::dot(direction, yaxis);

            if (fabs(f) > 0.001f) {

                float t1 = (e + aabbMin.y) / f;
                float t2 = (e + aabbMax.y) / f;

                if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

                if (t2 < tMax)
                    tMax = t2;
                if (t1 > tMin)
                    tMin = t1;
                if (tMin > tMax)
                    return false;

            }
            else {
                if (-e + aabbMin.y > 0.0f || -e + aabbMax.y < 0.0f)
                    return false;
            }
        }


        // Test intersection with the 2 planes perpendicular to the OBB's Z axis
        // Exactly the same thing than above.
        {
            glm::vec3 zaxis(modelMatrix[2].x, modelMatrix[2].y, modelMatrix[2].z);
            float e = glm::dot(zaxis, delta);
            float f = glm::dot(direction, zaxis);

            if (fabs(f) > 0.001f) {

                float t1 = (e + aabbMin.z) / f;
                float t2 = (e + aabbMax.z) / f;

                if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

                if (t2 < tMax)
                    tMax = t2;
                if (t1 > tMin)
                    tMin = t1;
                if (tMin > tMax)
                    return false;

            }
            else {
                if (-e + aabbMin.z > 0.0f || -e + aabbMax.z < 0.0f)
                    return false;
            }
        }

        distance = tMin;
        return true;
    }

    bool RayCollide2(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& objectPos, float maxCollisionDistance)
    {
        // ray equation : P(t) = start + t * ray
        glm::vec3 diff = glm::vec3(objectPos.x - origin.x, objectPos.y - origin.y, objectPos.z - origin.z);
        float t = (diff.x * direction.x + diff.y * direction.y + diff.z * direction.z) / (direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

        // clamp t to ensure the ray only extends in the forward direction
        if (t < 0) t = 0;

        // closest point on the ray to the object
        glm::vec3 closestPoint = glm::vec3(origin.x + t * direction.x, origin.y + t * direction.y, origin.z + t * direction.z);

        // check if the closest point is within the collision radius
        return glm::distance(closestPoint, objectPos) <= maxCollisionDistance;
    }

    bool RayCollide3(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& objectPos, float maxCollisionDistance)
    {
        // line segment equation: P(t) = p1 + t * (p2 - p1), where 0 <= t <= 1
        glm::vec3 line = glm::vec3(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
        glm::vec3 diff = glm::vec3(objectPos.x - p0.x, objectPos.y - p0.y, objectPos.z - p0.z);
        float t = (diff.x * line.x + diff.y * line.y + diff.z * line.z) / (line.x * line.x + line.y * line.y + line.z * line.z);

        // clamp t to ensure it's within the line segment range
        if (t < 0) t = 0;
        if (t > 1) t = 1;

        // closest point on the line segment to the object
        glm::vec3 closestPoint = glm::vec3(p0.x + t * line.x, p0.y + t * line.y, p0.z + t * line.z);

        // check if the closest point is within the collision radius
        return glm::distance(closestPoint, objectPos) <= maxCollisionDistance;
    }

    bool RaySphereCollide(const glm::vec3& origin, const glm::vec3& direction, const glm::vec3& sphereCenter, float sphereRadius, float& t)
    {
        // ray-sphere intersection formula:
        // solve for t in the quadratic equation: t^2 * (d.d) + 2t * (o-c).d + (o-c).(o-c) - r^2 = 0

        glm::vec3 oc = origin - sphereCenter;       // vector from ray origin to sphere center
        float a = glm::dot(direction, direction);   // coefficient of t^2
        float b = 2.0f * glm::dot(oc, direction);   // coefficient of t
        float c = glm::dot(oc, oc) - sphereRadius * sphereRadius; // Constant term
        float discriminant = b * b - 4 * a * c;     // discriminant of the quadratic equation
        
        // no real roots, ray does not intersect the sphere
        if (discriminant < 0) return false;

        float t1 = (-b - std::sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b + std::sqrt(discriminant)) / (2.0f * a);

        // we need the closest positive intersection
        if (t1 >= 0.0f) {
            t = t1;
            return true;
        }

        else if (t2 >= 0.0f) {
            t = t2;
            return true;
        }

        // both intersections are negative, sphere is behind the ray
        return false;
    }

    bool RayAABBCollide(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& aabbMin, const glm::vec3& aabbMax, float& tMin)
    {
        glm::vec3 invDir = 1.0f / rayDirection; // Calculate the inverse of the ray direction
        glm::vec3 t0 = (aabbMin - rayOrigin) * invDir;
        glm::vec3 t1 = (aabbMax - rayOrigin) * invDir;

        // Ensure t0 <= t1 by swapping as needed
        glm::vec3 tMinVec = glm::min(t0, t1);
        glm::vec3 tMaxVec = glm::max(t0, t1);

        // Find the largest tMin and smallest tMax
        float tEnter = std::max(std::max(tMinVec.x, tMinVec.y), tMinVec.z);
        float tExit = std::min(std::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

        // If tEnter > tExit, the ray misses the AABB
        if (tEnter > tExit || tExit < 0.0f) {
            return false;
        }

        // Return the closest intersection distance
        tMin = tEnter > 0.0f ? tEnter : tExit;
        return true;

        return false;
    }
}