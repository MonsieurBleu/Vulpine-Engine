#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <optional>
#include <array>
#include <vector>
#include <functional>
#include <tuple>

namespace geometry {

    struct RaycastInfo {
    public:

        RaycastInfo(const glm::vec3& intersectionPoint, const glm::vec3& normal, float parameter);

        bool intersects() const;

        glm::vec3 getIntersectionPoint() const;

        glm::vec3 getNormal() const;

        float getParameter() const;

        static RaycastInfo miss();

    private:

        RaycastInfo();

        bool doesIntersect;
        glm::vec3 intersectionPoint;
        glm::vec3 normal;
        float parameter;
    };

    struct DiscreteIntersectionInfo {
    public:

        DiscreteIntersectionInfo(const glm::vec3& intersectionPoint, const glm::vec3& normal, float penetration);

        bool intersects() const;

        glm::vec3 getIntersectionPoint() const;

        glm::vec3 getNormal() const;

        float getPenetration() const;

        DiscreteIntersectionInfo getInverse() const;

        static DiscreteIntersectionInfo none();

    private:

        DiscreteIntersectionInfo();

        bool doesIntersect;
        glm::vec3 intersectionPoint;
        glm::vec3 normal;
        float penetration;

    };

    std::function<bool(const glm::vec3&, const glm::vec3&)> sweepSortingFunction(const glm::vec3& direction);

    DiscreteIntersectionInfo sat(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, const std::vector<glm::vec3>& axes);

    DiscreteIntersectionInfo gjk(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2);

    glm::vec3 arbitraryOrthogonal(const glm::vec3& direction);

    glm::vec3 convexSetSupportPoint(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction);

    std::array<glm::vec3, 3> convexSetSupportTriangle(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction);

    std::vector<glm::vec3> convexSetSupportPolygon(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction);

    std::vector<std::array<glm::vec3, 3>> polygonTriangulation(const std::vector<glm::vec3>& polygon);

    std::vector<std::array<glm::vec3, 3>> convexSetTriangulation(const std::vector<glm::vec3>& convexSet);

    std::vector<std::array<glm::vec3, 4>> convexSetTetrahedralization(const std::vector<glm::vec3>& convexSet);

    glm::vec3 convexSetAverage(const std::vector<glm::vec3>& convexSet);

    glm::vec3 weightedConvexSetAverage(const std::vector<glm::vec3>& convexSet, const std::vector<float>& pointWeights);

    //glm::vec3 uniformConvexSetBarycenter(const std::vector<glm::vec3>& convexSet);

    std::vector<glm::vec3> getBoxVertices(const glm::vec3& halfLengths);

    std::vector<glm::vec3> translatePoints(const std::vector<glm::vec3>& points, const glm::vec3& translation);

    std::vector<glm::vec3> transformPoints(const std::vector<glm::vec3>& points, const glm::vec3& translation, const glm::mat3& rotation);

    std::vector<glm::vec3> transformPoints(const std::vector<glm::vec3>& points, const glm::vec3& translation, const glm::quat& rotation);
    
    std::pair<glm::vec3, glm::vec3> capsuleHemispheres(const glm::vec3& position, const glm::quat& rotation, float halfHeight);

    std::array<float, 3> barycentricCoordinates(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    bool arePointsOnSameSide(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    // Point inside

    bool isPointInsideTetrahedron(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);

    bool isPointInsideConvex(const glm::vec3& point, const std::vector<glm::vec3>& vertices);

    // Closest point

    glm::vec3 closestPointOnLine(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, bool closed = false);

    std::pair<glm::vec3, glm::vec3> closestPointsOnLines(const glm::vec3& a1, const glm::vec3& a2, const glm::vec3& b1, const glm::vec3& b2, bool closed1 = false, bool closed2 = false);

    glm::vec3 closestPointOnTriangle(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    std::pair<glm::vec3, glm::vec3> closestPointsLineTriangle(const glm::vec3& a1, const glm::vec3& a2, const glm::vec3& b1, const glm::vec3& b2, const glm::vec3 b3, bool closed = false);

    glm::vec3 closestPointOnPolygon(const glm::vec3& point, const std::vector<glm::vec3>& points);

    glm::vec3 closestPointOnPlane(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    glm::vec3 closestPointOnTetrahedron(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);

    std::pair<glm::vec3, glm::vec3> closestPointsLineTetrahedron(const glm::vec3& a1, const glm::vec3& a2, const glm::vec3& b1, const glm::vec3& b2, const glm::vec3 b3, const glm::vec3& b4, bool closed = false);

    glm::vec3 closestPointOnConvexSet(const glm::vec3& point, const std::vector<glm::vec3>& vertices);

    std::pair<glm::vec3, glm::vec3> closestPointsLineConvexSet(const glm::vec3& a1, const glm::vec3& a2, const std::vector<glm::vec3>& vertices, bool closed = false);

    // Raycasting

    RaycastInfo raycastTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    RaycastInfo raycastSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& position, float radius);

    RaycastInfo raycastBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& halfLengths);

    RaycastInfo raycastCapsule(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& position, const glm::vec3& rotation, float radius, float halfHeight);

    RaycastInfo raycastTetrahedron(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& d);

    RaycastInfo raycastConvexSet(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const std::vector<glm::vec3>& vertices);

    // Bounding box

    using BoundingBox = std::pair<glm::vec3, glm::vec3>;

    BoundingBox sphereBoundingBox(const glm::vec3& center, float radius);

    BoundingBox capsuleBoundingBox(const glm::vec3& center1, const glm::vec3& center2, float radius);

    BoundingBox convexSetBoundingBox(const std::vector<glm::vec3>& convexSet);

    // Intersection

    DiscreteIntersectionInfo AABBAABBIntersection(const glm::vec3& position1, const glm::vec3& halfLength1, const glm::vec3& position2, const glm::vec3 halfLength2);

    DiscreteIntersectionInfo boxBoxIntersection(const glm::vec3& position1, const glm::quat& rotation1, const glm::vec3& halfLength1, const glm::vec3& position2, const glm::quat& rotation2, const glm::vec3 halfLength2);

    DiscreteIntersectionInfo boxSphereIntersection(const glm::vec3& position1, const glm::quat& rotation, const glm::vec3& halfLength, const glm::vec3& position2, float radius);

    DiscreteIntersectionInfo boxCapsuleIntersection(const glm::vec3& position1, const glm::quat& rotation1, const glm::vec3& halfLength, const glm::vec3& position2, const glm::quat& rotation2, float halfHeight, float radius);

    DiscreteIntersectionInfo pointSphereIntersection(const glm::vec3& pointPosition, const glm::vec3& center, float radius);

    DiscreteIntersectionInfo sphereSphereIntersection(const glm::vec3& center1, float radius1, const glm::vec3& center2, float radius2);

    DiscreteIntersectionInfo sphereCapsuleIntersection(const glm::vec3& position1, float radius1, const glm::vec3& position2, const glm::quat& rotation, float halfLength, float radius2);

    DiscreteIntersectionInfo sphereConvexIntersection(const glm::vec3& position1, float radius, const glm::vec3& position2, const glm::quat& rotation, const std::vector<glm::vec3> vertices);

    DiscreteIntersectionInfo capsuleConvexIntersection(const glm::vec3& position1, const glm::quat& rotation1, float halfHeight, float radius, const glm::vec3& position2, const glm::quat& rotation2, const std::vector<glm::vec3> vertices);

    DiscreteIntersectionInfo capsuleCapsuleIntersection(const glm::vec3& position1, const glm::quat& rotation1, float halfHeight1, float radius1, const glm::vec3& position2, const glm::quat& rotation2, float halfHeight2, float radius2);

    DiscreteIntersectionInfo convexConvexIntersection(const glm::vec3& position1, const glm::quat& rotation1, const std::vector<glm::vec3>& vertices1, const glm::vec3& position2, const glm::quat& rotation2, const std::vector<glm::vec3>& vertices2);

    std::vector<glm::vec3> supportsIntersection(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, const glm::vec3& supportDirection);

    // Volume

    float tetrahedronVolume(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);

    float sphereVolume(float radius);

    float boxVolume(const glm::vec3& halfLengths);

    float cylinderVolume(float radius, float halfHeight);

    float capsuleVolume(float radius, float halfHeight);

    float convexSetVolume(const std::vector<glm::vec3>& convexSet);

    // Inertia

    glm::mat3 shiftInertiaTensor(const glm::mat3& inertiaTensor, const glm::vec3& shift);

    glm::mat3 rotateInertiaTensor(const glm::mat3& inertiaTensor, const glm::quat& rotation);

    glm::mat3 tetrahedronInertiaTensor(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d);

    glm::mat3 sphereInertiaTensor(float radius);

    glm::mat3 boxInertiaTensor(const glm::vec3& halfLengths);

    glm::mat3 cylinderInertiaTensor(float radius, float halfHeight);

    glm::mat3 capsuleInertiaTensor(float radius, float halfHeight);

    glm::mat3 convexSetInertiaTensor(const std::vector<glm::vec3>& convexSet);
}