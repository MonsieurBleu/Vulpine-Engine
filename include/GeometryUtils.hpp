#pragma once

#include <glm/glm.hpp>
#include <optional>
#include <array>
#include <vector>

struct IntersectionInfo {
    glm::vec3 collisionPoint;
    glm::vec3 normal;
    union {
        float penetration;
        float t;
    };

    IntersectionInfo& invert();
};

std::optional<IntersectionInfo> getPointSphereIntersection(const glm::vec3& center1, const glm::vec3& center2, float radius);

std::optional<IntersectionInfo> getSpheresIntersection(const glm::vec3& center1, float radius1, const glm::vec3& center2, float radius2);

std::optional<IntersectionInfo> sat(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, const std::vector<glm::vec3>& axes);

std::optional<IntersectionInfo> gjk(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2);

glm::vec3 closestPointOnTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

namespace geometry {

    struct RaycastInfo {
    public:

        RaycastInfo(const glm::vec3& intersectionPoint, const glm::vec3& normal, float timePoint);

        bool intersects() const;

        glm::vec3 getIntersectionPoint() const;

        glm::vec3 getNormal() const;

        float getTimePoint() const;

        static RaycastInfo none();

    private:

        RaycastInfo();

        bool doesIntersect;
        glm::vec3 intersectionPoint;
        glm::vec3 normal;
        float timePoint;
    };

    RaycastInfo raycastSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& spherePosition, float sphereRadius);

    glm::vec3 convexSetSupportPoint(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction);

    std::vector<std::array<glm::vec3, 4>> convexSetTetrahedralization(const std::vector<glm::vec3>& convexSet);

    glm::vec3 uniformConvexSetBarycenter(const std::vector<glm::vec3>& convexSet);

    glm::vec3 weightedConvexSetBarycenter(const std::vector<glm::vec3>& convexSet, const std::vector<float>& pointWeights);

    float tetrahedronVolume(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);

    float convexSetVolume(const std::vector<glm::vec3>& convexSet);

    std::vector<glm::vec3> getBoxVertices(const glm::vec3& halfLengths, const glm::quat& rotation);

    std::vector<glm::vec3> translatePoints(const std::vector<glm::vec3>& points, const glm::vec3& translation);

    std::pair<glm::vec3, glm::vec3> convexSetBoundingBox(const std::vector<glm::vec3>& convexSet);

}