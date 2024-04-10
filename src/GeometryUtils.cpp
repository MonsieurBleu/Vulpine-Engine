#include "GeometryUtils.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <deque>
#include <stdexcept>

IntersectionInfo& IntersectionInfo::invert() {
    normal = -normal;
    return *this;
}

std::optional<IntersectionInfo> getSpheresIntersection(const glm::vec3& center1, float radius1, const glm::vec3& center2, float radius2) {
    const float radiiSum{radius1 + radius2};
    const float distanceSquared = glm::distance2(center1, center2);
    if (distanceSquared <= radiiSum * radiiSum) {
        IntersectionInfo intersectionInfo;
        intersectionInfo.penetration = radiiSum - std::sqrt(distanceSquared);
        intersectionInfo.normal = glm::normalize(center1 - center2);
        intersectionInfo.collisionPoint = center2 + intersectionInfo.normal * (radius2 - 0.5f * intersectionInfo.penetration);
        return std::make_optional(intersectionInfo);
    }
    else {
        return std::nullopt;
    }
}

std::optional<IntersectionInfo> sat(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, const std::vector<glm::vec3>& axes) {
    IntersectionInfo intersectionInfo;
    intersectionInfo.penetration = std::numeric_limits<float>::infinity();

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
        if (overlap <= 0.0f) {
            return std::nullopt;
        } else if (overlap < intersectionInfo.penetration) {
            intersectionInfo.penetration = overlap;
            if (min1 + max1 > min2 + max2) {
                intersectionInfo.normal = axis;
            }
            else {
                intersectionInfo.normal = -axis;
            }
            intersectionInfo.collisionPoint = 0.5f * (std::min(max1, max2) + std::max(min1, min2)) * axis;
        }
    }

    return std::make_optional(intersectionInfo);
}

static glm::vec3 gjkSupport(const std::vector<glm::vec3>& vertices, const glm::vec3& direction) {
    float maxDot = -std::numeric_limits<float>::infinity();
    glm::vec3 supportPoint;
    for (const glm::vec3& vertex : vertices) {
        float dot = glm::dot(vertex, direction);
        if (dot > maxDot) {
            maxDot = dot;
            supportPoint = vertex;
        }
    }
    return supportPoint;
}

static bool gjkNearestSimplexLine(std::deque<glm::vec3>& simplex, glm::vec3& direction) {
    glm::vec3 a = simplex[0];
    glm::vec3 b = simplex[1];
    glm::vec3 ab = b - a;
    glm::vec3 ao = -a;

    if (glm::dot(ab, ao) > 0.0f) {
        direction = glm::cross(ab, glm::cross(ao, ab));
    }
    else {
        simplex = {a};
        direction = ao;
    }

    return false;
}

static bool gjkNearestSimplexTriangle(std::deque<glm::vec3>& simplex, glm::vec3& direction) {
    glm::vec3 a = simplex[0];
    glm::vec3 b = simplex[1];
    glm::vec3 c = simplex[2];
    glm::vec3 ab = b - a;
    glm::vec3 ac = c - a;
    glm::vec3 ao = -a;

    glm::vec3 abc = glm::cross(ab, ac);
    
    if (glm::dot(glm::cross(abc, ac), ao) > 0.0f) {
		if (glm::dot(ac, ao) > 0.0f) {
			simplex = {a, c};
			direction = glm::cross(cross(ac, ao), ac);
		}

		else {
            simplex = {a, b};
			return gjkNearestSimplexLine(simplex, direction);
		}
	}
	else {
		if (glm::dot(glm::cross(ab, abc), ao) > 0.0f) {
			simplex = {a, b};
			return gjkNearestSimplexLine(simplex, direction);
		}

		else {
			if (glm::dot(abc, ao) > 0.0f) {
				direction = abc;
			}

			else {
				simplex = {a, c, b};
				direction = -abc;
			}
		}
	}

    return false;
}

static bool gjkNearestSimplexTetrahedron(std::deque<glm::vec3>& simplex, glm::vec3& direction) {
    glm::vec3 a = simplex[0];
	glm::vec3 b = simplex[1];
	glm::vec3 c = simplex[2];
	glm::vec3 d = simplex[3];

	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ad = d - a;
	glm::vec3 ao = -a;
 
	glm::vec3 abc = glm::cross(ab, ac);
	glm::vec3 acd = glm::cross(ac, ad);
	glm::vec3 adb = glm::cross(ad, ab);
 
	if (glm::dot(abc, ao) > 0.0f) {
        simplex = {a, b, c};
		return gjkNearestSimplexTriangle(simplex, direction);
	}
		
	if (glm::dot(acd, ao) > 0.0f) {
        simplex = {a, c, d};
		return gjkNearestSimplexTriangle(simplex, direction);
	}
 
	if (glm::dot(adb, ao) > 0.0f) {
        simplex = {a, d, b};
		return gjkNearestSimplexTriangle(simplex, direction);
	}

    return true;
}

static bool gjkNearestSimplex(std::deque<glm::vec3>& nearestSimplex, glm::vec3& direction) {
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

static std::pair<std::vector<glm::vec4>, std::size_t> gjkGetFaceNormals(const std::deque<glm::vec3>& polytope, const std::vector<std::size_t>& faces) {
	std::vector<glm::vec4> normals;
	std::size_t minTriangle = 0;
	float minDistance = std::numeric_limits<float>::infinity();

	for (std::size_t i = 0; i < faces.size(); i += 3) {
		glm::vec3 a = polytope[faces[i]];
		glm::vec3 b = polytope[faces[i + 1]];
		glm::vec3 c = polytope[faces[i + 2]];

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

static IntersectionInfo gjkEpa(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2, std::deque<glm::vec3>& simplex) {
    std::vector<std::size_t> faces = {
		0, 1, 2,
		0, 3, 1,
		0, 2, 3,
		1, 3, 2
	};

	auto [normals, minFace] = gjkGetFaceNormals(simplex, faces);

    glm::vec3 minNormal;
	float minDistance = std::numeric_limits<float>::max();
	
	while (minDistance == std::numeric_limits<float>::max()) {
		minNormal = glm::vec3(normals[minFace]);
		minDistance = normals[minFace].w;
 
		glm::vec3 support = gjkSupport(points1, minNormal) - gjkSupport(points2, -minNormal);
 
		if (std::abs(glm::dot(minNormal, support) - minDistance) > 0.001f) {
			minDistance = std::numeric_limits<float>::max();

            std::vector<std::pair<std::size_t, std::size_t>> uniqueEdges;

			for (std::size_t i = 0; i < normals.size(); ++i) {
				if (glm::dot(glm::vec3(normals[i]), support) > 0.0f) {
					size_t f = i * 3;

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
			 
			simplex.push_back(support);

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
	}

    IntersectionInfo intersectionInfo;
    intersectionInfo.normal = -minNormal;
    intersectionInfo.penetration = minDistance + 0.001f;
 
	return intersectionInfo;
}

std::optional<IntersectionInfo> gjk(const std::vector<glm::vec3>& points1, const std::vector<glm::vec3>& points2) {
    glm::vec3 supportPoint = gjkSupport(points1, glm::vec3(1.0f, 0.0f, 0.0f)) - gjkSupport(points2, glm::vec3(-1.0f, 0.0f, 0.0f));
    std::deque<glm::vec3> simplex = {supportPoint};
    glm::vec3 direction = -supportPoint;
    
    while (true) {
        supportPoint = gjkSupport(points1, direction) - gjkSupport(points2, -direction);
        if (glm::dot(supportPoint, direction) < 0.0f) {
            return std::nullopt;
        }
        
        simplex.push_front(supportPoint);

        if (gjkNearestSimplex(simplex, direction)) {
            return std::make_optional(gjkEpa(points1, points2, simplex));
        }
    }
}

glm::vec3 closestPointOnTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
    const glm::vec3 ab = b - a;
    const glm::vec3 ac = c - a;
    const glm::vec3 ap = p - a;

    const float acac = glm::dot(ac, ac);
    const float acap = glm::dot(ac, ap);
    const float abab = glm::dot(ab, ab);
    const float abac = glm::dot(ab, ac);
    const float abap = glm::dot(ab, ap);

    const float d = 1.0f / (acac * abab - abac * abac);
    const float u = std::clamp((abab * acap - abac * abap) * d, 0.0f, 1.0f);
    const float v = std::clamp((acac * abap - abac * acap) * d, 0.0f, 1.0f);

    return a + u * ac + v * ab;
}

namespace geometry {

    RaycastInfo::RaycastInfo(
        const glm::vec3& intersectionPoint,
        const glm::vec3& normal,
        float timePoint
    ) :
        doesIntersect(true),
        intersectionPoint(intersectionPoint),
        normal(normal),
        timePoint(timePoint)
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

    float RaycastInfo::getTimePoint() const {
        if (!doesIntersect) {
            throw std::runtime_error("Cannot get the time point of a non-intersecting ray cast");
        }
        return timePoint;
    }

    RaycastInfo RaycastInfo::none() {
        return RaycastInfo();
    }

    RaycastInfo raycastSphere(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& spherePosition, float sphereRadius) {
        return RaycastInfo::none();
    }

    glm::vec3 convexSetSupportPoint(const std::vector<glm::vec3>& convexSet, const glm::vec3& direction) {
        float maxDot = -std::numeric_limits<float>::infinity();
        glm::vec3 supportPoint;
        for (const glm::vec3& point : convexSet) {
            float dot = glm::dot(point, direction);
            if (dot > maxDot) {
                maxDot = dot;
                supportPoint = point;
            }
        }
        return supportPoint;
    }

    std::vector<std::array<glm::vec3, 4>> convexSetTetrahedralization(const std::vector<glm::vec3>& convexSet) {
        if (convexSet.size() < 4) {
            throw std::runtime_error("Convex set of size less than 4 (" + std::to_string(convexSet.size()) + ") cannot be broken down into tetrahedra");
        }

        std::vector<std::array<glm::vec3, 4>> tetrahedra;
        /*for (std::size_t i = 1; i < convexSet.size() - 2; ++i) {
            for (std::size_t j = i + 1; j < convexSet.size() - 1; ++j) {
                for (std::size_t k = j + 1; k < convexSet.size(); ++k) {
                    tetrahedra.push_back({convexSet[0], convexSet[i], convexSet[j], convexSet[k]});
                }
            }
        }*/

        return tetrahedra;
    }

    glm::vec3 uniformConvexSetBarycenter(const std::vector<glm::vec3>& convexSet) {
        glm::vec3 barycenter(0.0f);

        for (const glm::vec3& point : convexSet) {
            barycenter += point;
        }

        return barycenter / static_cast<float>(convexSet.size());
    }

    glm::vec3 weightedConvexSetBarycenter(const std::vector<glm::vec3>& convexSet, const std::vector<float>& pointWeights) {
        if (convexSet.size() != pointWeights.size()) {
            throw std::runtime_error("Cannot calculate the convex barycenter of unequal sizes of position and mass vectors");
        }

        float totalMass = 0.0f;
        glm::vec3 barycenter(0.0f);

        for (std::size_t i = 0; i < convexSet.size(); ++i) {
            totalMass += pointWeights[i];
            barycenter += convexSet[i] * pointWeights[i];
        }

        return barycenter / totalMass;
    }

    float tetrahedronVolume(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d) {
        return std::abs(glm::dot(a - d, glm::cross(b - d, c - d))) / 6.0f;
    }

    float convexSetVolume(const std::vector<glm::vec3>& convexSet) {
        float volume = 0.0f;
        for (auto [a, b, c, d] : convexSetTetrahedralization(convexSet)) {
            volume += tetrahedronVolume(a, b, c, d);
        }
        return volume;
    }

    std::vector<glm::vec3> getBoxVertices(const glm::vec3& halfLengths, const glm::quat& rotation) {
        glm::vec3 xAxis(halfLengths.x, 0.0f, 0.0f);
        glm::vec3 yAxis(0.0f, halfLengths.y, 0.0f);
        glm::vec3 zAxis(0.0f, 0.0f, halfLengths.z);
        if (rotation != glm::identity<glm::quat>()) {
            xAxis = rotation * xAxis;
            yAxis = rotation * yAxis;
            zAxis = rotation * zAxis;
        }
        std::vector<glm::vec3> boxVertices(8);
        boxVertices[0] = -xAxis - yAxis - zAxis;
        boxVertices[1] = -xAxis - yAxis + zAxis;
        boxVertices[2] = -xAxis + yAxis - zAxis;
        boxVertices[3] = -xAxis + yAxis + zAxis;
        boxVertices[4] = xAxis - yAxis - zAxis;
        boxVertices[5] = xAxis - yAxis + zAxis;
        boxVertices[6] = xAxis + yAxis - zAxis;
        boxVertices[7] = xAxis + yAxis + zAxis;
        return boxVertices;
    }

    // todo transforPoints
    std::vector<glm::vec3> translatePoints(const std::vector<glm::vec3>& points, const glm::vec3& translation) {
        std::vector<glm::vec3> translatedPoints(points.size());
        for (std::size_t i = 0; i < points.size(); ++i) {
            translatedPoints[i] = points[i] + translation;
        }
        return translatedPoints;
    }

    std::pair<glm::vec3, glm::vec3> convexSetBoundingBox(const std::vector<glm::vec3>& convexSet) {
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

}