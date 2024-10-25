#include "GeometryUtils.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <deque>
#include <stdexcept>
#include <set>

#include <Graphics/Mesh.hpp>
#include <Graphics/Scene.hpp>
extern Mesh* globalEarthMesh;
extern Scene* scenePointer;

namespace geometry {

    RaycastInfo::RaycastInfo(
        const glm::vec3& intersectionPoint,
        const glm::vec3& normal,
        float parameter
    ) :
        doesIntersect(true),
        intersectionPoint(intersectionPoint),
        normal(normal),
        parameter(parameter)
    {}

    RaycastInfo::RaycastInfo() :
        doesIntersect(false)
    {}

    bool RaycastInfo::intersects() const {
        return doesIntersect;
    }

    glm::vec3 RaycastInfo::getIntersectionPoint() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the intersection point of a non-intersecting ray cast");
        }
        return intersectionPoint;
    }

    glm::vec3 RaycastInfo::getNormal() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the normal direction of a non-intersecting ray cast");
        }
        return normal;
    }

    float RaycastInfo::getParameter() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the parameter t of a non-intersecting ray cast");
        }
        return parameter;
    }

    RaycastInfo RaycastInfo::miss() {
        return RaycastInfo();
    }

    DiscreteIntersectionInfo::DiscreteIntersectionInfo(const glm::vec3& intersectionPoint, const glm::vec3& normal, float penetration) :
        doesIntersect(true),
        intersectionPoint(intersectionPoint),
        normal(normal),
        penetration(penetration)
    {}

    DiscreteIntersectionInfo::DiscreteIntersectionInfo() :
        doesIntersect(false)
    {}

    bool DiscreteIntersectionInfo::intersects() const {
        return doesIntersect;
    }

    glm::vec3 DiscreteIntersectionInfo::getIntersectionPoint() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the intersection point of a non-intersecting test");
        }
        return intersectionPoint;
    }

    glm::vec3 DiscreteIntersectionInfo::getNormal() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the normal direction of a non-intersecting test");
        }
        return normal;
    }

    float DiscreteIntersectionInfo::getPenetration() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the penetration distance of a non-intersecting test");
        }
        return penetration;
    }

    DiscreteIntersectionInfo DiscreteIntersectionInfo::getInverse() const {
        return doesIntersect ? DiscreteIntersectionInfo(intersectionPoint, normal, penetration) : DiscreteIntersectionInfo();
    }

    DiscreteIntersectionInfo DiscreteIntersectionInfo::none() {
        return DiscreteIntersectionInfo();
    }
    
    DiscreteIntersectionInfo sat(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, const std::vector<glm::vec3>& axes) {
        glm::vec3 normal;
        float penetration = std::numeric_limits<float>::infinity();

        for (const glm::vec3& axis : axes) {
            float min1 = std::numeric_limits<float>::infinity();
            float max1 = -std::numeric_limits<float>::infinity();

            for (const glm::vec3& point : points1) {
                float projection = glm::dot(axis, point);
                if (projection < min1) {
                    min1 = projection;
                }
                else if (projection > max1) {
                    max1 = projection;
                }
            }

            float min2 = std::numeric_limits<float>::infinity();
            float max2 = -std::numeric_limits<float>::infinity();

            for (const glm::vec3& point : points2) {
                float projection = glm::dot(axis, point);
                if (projection < min2) {
                    min2 = projection;
                }
                else if (projection > max2) {
                    max2 = projection;
                }
            }

            float overlap = std::min(max1, max2) - std::max(min1, min2);
            if (overlap < 0.0f) {
                return DiscreteIntersectionInfo::none();
            } else if (overlap < penetration) {
                penetration = overlap;
                normal = min1 + max1 > min2 + max2 ? axis : -axis;
            }
        }

        return DiscreteIntersectionInfo(
            glm::vec3(0.0f, 0.0f, 0.0f),//translatePoints(intersectingConvexPolygon(points1, points2, -normal), -normal * 0.5f * penetration),
            normal,
            penetration
        );
    }

    std::function<bool(const glm::vec3&, const glm::vec3&)> sweepSortingFunction(const glm::vec3& direction) {
        return [&direction](const glm::vec3& a, const glm::vec3& b) {
            return glm::dot(direction, a) < glm::dot(direction, b);
        };
    }

    static bool gjkNearestSimplexLine(std::deque<std::array<glm::vec3, 3>>& simplex, glm::vec3& direction) {
        std::array<glm::vec3, 3> sa = simplex[0];
        std::array<glm::vec3, 3> sb = simplex[1];
        glm::vec3 a = sa[0];
        glm::vec3 b = sb[0];
        glm::vec3 ab = b - a;
        glm::vec3 ao = -a;

        if (glm::dot(ab, ao) > 0.0f) {
            direction = glm::cross(glm::cross(ab, ao), ab);
        }
        else {
            simplex = {sa};
            direction = ao;
        }

        return false;
    }

    static bool gjkNearestSimplexTriangle(std::deque<std::array<glm::vec3, 3>>& simplex, glm::vec3& direction) {
        std::array<glm::vec3, 3> sa = simplex[0];
        std::array<glm::vec3, 3> sb = simplex[1];
        std::array<glm::vec3, 3> sc = simplex[2];
        glm::vec3 a = sa[0];
        glm::vec3 b = sb[0];
        glm::vec3 c = sc[0];
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 ao = -a;

        glm::vec3 abc = glm::cross(ab, ac);
        
        if (glm::dot(glm::cross(abc, ac), ao) > 0.0f) {
            if (glm::dot(ac, ao) > 0.0f) {
                simplex = {sa, sc};
                direction = glm::cross(glm::cross(ac, ao), ac);
            }

            else {
                simplex = {sa, sb};
                return gjkNearestSimplexLine(simplex, direction);
            }
        }
        else {
            if (glm::dot(glm::cross(ab, abc), ao) > 0.0f) {
                simplex = {sa, sb};
                return gjkNearestSimplexLine(simplex, direction);
            }

            else {
                if (glm::dot(abc, ao) > 0.0f) {
                    direction = abc;
                }

                else {
                    simplex = {sa, sc, sb};
                    direction = -abc;
                }
            }
        }

        return false;
    }

    static bool gjkNearestSimplexTetrahedron(std::deque<std::array<glm::vec3, 3>>& simplex, glm::vec3& direction) {
        std::array<glm::vec3, 3> sa = simplex[0];
        std::array<glm::vec3, 3> sb = simplex[1];
        std::array<glm::vec3, 3> sc = simplex[2];
        std::array<glm::vec3, 3> sd = simplex[3];
        glm::vec3 a = sa[0];
        glm::vec3 b = sb[0];
        glm::vec3 c = sc[0];
        glm::vec3 d = sd[0];

        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 ad = d - a;
        glm::vec3 ao = -a;
    
        glm::vec3 abc = glm::cross(ab, ac);
        glm::vec3 acd = glm::cross(ac, ad);
        glm::vec3 adb = glm::cross(ad, ab);
    
        if (glm::dot(abc, ao) > 0.0f) {
            simplex = {sa, sb, sc};
            return gjkNearestSimplexTriangle(simplex, direction);
        }
            
        if (glm::dot(acd, ao) > 0.0f) {
            simplex = {sa, sc, sd};
            return gjkNearestSimplexTriangle(simplex, direction);
        }
    
        if (glm::dot(adb, ao) > 0.0f) {
            simplex = {sa, sd, sb};
            return gjkNearestSimplexTriangle(simplex, direction);
        }

        return true;
    }

    static bool gjkNearestSimplex(std::deque<std::array<glm::vec3, 3>>& nearestSimplex, glm::vec3& direction) {
        switch (nearestSimplex.size()) {
            case 2 :
                return gjkNearestSimplexLine(nearestSimplex, direction);
            case 3 :
                return gjkNearestSimplexTriangle(nearestSimplex, direction);
            case 4 :
                return gjkNearestSimplexTetrahedron(nearestSimplex, direction);
            default:
                return false;
        }
    }

    static std::pair<std::vector<glm::vec4>, std::size_t> gjkGetFaceNormals(std::deque<std::array<glm::vec3, 3>>& polytope, const std::vector<std::size_t>& faces) {
        std::vector<glm::vec4> normals;
        std::size_t minTriangle = 0;
        float minDistance = std::numeric_limits<float>::infinity();

        for (std::size_t i = 0; i < faces.size(); i += 3) {
            glm::vec3 a = polytope[faces[i]][0];
            glm::vec3 b = polytope[faces[i + 1]][0];
            glm::vec3 c = polytope[faces[i + 2]][0];

            glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
            float distance = glm::dot(normal, a);

            if (distance < 0.0f) {
                normal = -normal;
                distance = -distance;
            }

            normals.emplace_back(normal, distance);

            if (distance < minDistance) {
                minTriangle = i / 3;
                minDistance = distance;
            }
        }

        return {normals, minTriangle};
    }

    static void gjkAddUnique(std::vector<std::pair<std::size_t, std::size_t>>& edges, const std::vector<size_t>& faces, std::size_t a, std::size_t b) {
        auto reverse = std::find(
            edges.begin(),
            edges.end(),
            std::make_pair(faces[b], faces[a])
        );
    
        if (reverse != edges.end()) {
            edges.erase(reverse);
        }
        else {
            edges.emplace_back(faces[a], faces[b]);
        }
    }

    static DiscreteIntersectionInfo gjkEpa(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, std::deque<std::array<glm::vec3, 3>>& simplex) {
        std::vector<std::size_t> faces = {
            0, 1, 2,
            0, 3, 1,
            0, 2, 3,
            1, 3, 2
        };

        auto [normals, minFace] = gjkGetFaceNormals(simplex, faces);

        glm::vec3 minNormal;
        float minDistance = std::numeric_limits<float>::max();
        
        do {
            minNormal = glm::vec3(normals[minFace]);
            minDistance = normals[minFace].w;

            glm::vec3 supportPoint1 = convexSetSupportPoint(points1, minNormal);
            glm::vec3 supportPoint2 = convexSetSupportPoint(points2, -minNormal);
            glm::vec3 support = supportPoint1 - supportPoint2;

            if (std::abs(glm::dot(minNormal, support) - minDistance) > 0.001f) {
                minDistance = std::numeric_limits<float>::max();

                std::vector<std::pair<std::size_t, std::size_t>> uniqueEdges;

                for (std::size_t i = 0; i < normals.size(); ++i) {
                    if (glm::dot(glm::vec3(normals[i]), support) > glm::dot(glm::vec3(normals[i]), simplex[faces[i*3]][0])) {
                        std::size_t f = i * 3;

                        gjkAddUnique(uniqueEdges, faces, f, f + 1);
                        gjkAddUnique(uniqueEdges, faces, f + 1, f + 2);
                        gjkAddUnique(uniqueEdges, faces, f + 2, f);

                        faces[f + 2] = faces.back();
                        faces.pop_back();
                        faces[f + 1] = faces.back();
                        faces.pop_back();
                        faces[f] = faces.back();
                        faces.pop_back();

                        normals[i] = normals.back();
                        normals.pop_back();

                        --i;
                    }
                }

                std::vector<std::size_t> newFaces;
                for (auto [edgeIndex1, edgeIndex2] : uniqueEdges) {
                    newFaces.push_back(edgeIndex1);
                    newFaces.push_back(edgeIndex2);
                    newFaces.push_back(simplex.size());
                }
                
                simplex.push_back({support, supportPoint1, supportPoint2});

                auto [newNormals, newMinFace] = gjkGetFaceNormals(simplex, newFaces);

                float oldMinDistance = std::numeric_limits<float>::max();
                for (std::size_t i = 0; i < normals.size(); i++) {
                    if (normals[i].w < oldMinDistance) {
                        oldMinDistance = normals[i].w;
                        minFace = i;
                    }
                }

                if (newNormals[newMinFace].w < oldMinDistance) {
                    minFace = newMinFace + normals.size();
                }

                faces.insert(faces.end(), newFaces.begin(), newFaces.end());
                normals.insert(normals.end(), newNormals.begin(), newNormals.end());
            }
        } while (minDistance == std::numeric_limits<float>::max());

        std::vector<glm::vec3> contactPatch = supportsIntersection(points1, points2, -minNormal);

        return DiscreteIntersectionInfo(
            convexSetAverage(contactPatch),
            //minNormal * glm::dot(simplex[faces[3 * minFace]], minNormal),
            -minNormal,
            minDistance
        );
    }

    DiscreteIntersectionInfo gjk(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2) {
        // TODO Heuristic (velocity instead of random vector)
        glm::vec3 supportPoint1 = convexSetSupportPoint(points1, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 supportPoint2 = convexSetSupportPoint(points2, glm::vec3(-1.0f, 0.0f, 0.0f));
        glm::vec3 support = supportPoint1 - supportPoint2;
        std::deque<std::array<glm::vec3, 3>> simplex = {{support, supportPoint1, supportPoint2}};
        glm::vec3 direction = -support;
        
        // TODO I don't know why it crashes, but ignore faulty collisions for now... Set while(true) when fixed and remove i
        int i = 0;
        while (i++ < 20) {
            supportPoint1 = convexSetSupportPoint(points1, direction);
            supportPoint2 = convexSetSupportPoint(points2, -direction);
            support = supportPoint1 - supportPoint2;
            if (glm::dot(support, direction) < 0.0f) {
                return DiscreteIntersectionInfo::none();
            }
            
            simplex.push_front({support, supportPoint1, supportPoint2});

            if (gjkNearestSimplex(simplex, direction)) {
                return gjkEpa(points1, points2, simplex);
            }
        }

        return DiscreteIntersectionInfo::none(); 
    }

    bool arePointsOnSameSide(const glm::vec3& point1, const glm::vec3& point2, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        const glm::vec3 normal = glm::cross(c - a, b - a);
        return glm::dot(normal, point1 - a) * glm::dot(normal, point2 - a) > 0.0f;
    }

    // Point inside

    bool isPointInsideTetrahedron(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
        return arePointsOnSameSide(point, a, b, c, d) && arePointsOnSameSide(point, b, a, c, d) && arePointsOnSameSide(point, c, a, b, d) && arePointsOnSameSide(point, d, a, b, c);
    }

    bool isPointInsideConvex(const glm::vec3& point, const std::vector<glm::vec3>& vertices) {
        for (const auto [a, b, c, d] : convexSetTetrahedralization(vertices)) {
            if (isPointInsideTetrahedron(point, a, b, c, d)) {
                return true;
            }
        }
        return false;
    }

    // Closest point

    glm::vec3 closestPointOnLine(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, bool closed) {
        glm::vec3 ab = b - a;
        const float t = glm::dot(point - a, ab) / glm::dot(ab, ab);
        return a + (closed ? std::clamp(t, 0.0f, 1.0f) : t);
    }

    std::pair<glm::vec3, glm::vec3> closestPointsOnLines(const glm::vec3& a1, const glm::vec3& a2, const glm::vec3& b1, const glm::vec3& b2, bool closed1, bool closed2) {
        const glm::vec3 d = b1 - a1;
        const glm::vec3 u = a2 - a1;
        const glm::vec3 v = b2 - b1;

        const float du = glm::dot(d, u);
        const float dv = glm::dot(d, v);
        const float uu = glm::dot(u, u);
        const float uv = glm::dot(u, v);
        const float vv = glm::dot(v, v);

        const float det = uu * vv - uv * uv;

        float s;
        float t;

        if (det < glm::epsilon<float>()) {
            s = du / uu;
            t = 0.0f;
        }
        else {
            const float invDet = 1.0f / det;
            s = (du * vv - dv * uv) * invDet;
            t = (du * uv - dv * uu) * invDet;
        }

        if (closed1) {
            s = std::clamp(s, 0.0f, 1.0f);
        }
        if (closed2) {
            t = std::clamp(t, 0.0f, 1.0f);
        }

        float sp = (t * uv + du) / uu;
        float tp = (s * uv - dv) / vv;

        if (closed1) {
            sp = std::clamp(sp, 0.0f, 1.0f);
        }
        if (closed2) {
            tp = std::clamp(tp, 0.0f, 1.0f);
        }

        return {a1 + sp * u, b1 + tp * v};
    }

    glm::vec3 closestPointOnTriangle(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        auto [u, v, w] = barycentricCoordinates(point, a, b, c);
        if (u < 0.0f) {
            const glm::vec3 bc = c - b;
            const float t = std::clamp(glm::dot(point - b, bc) / glm::dot(bc, bc), 0.0f, 1.0f);
            u = 0.0f;
            v = 1.0f - t;
            w = t;
        }
        else if (v < 0.0f) {
            const glm::vec3 ac = c - a;
            const float t = std::clamp(glm::dot(point - c, ac) / glm::dot(ac, ac), 0.0f, 1.0f);
            u = t;
            v = 0.0f;
            w = 1.0f - t;
        }
        else if (w < 0.0f) {
            const glm::vec3 ab = b - a;
            const float t = std::clamp(glm::dot(point - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
            u = 1.0f - t;
            v = t;
            w = 0.0f;
        }

        return u * a + v * b + w * c;
    }

    std::pair<glm::vec3, glm::vec3> closestPointsLineTriangle(const glm::vec3& a1, const glm::vec3& a2, const glm::vec3& b1, const glm::vec3& b2, const glm::vec3 b3, bool closed) {
        glm::vec3 minimalClosestPoint1;
        glm::vec3 minimalClosestPoint2;
        float minimalClosestDistance = std::numeric_limits<float>::infinity();

        RaycastInfo lineTriangleIntersection = raycastTriangle(a1, a2 - a1, b1, b2, b3);
        if (lineTriangleIntersection.intersects() && (!closed || (lineTriangleIntersection.getParameter() >= 0.0f && lineTriangleIntersection.getParameter() <= 1.0f))) {
            return std::make_pair(lineTriangleIntersection.getIntersectionPoint(), lineTriangleIntersection.getIntersectionPoint());
        }

        if (closed) {
            const glm::vec3 p1 = closestPointOnTriangle(a1, b1, b2, b3);
            const glm::vec3 p2 = closestPointOnTriangle(a2, b1, b2, b3);
            const float distance1 = glm::distance2(a1, p1);
            const float distance2 = glm::distance2(a2, p2);

            if (distance1 < distance2) {
                minimalClosestPoint1 = a1;
                minimalClosestPoint2 = p1;
                minimalClosestDistance = distance1;
            }
            else {
                minimalClosestPoint1 = a2;
                minimalClosestPoint2 = p2;
                minimalClosestDistance = distance2;
            }
        }

        std::pair<glm::vec3, glm::vec3> closestPoints[]{
            closestPointsOnLines(a1, a2, b1, b2, closed, true),
            closestPointsOnLines(a1, a2, b2, b3, closed, true),
            closestPointsOnLines(a1, a2, b3, b1, closed, true)
        };

        for (int i = 0; i < 3; ++i) {
            const auto [a, b] = closestPoints[i];
            const float closestDistance = glm::distance2(a, b);
            if (closestDistance < minimalClosestDistance) {
                minimalClosestPoint1 = a;
                minimalClosestPoint2 = b;
                minimalClosestDistance = closestDistance;
            }
        }

        return std::make_pair(minimalClosestPoint1, minimalClosestPoint2);
    }

    glm::vec3 closestPointOnPolygon(const glm::vec3& point, const std::vector<glm::vec3>& points) {
        if (points.size() == 0) {
            throw std::runtime_error("Polygon is empty");
        }
        if (points.size() == 1) {
            return points[0];
        }
        if (points.size() == 2) {
            return closestPointOnLine(point, points[0], points[1], true);
        }
        glm::vec3 minimalClosestPoint;
        float minimalClosestDistance = std::numeric_limits<float>::infinity();
        for (const auto [a, b, c] : polygonTriangulation(points)) {
            glm::vec3 closestPoint = closestPointOnTriangle(point, a, b, c);
            const float closestDistance = glm::distance2(point, closestPoint);
            if (closestDistance < minimalClosestDistance) {
                minimalClosestDistance = closestDistance;
                minimalClosestPoint = closestPoint;
            }
        }
        return minimalClosestPoint;
    }

    glm::vec3 closestPointOnPlane(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        glm::vec3 normal = glm::cross(b - a, c - a);
        return point - (glm::dot(normal, point) - glm::dot(normal, a)) * normal;
    }

    glm::vec3 closestPointOnTetrahedron(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
        if (isPointInsideTetrahedron(point, a, b, c, d)) {
            return point;
        }
        const glm::vec3 trianglePoints[]{
            closestPointOnTriangle(point, a, b, c),
            closestPointOnTriangle(point, b, c, d),
            closestPointOnTriangle(point, a, b, d),
            closestPointOnTriangle(point, a, c, d)
        };
        glm::vec3 minimalClosestPoint;
        float minimalClosestDistance = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 4; ++i) {
            const float closestDistance = glm::distance2(point, trianglePoints[i]);
            if (closestDistance < minimalClosestDistance) {
                minimalClosestPoint = trianglePoints[i];
                minimalClosestDistance = closestDistance;
            }
        }
        return minimalClosestPoint;
    }

    std::pair<glm::vec3, glm::vec3> closestPointsLineTetrahedron(const glm::vec3& a1, const glm::vec3& a2, const glm::vec3& b1, const glm::vec3& b2, const glm::vec3 b3, const glm::vec3& b4, bool closed) {
        if (isPointInsideTetrahedron(a1, b1, b2, b3, b4)) {
            return std::make_pair(a1, a1);
        }
        if (isPointInsideTetrahedron(a2, b1, b2, b3, b4)) {
            return std::make_pair(a2, a2);
        }
        const std::pair<glm::vec3, glm::vec3> trianglePointsPairs[]{
            closestPointsLineTriangle(a1, a2, b1, b2, b3, true),
            closestPointsLineTriangle(a1, a2, b2, b3, b4, true),
            closestPointsLineTriangle(a1, a2, b1, b2, b4, true),
            closestPointsLineTriangle(a1, a2, b1, b3, b4, true)
        };

        glm::vec3 minimalClosestPoint1;
        glm::vec3 minimalClosestPoint2;
        float minimalClosestDistance = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 3; ++i) {
            const auto [x, y] = trianglePointsPairs[i];
            const float closestDistance = glm::distance2(x, y);
            if (closestDistance < minimalClosestDistance) {
                minimalClosestPoint1 = x;
                minimalClosestPoint2 = y;
                minimalClosestDistance = closestDistance;
            }
        }
        return std::make_pair(minimalClosestPoint1, minimalClosestPoint2);
    }

    glm::vec3 closestPointOnConvexSet(const glm::vec3& point, const std::vector<glm::vec3>& vertices) {
        glm::vec3 minimalClosestPoint;
        float minimalClosestDistance = std::numeric_limits<float>::infinity();
        for (const auto [a, b, c] : convexSetTriangulation(vertices)) {
            const glm::vec3 closestPoint = closestPointOnTriangle(point, a, b, c);
            const float closestDistance = glm::distance2(point, closestPoint);
            if (closestDistance < minimalClosestDistance) {
                minimalClosestPoint = closestPoint;
                minimalClosestDistance = closestDistance;
            }
        }

        return minimalClosestPoint;
    }

    std::pair<glm::vec3, glm::vec3> closestPointsLineConvexSet(const glm::vec3& a1, const glm::vec3& a2, const std::vector<glm::vec3>& vertices, bool closed) {
        glm::vec3 minimalClosestPoint1;
        glm::vec3 minimalClosestPoint2;
        float minimalClosestDistance = std::numeric_limits<float>::infinity();
        for (const auto [a, b, c] : convexSetTriangulation(vertices)) {
            const auto [x, y] = closestPointsLineTriangle(a1, a2, a, b, c, closed);
            const float closestDistance = glm::distance2(x, y);
            if (closestDistance < minimalClosestDistance) {
                minimalClosestPoint1 = x;
                minimalClosestPoint2 = y;
                minimalClosestDistance = closestDistance;
            }
        }

        return std::make_pair(minimalClosestPoint1, minimalClosestPoint2);
    }

    glm::vec3 arbitraryOrthogonal(const glm::vec3& direction) {
        return glm::cross(direction, direction.x == 0.0f ? glm::vec3(1.0f, 0.0f, 0.5f) : glm::vec3(0.0f, 1.0f, 0.5f));
    }

    glm::vec3 convexSetSupportPoint(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction) {
        float maxDot = -std::numeric_limits<float>::infinity();
        glm::vec3 supportPoint(0);
        for (const glm::vec3& point : convexSet) {
            float dot = glm::dot(point, direction);
            if (dot > maxDot) {
                maxDot = dot;
                supportPoint = point;
            }
        }
        return supportPoint;
    }

    std::array<glm::vec3, 3> convexSetSupportTriangle(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction) {
        float maxDot[3] = {-std::numeric_limits<float>::infinity()};
        std::array<glm::vec3, 3> supportPoint;
        for (const glm::vec3& point : convexSet) {
            float dot = glm::dot(point, direction);
            for (int i = 0; i < 3; ++i) {
                if (dot > maxDot[i]) {
                    maxDot[i] = dot;
                    supportPoint[i] = point;
                    break;
                }
            }
        }
        return supportPoint;
    }

    std::vector<glm::vec3> convexSetSupportPolygon(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction) {
        float maxDot = -std::numeric_limits<float>::infinity();
        std::vector<glm::vec3> supportPlane;
        for (const glm::vec3& point : convexSet) {
            float dot = glm::dot(point, direction);
            if (dot > maxDot) {
                maxDot = dot;
            }
        }

        for (const glm::vec3& point : convexSet) {
            if (glm::dot(point, direction) == maxDot) {
                supportPlane.push_back(point);
            }
        }
        return supportPlane;
    }

    std::vector<std::array<glm::vec3, 3>> polygonTriangulation(const std::vector<glm::vec3>& polygon) {
        if (polygon.size() < 3) {
            throw std::runtime_error("Polygon of size less than 3 (" + std::to_string(polygon.size()) + ") cannot be broken down into triangles");
        }

        const glm::vec3 normal = glm::cross(polygon[2] - polygon[0], polygon[1] - polygon[0]);
        const glm::vec3 sweepingDirection = arbitraryOrthogonal(normal);
        const glm::vec3 orthogonalDirection = glm::cross(normal, sweepingDirection);

        std::vector<std::array<glm::vec3, 3>> triangles;
        std::vector<glm::vec3> sortedPolygon = polygon;
        std::sort(sortedPolygon.begin(), sortedPolygon.end(), sweepSortingFunction(sweepingDirection));
        
        std::size_t j;
        std::size_t k;
        if (glm::dot(sortedPolygon[1] - sortedPolygon[0], orthogonalDirection) > 0.0f) {
            j = 1;
            k = 0;
        }
        else {
            j = 0;
            k = 1;
        }

        for (std::size_t i = 2; i < sortedPolygon.size(); ++i) {
            triangles.push_back({sortedPolygon[j], sortedPolygon[k], sortedPolygon[i]});
            if (glm::dot(sortedPolygon[i] - sortedPolygon[0], orthogonalDirection) > 0.0f) {
                j = i;
            }
            else {
                k = i;
            }
        }
        return triangles;
    }

    static float triangulationDistance(const glm::vec3& a, const glm::vec3& b) {
        const float y = b.y - a.y;
        const float z = b.z - a.z;
        return y * y + z * z;
    }

    static std::size_t closestPointInHull(const glm::vec3& point, const std::deque<glm::vec3>& hull) {
        std::size_t closestIndex = 0;
        float closestDistance = std::numeric_limits<float>::infinity();
        for (std::size_t i = 0; i < hull.size(); ++i) {
            const float distance = triangulationDistance(point, hull[i]);
            if (distance < closestDistance) {
                closestIndex = i;
                closestDistance = distance;
            }
        }
        return closestIndex;
    }

    static std::size_t nextPointInHull(const std::deque<glm::vec3>& hull, std::size_t current, bool clockwise) {
        return clockwise ? (current == hull.size() - 1 ? 0 : current + 1) : (current == 0 ? hull.size() - 1 : current - 1);
    }

    std::vector<std::array<glm::vec3, 3>> convexSetTriangulation(const std::vector<glm::vec3>& convexSet) {
        if (convexSet.size() < 3) {
            throw std::runtime_error("Convex set of size less than 3 (" + std::to_string(convexSet.size()) + ") cannot be broken down into triangles");
        }

        std::vector<glm::vec3> sortedConvexSet = convexSet;

        const glm::vec3 sweepingDirection = glm::vec3(1.0f, 0.0f, 0.0f);
        std::sort(sortedConvexSet.begin(), sortedConvexSet.end(), sweepSortingFunction(sweepingDirection));

        std::vector<std::array<glm::vec3, 3>> triangles{{sortedConvexSet[0], sortedConvexSet[1], sortedConvexSet[2]}};
        std::deque<glm::vec3> hull{sortedConvexSet[0]};
        
        if (glm::dot(sortedConvexSet[2] - sortedConvexSet[0], glm::cross(sweepingDirection, sortedConvexSet[1] - sortedConvexSet[0])) >= 0.0f) {
            hull.push_back(sortedConvexSet[2]);
            hull.push_back(sortedConvexSet[1]);
        }
        else {
            hull.push_back(sortedConvexSet[1]);
            hull.push_back(sortedConvexSet[2]);
        }
        
        for (std::size_t i = 3; i < sortedConvexSet.size(); ++i) {
            std::size_t closestIndex = closestPointInHull(sortedConvexSet[i], hull);
            
            std::size_t previous = nextPointInHull(hull, closestIndex, false);
            std::size_t next = nextPointInHull(hull, closestIndex, true);

            std::set<std::size_t, std::greater<std::size_t>> hullDeleted;

            std::size_t putAt = 0;

            // Inside
            if (glm::dot(sortedConvexSet[i] - hull[previous], glm::cross(sweepingDirection, hull[closestIndex] - hull[previous])) <= 0.0f && glm::dot(sortedConvexSet[i] - hull[closestIndex], glm::cross(sweepingDirection, hull[next] - hull[closestIndex])) <= 0.0f) {
                hullDeleted.insert(closestIndex);
                triangles.push_back({sortedConvexSet[i], hull[closestIndex], hull[previous]});
                triangles.push_back({sortedConvexSet[i], hull[closestIndex], hull[next]});
                while (glm::dot(sortedConvexSet[i] - hull[previous], glm::cross(sweepingDirection, hull[next] - hull[previous])) < 0.0f) {
                    if (triangulationDistance(sortedConvexSet[i], hull[previous]) < triangulationDistance(sortedConvexSet[i], hull[previous])) {
                        std::size_t current = previous;
                        previous = nextPointInHull(hull, previous, false);
                        triangles.push_back({sortedConvexSet[i], hull[current], hull[previous]});
                        hullDeleted.insert(current);
                    }
                    else {
                        std::size_t current = next;
                        next = nextPointInHull(hull, next, true);
                        triangles.push_back({sortedConvexSet[i], hull[current], hull[next]});
                        hullDeleted.insert(current);
                    }
                }
                putAt = nextPointInHull(hull, previous, true);
            }
            // Outside
            else {
                bool rightHullExpansion = false;
                bool leftHullExpansion = false;

                std::size_t current = closestIndex;

                // Check clockwise
                while (glm::dot(hull[next] - hull[current], glm::cross(sweepingDirection, hull[current] - sortedConvexSet[i])) >= 0.0f) {
                    triangles.push_back({sortedConvexSet[i], hull[current], hull[next]});
                    if (rightHullExpansion) {
                        hullDeleted.insert(current);
                    }
                    current = next;
                    next = nextPointInHull(hull, current, true);
                    rightHullExpansion = true;
                };

                // Check counterclockwise
                current = closestIndex;
                while (glm::dot(hull[current] - hull[previous], glm::cross(sweepingDirection, hull[current] - sortedConvexSet[i])) >= 0.0f) {
                    triangles.push_back({sortedConvexSet[i], hull[current], hull[previous]});
                    if (leftHullExpansion) {
                        hullDeleted.insert(current);
                    }
                    current = previous;
                    previous = nextPointInHull(hull, current, false);
                    leftHullExpansion = true;
                };

                // If expanded both sides, suppress the closest
                if (rightHullExpansion && leftHullExpansion) {
                    hullDeleted.insert(closestIndex);
                }

                if (leftHullExpansion) {
                    putAt = nextPointInHull(hull, previous, true);
                }
                else {
                    putAt = nextPointInHull(hull, closestIndex, true);
                }
            }

            for (std::size_t index : hullDeleted) {
                hull.erase(hull.begin() + index);
            }

            hull.insert(hull.begin() + putAt, sortedConvexSet[i]);
        }

        // Finish with the remaining faces of the hull
        for (std::size_t i = 1; i < hull.size() - 1; ++i) {
            triangles.push_back({hull[0], hull[i], hull[i + 1]});
        }

        return triangles;
    }

    std::vector<std::array<glm::vec3, 4>> convexSetTetrahedralization(const std::vector<glm::vec3>& convexSet) {
        if (convexSet.size() < 4) {
            throw std::runtime_error("Convex set of size less than 4 (" + std::to_string(convexSet.size()) + ") cannot be broken down into tetrahedra");
        }

        std::vector<std::array<glm::vec3, 4>> tetrahedra;
        const glm::vec3 barycenter = convexSetAverage(convexSet);
        for (const auto [a, b, c] : convexSetTriangulation(convexSet)) {
            tetrahedra.push_back({barycenter, a, b, c});
        }
        return tetrahedra;
    }

    glm::vec3 convexSetAverage(const std::vector<glm::vec3>& convexSet) {
        glm::vec3 average(0.0f);

        for (const glm::vec3& point : convexSet) {
            average += point;
        }

        return average / static_cast<float>(convexSet.size());
    }

    glm::vec3 weightedConvexSetAverage(const std::vector<glm::vec3>& convexSet, const std::vector<float>& pointWeights) {
        if (convexSet.size() != pointWeights.size()) {
            throw std::runtime_error("Cannot calculate the convex barycenter of unequal sizes of position and mass vectors");
        }

        float totalWeight = 0.0f;
        glm::vec3 average(0.0f);

        for (std::size_t i = 0; i < convexSet.size(); ++i) {
            totalWeight += pointWeights[i];
            average += convexSet[i] * pointWeights[i];
        }

        return average / totalWeight;
    }

    /*glm::vec3 uniformConvexSetBarycenter(const std::vector<glm::vec3>& convexSet) {
        std::vector<std::array<glm::vec3, 4>> tetrahedra = convexSetTetrahedralization(convexSet);
        std::vector<glm::vec3> centroids(tetrahedra.size());
        std::vector<float> weights(tetrahedra.size());
        for (std::size_t i = 0; i < tetrahedra.size(); ++i) {
            const auto [a, b, c, d] = tetrahedra[i];
            centroids[i] = 0.25f * (a + b + c + d);
            weights[i] = tetrahedronVolume(a, b, c, d);
        }
        return weightedConvexSetAverage(centroids, weights);
    }*/

    std::vector<glm::vec3> getBoxVertices(const glm::vec3& halfLengths) {
        return {
            glm::vec3(-halfLengths.x, -halfLengths.y, -halfLengths.z),
            glm::vec3(-halfLengths.x, -halfLengths.y, halfLengths.z),
            glm::vec3(-halfLengths.x, halfLengths.y, -halfLengths.z),
            glm::vec3(-halfLengths.x, halfLengths.y, halfLengths.z),
            glm::vec3(halfLengths.x, -halfLengths.y, -halfLengths.z),
            glm::vec3(halfLengths.x, -halfLengths.y, halfLengths.z),
            glm::vec3(halfLengths.x, halfLengths.y, -halfLengths.z),
            glm::vec3(halfLengths.x, halfLengths.y, halfLengths.z)
        };
    }

    std::vector<glm::vec3> translatePoints(const std::vector<glm::vec3>& points, const glm::vec3& translation) {
        std::vector<glm::vec3> translatedPoints(points.size());
        for (std::size_t i = 0; i < points.size(); ++i) {
            translatedPoints[i] = points[i] + translation;
        }
        return translatedPoints;
    }

    std::vector<glm::vec3> transformPoints(const std::vector<glm::vec3>& points, const glm::vec3& translation, const glm::mat3& rotation) {
        if (rotation == glm::identity<glm::mat3>()) {
            return translatePoints(points, translation);
        }
        std::vector<glm::vec3> translatedPoints(points.size());
        for (std::size_t i = 0; i < points.size(); ++i) {
            translatedPoints[i] = rotation * points[i] + translation;
        }
        return translatedPoints;
    }

    std::vector<glm::vec3> transformPoints(const std::vector<glm::vec3>& points, const glm::vec3& translation, const glm::quat& rotation) {
        if (rotation == glm::identity<glm::quat>()) {
            return translatePoints(points, translation);
        }
        std::vector<glm::vec3> transformedPoints(points.size());
        for (std::size_t i = 0; i < points.size(); ++i) {
            transformedPoints[i] = rotation * points[i] + translation;
        }
        return transformedPoints;
    }

    std::pair<glm::vec3, glm::vec3> capsuleHemispheres(const glm::vec3& position, const glm::quat& rotation, float halfHeight) {
        return std::make_pair(
            position + rotation * glm::vec3(0.0f, -halfHeight, 0.0f),
            position + rotation * glm::vec3(0.0f, halfHeight, 0.0f)
        );
    }

    std::array<float, 3> barycentricCoordinates(const glm::vec3& point, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        const glm::vec3 d = point - a;
        const glm::vec3 u = b - a;
        const glm::vec3 v = c - a;

        const float du = glm::dot(d, u);
        const float dv = glm::dot(d, v);
        const float uu = glm::dot(u, u);
        const float uv = glm::dot(u, v);
        const float vv = glm::dot(v, v);

        const float invDet = 1.0f / (vv * uu - uv * uv);
        const float x = (uu * dv - uv * du) * invDet;
        const float y = (vv * du - uv * dv) * invDet;
        const float z = 1.0f - x - y;

        return {z, y, x};
    }

    RaycastInfo raycastTriangle(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        const glm::vec3 x = b - a;
        const glm::vec3 y = c - a;
        const glm::vec3 h = glm::cross(rayDirection, y);
        const float d = glm::dot(x, h);

        if (d > -glm::epsilon<float>() && d < glm::epsilon<float>()) {
            return RaycastInfo::miss();
        }

        const float f = 1.0f / d;
        const glm::vec3 s = rayOrigin - a;
        const float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f) {
            return RaycastInfo::miss(); 
        }

        const glm::vec3 q = glm::cross(s, x);
        const float v = f * glm::dot(rayDirection, q);

        if (v < 0.0f || u + v > 1.0f) {
            return RaycastInfo::miss(); 
        }

        const float t = f * glm::dot(y, q);
        
        if (t <= glm::epsilon<float>()) {
            return RaycastInfo::miss();  
        }

        const glm::vec3 normal = glm::normalize(h);

        return RaycastInfo(
            rayOrigin + t * rayDirection,
            glm::dot(rayDirection, normal) < 0.0f ? normal : -normal,
            t
        );
    }

    RaycastInfo raycastSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& position, float radius) {
        glm::vec3 difference = rayOrigin - position;
        const float a = 2.0f * glm::dot(rayDirection, difference);
        const float b = glm::length2(rayDirection);
        const float c = 4.0f * b * (glm::length2(difference) - radius * radius);
        const float d = a * a - c;
        if (d < glm::epsilon<float>()) {
            return RaycastInfo::miss();
        }
        const float t = -(a + std::sqrt(d)) / (2.0f * b) - glm::epsilon<float>();
        if (t < 0.0f) {
            return RaycastInfo::miss();
        }
        const glm::vec3 hitPosition = rayOrigin + t * rayDirection;
        const glm::vec3 normal = glm::normalize(hitPosition - position);
        return RaycastInfo(
            hitPosition,
            normal,
            t
        );
    }

    RaycastInfo raycastBox(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& halfLengths) {
        return RaycastInfo::miss();
    }

    RaycastInfo raycastCapsule(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& position, const glm::vec3& rotation, float radius, float halfHeight) {
        const auto [hemisphere1, hemisphere2] = capsuleHemispheres(position, rotation, halfHeight);
        const auto [_, closestPoint] = closestPointsOnLines(rayOrigin, rayOrigin + rayDirection, hemisphere1, hemisphere2, false, true);
        return raycastSphere(rayOrigin, rayDirection, closestPoint, radius);
    }

    RaycastInfo raycastTetrahedron(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& d) {
        RaycastInfo raycasts[]{
            raycastTriangle(rayOrigin, rayDirection, a, b, c),
            raycastTriangle(rayOrigin, rayDirection, b, c, d),
            raycastTriangle(rayOrigin, rayDirection, a, b, d),
            raycastTriangle(rayOrigin, rayDirection, a, c, d)
        };

        int minimalDistanceRaycast = -1;
        float minimalParameter = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 4; ++i) {
            if (raycasts[i].intersects() && raycasts[i].getParameter() < minimalParameter) {
                minimalDistanceRaycast = i;
                minimalParameter = raycasts[i].getParameter();
            }
        }

        if (minimalDistanceRaycast == -1) {
            return RaycastInfo::miss();
        }
        return raycasts[minimalDistanceRaycast];
    }

    RaycastInfo raycastConvexSet(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const std::vector<glm::vec3>& vertices) {
        return RaycastInfo::miss();
    }

    BoundingBox sphereBoundingBox(const glm::vec3& center, float radius) {
        return std::make_pair(
            center - glm::vec3(radius),
            center + glm::vec3(radius)
        );
    }

    BoundingBox capsuleBoundingBox(const glm::vec3& center1, const glm::vec3& center2, float radius) {
        auto [minCenter1, maxCenter1] = sphereBoundingBox(center1, radius);
        auto [minCenter2, maxCenter2] = sphereBoundingBox(center2, radius);
        return std::make_pair(
            glm::vec3(std::min(minCenter1.x, minCenter2.x), std::min(minCenter1.y, minCenter2.y), std::min(minCenter1.z, minCenter2.z)),
            glm::vec3(std::max(maxCenter1.x, maxCenter2.x), std::max(maxCenter1.y, maxCenter2.y), std::max(maxCenter1.z, maxCenter2.z))
        );
    }

    BoundingBox convexSetBoundingBox(const std::vector<glm::vec3>& convexSet) {
        glm::vec3 min(std::numeric_limits<float>::infinity());
        glm::vec3 max(-std::numeric_limits<float>::infinity());
        for (const glm::vec3& point : convexSet) {
            for (int i{}; i < 3; ++i) {
                min[i] = std::min(min[i], point[i]);
                max[i] = std::max(max[i], point[i]);
            }
        }
        return std::make_pair(min, max);
    }

    DiscreteIntersectionInfo AABBAABBIntersection(const glm::vec3& position1, const glm::vec3& halfLength1, const glm::vec3& position2, const glm::vec3 halfLength2) {
        const glm::vec3 sumSize = halfLength1 + halfLength2;
        const glm::vec3 difference = glm::abs(position1 - position2);
        if (glm::all(glm::lessThanEqual(difference, sumSize))) {
            const glm::vec3 penetration{sumSize - difference};
            if (penetration.x < penetration.y && penetration.x < penetration.z) {
                const glm::vec3 normal = glm::vec3(position1.x >= position2.x ? 1.0f : -1.0f, 0.0f, 0.0f);

                return DiscreteIntersectionInfo(
                    glm::vec3(
                        position1.x - halfLength1.x * normal.x - 0.5f * penetration.x,
                        0.5f * (std::max(position1.y - halfLength1.y, position2.y - halfLength2.y) + std::min(position1.y + halfLength1.y, position2.y + halfLength2.y)),
                        0.5f * (std::max(position1.z - halfLength1.z, position2.z - halfLength2.z) + std::min(position1.z + halfLength1.z, position2.z + halfLength2.z))
                    ),
                    normal,
                    penetration.x
                );
            }
            else if (penetration.y < penetration.x && penetration.y < penetration.z) {
                const glm::vec3 normal = glm::vec3(0.0f, position1.y >= position2.y ? 1.0f : -1.0f, 0.0f);

                return DiscreteIntersectionInfo(
                    glm::vec3(
                        0.5f * (std::max(position1.x - halfLength1.x, position2.x - halfLength2.x) + std::min(position1.x + halfLength1.x, position2.x + halfLength2.x)),
                        position1.y - halfLength1.y * normal.y - 0.5f * penetration.y,
                        0.5f * (std::max(position1.z - halfLength1.z, position2.z - halfLength2.z) + std::min(position1.z + halfLength1.z, position2.z + halfLength2.z))
                    ),
                    normal,
                    penetration.y
                );
            }
            else {
                const glm::vec3 normal = glm::vec3(0.0f, 0.0f, position1.z >= position2.z ? 1.0f : -1.0f);

                return DiscreteIntersectionInfo(
                    glm::vec3(
                        0.5f * (std::max(position1.x - halfLength1.x, position2.x - halfLength2.x) + std::min(position1.x + halfLength1.x, position2.x + halfLength2.x)),
                        0.5f * (std::max(position1.y - halfLength1.z, position2.y - halfLength2.y) + std::min(position1.y + halfLength1.y, position2.y + halfLength2.y)),
                        position1.z - halfLength1.z * normal.z - 0.5f * penetration.z
                    ),
                    normal,
                    penetration.z
                );
            }
        }
        return DiscreteIntersectionInfo::none();
    }

    DiscreteIntersectionInfo boxBoxIntersection(const glm::vec3& position1, const glm::quat& rotation1, const glm::vec3& halfLength1, const glm::vec3& position2, const glm::quat& rotation2, const glm::vec3 halfLength2) {
        if (rotation1 == glm::identity<glm::quat>() && rotation2 == glm::identity<glm::quat>()) {
            return AABBAABBIntersection(position1, halfLength1, position2, halfLength2);
        }

        return gjk(
            transformPoints(getBoxVertices(halfLength1), position1, rotation1),
            transformPoints(getBoxVertices(halfLength2), position2, rotation2)
        );

        /*std::vector<glm::vec3> axes(6);
        axes[0] = rotation1 * glm::vec3(1.0f, 0.0f, 0.0f);
        axes[1] = rotation1 * glm::vec3(0.0f, 1.0f, 0.0f);
        axes[2] = rotation1 * glm::vec3(0.0f, 0.0f, 1.0f);

        axes[3] = rotation2 * glm::vec3(1.0f, 0.0f, 0.0f);
        axes[4] = rotation2 * glm::vec3(0.0f, 1.0f, 0.0f);
        axes[5] = rotation2 * glm::vec3(0.0f, 0.0f, 1.0f);

        for (int i = 0; i < 3; ++i) {
            for (int j = 3; j < 6; ++j) {
                const glm::vec3 crossAxes = glm::cross(axes[i], axes[j]);
                if (crossAxes != glm::vec3(0.0f, 0.0f, 0.0f)) {
                    axes.push_back(glm::normalize(crossAxes));
                }
            }
        }

        return sat(
            transformPoints(getBoxVertices(halfLength1), position1, rotation1),
            transformPoints(getBoxVertices(halfLength2), position2, rotation2),
            axes
        );*/
    }

    DiscreteIntersectionInfo boxSphereIntersection(const glm::vec3& position1, const glm::quat& rotation, const glm::vec3& halfLength, const glm::vec3& position2, float radius) {
        std::vector<glm::vec3> axes(3);
        axes[0] = rotation * glm::vec3(halfLength.x, 0.0f, 0.0f);
        axes[1] = rotation * glm::vec3(0.0f, halfLength.y, 0.0f);
        axes[2] = rotation * glm::vec3(0.0f, 0.0f, halfLength.z);

        const glm::vec3 relativePosition = position2 - position1;
        glm::vec3 normalizedPosition;
        for (int i = 0; i < 3; ++i) {
            normalizedPosition[i] = std::clamp(glm::dot(relativePosition, axes[i]) / glm::length2(axes[i]), -1.0f, 1.0f);
        }
        const glm::vec3 normalizedDifference = glm::abs(normalizedPosition);
        if (glm::all(glm::lessThan(normalizedDifference, glm::vec3(1.0f)))) {
            // Sphere inside
            float penetration;
            glm::vec3 normal;
            if (normalizedDifference[0] > normalizedDifference[1] && normalizedDifference[0] > normalizedDifference[2]) {
                penetration = (1.0f - normalizedDifference[0]) * halfLength.x;
                normal = axes[0] / halfLength.x;
            }
            else if (normalizedDifference[1] > normalizedDifference[0] && normalizedDifference[1] > normalizedDifference[2]) {
                penetration = (1.0f - normalizedDifference[1]) * halfLength.y;
                normal = axes[1] / halfLength.y;
            }
            else {
                penetration = (1.0f - normalizedDifference[2]) * halfLength.z;
                normal = axes[2] / halfLength.z;
            }
            return DiscreteIntersectionInfo(
                position2 + 0.5f * penetration * normal,
                normal,
                penetration
            );
        }
        else {
            // Sphere outside
            glm::vec3 closestPointPosition(0.0f);
            for (int i = 0; i < 3; ++i) {
                closestPointPosition += axes[i] * normalizedPosition[i];
            }
            return pointSphereIntersection(closestPointPosition + position1, position2, radius);
        }
    }

    DiscreteIntersectionInfo boxCapsuleIntersection(const glm::vec3& position1, const glm::quat& rotation1, const glm::vec3& halfLength, const glm::vec3& position2, const glm::quat& rotation2, float halfHeight, float radius) {
        // TODO There's probably easier stuff with boxes...
        return capsuleConvexIntersection(position2, rotation2, halfHeight, radius, position1, rotation1, getBoxVertices(halfLength)).getInverse();
    }

    DiscreteIntersectionInfo pointSphereIntersection(const glm::vec3& pointPosition, const glm::vec3& center, float radius) {
        return sphereSphereIntersection(pointPosition, 0.0f, center, radius);
    }

    DiscreteIntersectionInfo sphereSphereIntersection(const glm::vec3& center1, float radius1, const glm::vec3& center2, float radius2) {
        if (center1 == center2) {
            const float penetration = std::max(radius1, radius2);
            return DiscreteIntersectionInfo(
                center1 + glm::vec3(0.5f * penetration, 0.0f, 0.0f),
                glm::vec3(-1.0f, 0.0f, 0.0f),
                penetration
            );
        }
        const float radiiSum{radius1 + radius2};
        const float distanceSquared = glm::distance2(center1, center2);
        if (distanceSquared <= radiiSum * radiiSum) {
            const glm::vec3 normal = glm::normalize(center1 - center2);
            const float penetration = radiiSum - std::sqrt(distanceSquared);
            return DiscreteIntersectionInfo(
                center2 + normal * (radius2 - 0.5f * penetration),
                normal,
                penetration
            );
        }
        return DiscreteIntersectionInfo::none();
    }
    
    DiscreteIntersectionInfo sphereCapsuleIntersection(const glm::vec3& position1, float radius1, const glm::vec3& position2, const glm::quat& rotation, float halfLength, float radius2) {
        auto [capsuleCenter1, capsuleCenter2] = capsuleHemispheres(position2, rotation, halfLength);
        const glm::vec3 closestPoint = closestPointOnLine(position1, capsuleCenter1, capsuleCenter2, true);
        return sphereSphereIntersection(position1, radius1, closestPoint, radius2);
    }

    DiscreteIntersectionInfo sphereConvexIntersection(const glm::vec3& position1, float radius, const glm::vec3& position2, const glm::quat& rotation, const std::vector<glm::vec3> vertices) {
        const glm::vec3 closestPoint = closestPointOnConvexSet(position1, transformPoints(vertices, position2, rotation));
        return pointSphereIntersection(position1, closestPoint, radius);
    }

    DiscreteIntersectionInfo capsuleConvexIntersection(const glm::vec3& position1, const glm::quat& rotation1, float halfHeight, float radius, const glm::vec3& position2, const glm::quat& rotation2, const std::vector<glm::vec3> vertices) {        
        const auto [hemisphere1, hemisphere2] = capsuleHemispheres(position1, rotation1, halfHeight);
        const auto [a, b] = closestPointsLineConvexSet(hemisphere1, hemisphere2, transformPoints(vertices, position2, rotation2), true);
        return pointSphereIntersection(b, a, radius);
    }

    DiscreteIntersectionInfo capsuleCapsuleIntersection(const glm::vec3& position1, const glm::quat& rotation1, float halfHeight1, float radius1, const glm::vec3& position2, const glm::quat& rotation2, float halfHeight2, float radius2) {
        const auto [hemisphere11, hemisphere12] = capsuleHemispheres(position1, rotation1, halfHeight1);
        const auto [hemisphere21, hemisphere22] = capsuleHemispheres(position2, rotation2, halfHeight2);
        const auto [closestPoint1, closestPoint2] = closestPointsOnLines(hemisphere11, hemisphere12, hemisphere21, hemisphere22, true, true);
        return sphereSphereIntersection(closestPoint1, radius1, closestPoint2, radius2);
    }

    DiscreteIntersectionInfo convexConvexIntersection(const glm::vec3& position1, const glm::quat& rotation1, const std::vector<glm::vec3>& vertices1, const glm::vec3& position2, const glm::quat& rotation2, const std::vector<glm::vec3>& vertices2) {
        return gjk(
            transformPoints(vertices1, position1, rotation1),
            transformPoints(vertices2, position2, rotation2)
        );
    }

    static std::pair<std::vector<glm::vec3>, std::vector<glm::vec3>> halvePolygon(const std::vector<glm::vec3>& polygon, const glm::vec3& orthogonalDirection) {
        std::vector<glm::vec3> firstHalf;
        std::vector<glm::vec3> secondHalf;
        firstHalf.push_back(polygon[0]);
        secondHalf.push_back(polygon[0]);
        
        for (std::size_t i = 1; i < polygon.size() - 1; ++i) {
            if (glm::dot(orthogonalDirection, polygon[i] - polygon[0]) > 0.0f) {
                firstHalf.push_back(polygon[i]);
            }
            else {
                secondHalf.push_back(polygon[i]);
            }
        }
        firstHalf.push_back(polygon[polygon.size() - 1]);
        secondHalf.push_back(polygon[polygon.size() - 1]);
        return std::make_pair(firstHalf, secondHalf);
    }

    static void addInnerPoints(std::vector<glm::vec3>& intersectionPoints, const std::vector<glm::vec3>& half, const std::vector<glm::vec3>& points, const glm::vec3& sweepDirection, const glm::vec3& orthogonalDirection) {
        std::size_t i = 0;
        std::size_t j = 0;
        float a = glm::dot(half[0], sweepDirection);

        while (j < points.size()) {
            const float b = glm::dot(half[i + 1], sweepDirection);
            const float u = glm::dot(points[j], sweepDirection);

            if (a <= u && u < b) {
                const glm::vec3 v = glm::mix(half[i], half[i + 1], (u - a) / (b - a));

                const float up = glm::dot(points[j], orthogonalDirection);
                const float vp = glm::dot(v, orthogonalDirection);
                
                if (up >= 0.0f && up < vp) {
                    intersectionPoints.push_back(points[j]);
                }
            }

            if (i < half.size() - 1 && b < u) {
                a = b;
                ++i;
            }
            else {
                ++j;
            }
        }
    }

    static void addIntersections(std::vector<glm::vec3>& intersectionPoints, const std::vector<glm::vec3>& firstHalf, const std::vector<glm::vec3>& secondHalf, const glm::vec3& sweepDirection, const glm::vec3& orthogonalDirection) {
        std::size_t i = 0;
        std::size_t j = 0;

        float aix = glm::dot(firstHalf[0], sweepDirection);
        float ajx = glm::dot(secondHalf[0], sweepDirection);

        while (i < firstHalf.size() - 1 && j < secondHalf.size() - 1) {
            const float bix = glm::dot(firstHalf[i + 1], sweepDirection);
            const float bjx = glm::dot(secondHalf[j + 1], sweepDirection);

            const float s = std::max(aix, ajx);
            const float e = std::min(bix, bjx);

            if (s <= e) {
                const float aiy = glm::dot(firstHalf[i], orthogonalDirection);
                const float biy = glm::dot(firstHalf[i + 1], orthogonalDirection);
                const float ajy = glm::dot(secondHalf[j], orthogonalDirection);
                const float bjy = glm::dot(secondHalf[j + 1], orthogonalDirection);

                const float ix = aix - bix;
                const float jx = ajx - bjx;
                const float d = jx * (aix * biy - aiy * bix) - ix * (ajx * bjy - ajy * bjx);
                const float x = d / (ix * (ajy - bjy) - jx * (aiy - biy));                
                if (s <= x && x < e) {
                    const float xr = (x - aix) / (bix - aix);
                    const glm::vec3 v = glm::mix(firstHalf[i], firstHalf[i + 1], xr);
                    intersectionPoints.push_back(v);
                }
            }

            if (i < firstHalf.size() - 1 && aix < ajx) {
                aix = bix;
                ++i;
            }
            else {
                ajx = bjx;
                ++j;
            }
        }
    }

    // TODO It looks like it works, although I'm not entirely sure
    std::vector<glm::vec3> supportsIntersection(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, const glm::vec3& supportDirection) {
        std::vector<glm::vec3> supportPoints1 = convexSetSupportPolygon(points1, -supportDirection);
        std::vector<glm::vec3> supportPoints2 = convexSetSupportPolygon(points2, supportDirection);

        if (supportPoints1.size() == 1) {
            return {supportPoints1[0]};
        }
        else if (supportPoints2.size() == 1) {
            return {supportPoints2[0]};
        }

        float midShift = 0.5f * glm::dot(supportPoints2[0] - supportPoints1[0], supportDirection);

        supportPoints1 = translatePoints(supportPoints1, midShift * supportDirection);
        supportPoints2 = translatePoints(supportPoints2, -midShift * supportDirection);

        glm::vec3 sweepDirection = arbitraryOrthogonal(supportDirection);
        sweepDirection = convexSetSupportPoint(points1, sweepDirection) - convexSetSupportPoint(points1, -sweepDirection);

        std::sort(supportPoints1.begin(), supportPoints1.end(), sweepSortingFunction(sweepDirection));
        std::sort(supportPoints2.begin(), supportPoints2.end(), sweepSortingFunction(sweepDirection));

        const glm::vec3 origin = supportPoints1[0];

        supportPoints1 = translatePoints(supportPoints1, -origin);
        supportPoints2 = translatePoints(supportPoints2, -origin);

        const glm::vec3 orthogonalDirection = glm::cross(supportDirection, sweepDirection);

        const auto [firstHalf1, secondHalf1] = halvePolygon(supportPoints1, orthogonalDirection);
        const auto [firstHalf2, secondHalf2] = halvePolygon(supportPoints2, orthogonalDirection);

        std::vector<glm::vec3> intersectionPoints;

        // TODO May be possible to reduce the number of calls here
        addInnerPoints(intersectionPoints, firstHalf1, supportPoints2, sweepDirection, orthogonalDirection);
        addInnerPoints(intersectionPoints, secondHalf1, supportPoints2, sweepDirection, -orthogonalDirection);
        addInnerPoints(intersectionPoints, firstHalf2, supportPoints1, sweepDirection, orthogonalDirection);
        addInnerPoints(intersectionPoints, secondHalf2, supportPoints1, sweepDirection, -orthogonalDirection);
        addIntersections(intersectionPoints, firstHalf1, firstHalf2, sweepDirection, orthogonalDirection);
        addIntersections(intersectionPoints, firstHalf1, secondHalf2, sweepDirection, orthogonalDirection);
        addIntersections(intersectionPoints, secondHalf1, firstHalf2, sweepDirection, -orthogonalDirection);
        addIntersections(intersectionPoints, secondHalf1, secondHalf2, sweepDirection, -orthogonalDirection);

        // Failsafe : should not happen
        if (intersectionPoints.size() == 0) {
            return {supportPoints1[0] + origin};
        }

        return translatePoints(intersectionPoints, origin);
    }

    // Volume

    float tetrahedronVolume(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
        return std::abs(glm::dot(a - d, glm::cross(b - d, c - d))) / 6.0f;
    }

    float sphereVolume(float radius) {
        return 4.0f / 3.0f * glm::pi<float>() * radius * radius * radius;
    }

    float boxVolume(const glm::vec3& halfLengths) {
        return 8.0f * (halfLengths.x * halfLengths.y * halfLengths.z);
    }

    float convexSetVolume(const std::vector<glm::vec3>& convexSet) {
        float volume = 0.0f;
        for (auto [a, b, c, d] : convexSetTetrahedralization(convexSet)) {
            volume += tetrahedronVolume(a, b, c, d);
        }
        return volume;
    }

    float cylinderVolume(float radius, float halfHeight) {
        return 2.0f * halfHeight * radius * radius * glm::pi<float>();
    }

    float capsuleVolume(float radius, float halfHeight) {
        return cylinderVolume(radius, halfHeight) + sphereVolume(radius);
    }

    // Inertia

    glm::mat3 shiftInertiaTensor(const glm::mat3& inertiaTensor, const glm::vec3& shift) {
        const glm::vec3 sq{shift * shift};
        const float xy{-shift.x * shift.y};
        const float xz{-shift.x * shift.z};
        const float yz{-shift.y * shift.z};
        return inertiaTensor + glm::mat3(
            sq.y + sq.z, xy, xz,
            xy, sq.x + sq.y, yz,
            xz, yz, sq.x + sq.y
        );
    }

    glm::mat3 rotateInertiaTensor(const glm::mat3& inertiaTensor, const glm::quat& rotation) {
        glm::mat3 rotationMatrix = glm::mat3_cast(rotation);
        return glm::transpose(rotationMatrix) * inertiaTensor * rotationMatrix;
    }

    glm::mat3 tetrahedronInertiaTensor(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
        const float v = 0.05f * tetrahedronVolume(a, b, c, d);
        const glm::vec3 s2 = c + d;
        const glm::vec3 s3 = s2 + c;
        const glm::vec3 s4 = s3 + d;
        const glm::vec3 s = 2.0f * (a * s4 + b * s3 + c * s2 + d);

        const float ap = -s4.y * s4.z;
        const float bp = -s4.x * s4.z;
        const float cp = -s4.x * s4.y;
        return v * glm::mat3(
            s.y + s.z, bp, cp,
            bp, s.x + s.z, ap,
            cp, ap, s.x + s.y
        );
    }

    glm::mat3 sphereInertiaTensor(float radius) {
        return glm::mat3(2.0f / 5.0f * radius * radius);
    }

    glm::mat3 boxInertiaTensor(const glm::vec3& halfLengths) {
        const glm::vec3 sqLength = halfLengths * halfLengths;
        return glm::diagonal3x3(glm::vec3(sqLength.y + sqLength.z, sqLength.x + sqLength.z, sqLength.x + sqLength.y) / 3.0f);
    }

    glm::mat3 cylinderInertiaTensor(float radius, float halfHeight) {
        const float radius2 = radius * radius;
        const float a = (3.0f * radius2 + 4.0f * halfHeight * halfHeight) / 6.0f;
        return glm::diagonal3x3(glm::vec3(a, radius2, a) * 0.5f);
    }

    glm::mat3 capsuleInertiaTensor(float radius, float halfHeight) {
        const float height = 2.0f * halfHeight;
        const float radius2 = radius * radius;

        const float cylinderVolume = glm::pi<float>() * height * radius2;
        const float hemispheresVolume = 2.0f * glm::pi<float>() * radius * radius2 / 3.0f;
        const float cylinderPart = cylinderVolume / (cylinderVolume + 2.0f * hemispheresVolume);
        const float hemispheresPart = 1.0f - cylinderPart;

        const float a = 0.5f * radius2 * cylinderPart;
        const float b = 0.5f * a + cylinderPart * height * height / 12.0f;
        const float c = 2.0f * hemispheresPart * radius2 / 5.0f;
        const float d = c + hemispheresPart * (halfHeight * halfHeight + 3.0f / 8.0f * height * radius);
        return glm::diagonal3x3(glm::vec3(b + 2.0f * d, a + 2.0f * c, b + 2.0f * d));
    }

    glm::mat3 convexSetInertiaTensor(const std::vector<glm::vec3>& convexSet) {
        glm::vec3 barycenter = convexSetAverage(convexSet);
        glm::mat3 inertiaTensor = glm::mat3(0.0f);
        for (const auto[a, b, c, d] : convexSetTetrahedralization(convexSet)) {
            inertiaTensor += shiftInertiaTensor(tetrahedronInertiaTensor(a, b, c, d), barycenter);
        }
        return inertiaTensor;
    }
}