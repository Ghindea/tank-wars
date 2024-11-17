#include "tankWars/elementConstructor.h"

#include <vector>
#include <iostream>

using namespace std;
using namespace m1;

element::element()
{
	wave.a1 = 113.0f;
    wave.f1 = 0.009f;
    wave.a2 = 45.0f;
    wave.f2 = 0.01f;
    wave.a3 = 180.0f;
    wave.f3 = 0.0006f;

    xStep = 2.0f;
	raiseFactor = 300.0f;
}

element::~element()
{
}

std::vector<float> element::getHeightMap() {
	return heightMap;
}

float element::getRaiseFactor() {
	return raiseFactor;
}

float element::getXStep() {
	return xStep;
}

void element::addCircle(std::vector<VertexFormat>& vertices, std::vector<unsigned int>& indices, glm::vec3 center, float radius, glm::vec3 color, int numSegments) {
    unsigned int startIndex = vertices.size();

    // Add center vertex for the circle
    vertices.push_back(VertexFormat(center, color));

    // Generate circle vertices
    for (int i = 0; i <= numSegments; i++) {
        float angle = 2.0f * 3.1415926f * float(i) / float(numSegments);
        float x = radius * cosf(angle);
        float y = radius * sinf(angle);
        vertices.push_back(VertexFormat(center + glm::vec3(x, y, 0.0f), color));
    }

    // Generate indices for the circle using GL_TRIANGLE_FAN
    for (int i = 0; i <= numSegments; i++) {
        indices.push_back(startIndex);      // Center vertex
        indices.push_back(startIndex + i);  // Current edge vertex
        indices.push_back(startIndex + i + 1); // Next edge vertex
    }
}

Mesh* element::createProjectile(const std::string& name, glm::vec3 center, float radius, glm::vec3 color) {

	glm::vec3 middle = center;

	std::vector<VertexFormat> vertices = std::vector<VertexFormat>();
	std::vector<unsigned int> indices = std::vector<unsigned int>();

	Mesh* projectile = new Mesh(name);

	int numSegments = 25; // Adjust for smoothness

	addCircle(vertices, indices, middle, radius, color, numSegments);

	projectile->SetDrawMode(GL_TRIANGLE_FAN);
	projectile->InitFromData(vertices, indices);

	return projectile;
}

Mesh* element::createTank(const std::string& name, glm::vec3 bottomMiddle, glm::vec3 color, float length, bool fill = false)
{
    float darkening_factor = 0.5f;
    glm::vec3 bottom = bottomMiddle;

    std::vector<VertexFormat> vertices = {
        VertexFormat(bottom + glm::vec3(length,  length / 4, 0), color),                                   // 0
        VertexFormat(bottom + glm::vec3(-length, length / 4, 0), color),                                   // 1
        VertexFormat(bottom + glm::vec3(-length + length / 4, length / 4 + length / 2.66, 0), color),	   // 2
        VertexFormat(bottom + glm::vec3(length - length / 4, length / 4 + length / 2.66, 0), color),       // 3
        VertexFormat(bottom + glm::vec3(length,  length / 4, 0), color),                                   // 0
        VertexFormat(bottom + glm::vec3(length - length / 4, length / 4, 0), color * darkening_factor),    // 4
        VertexFormat(bottom + glm::vec3(length / 2, 0, 0), color * darkening_factor),                      // 5
        VertexFormat(bottom + glm::vec3(-length / 2, 0, 0), color * darkening_factor),                     // 6
        VertexFormat(bottom + glm::vec3(-length + length / 4, length / 4, 0), color * darkening_factor),   // 7
    };

    Mesh* tank = new Mesh(name);
    std::vector<unsigned int> indices = {
        0, 1, 3,
        1, 2, 3,
        5, 6, 8,
        6, 7, 8,
    };

    // Define circle properties
    glm::vec3 circleCenter = bottom + glm::vec3(0, length / 4 + length / 2.66, 0); // Position the circle above the bottom
    float circleRadius = length / 2.66;
    int numSegments = 25; // Adjust for smoothness

    // Add the circle vertices and indices
    addCircle(vertices, indices, circleCenter, circleRadius, color, numSegments);

    if (!fill) {
        tank->SetDrawMode(GL_LINE_LOOP);
    }
    else {
        tank->SetDrawMode(GL_TRIANGLES);
    }

    tank->InitFromData(vertices, indices);
    return tank;
}

Mesh* element::createTerrain(const std::string& name, glm::vec3 color, float length) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

	heightMap.clear();

    // Generate curve points
    for (float x = 0.0f; x <= length; x += xStep) {
		heightMap.push_back(
            wave.a1 * sin(wave.f1 * x) + 
            wave.a2 * sin(wave.f2 * x) + 
            wave.a3 * sin(wave.f3 * x) + 
			raiseFactor); // Raise the curve
    }

    float x = 0, y = 0;
	for (int i = 0; i < heightMap.size(); i++) {
		x = i * xStep;
		y = heightMap[i];

		vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color)); 		// top vertex
		vertices.push_back(VertexFormat(glm::vec3(x, 45, 0), glm::vec3(0.49f, 0.38f, 0.31f))); 		// bottom vertex
	}

	for (int i = 0; i < vertices.size()/2 - 1; i++) {
        int topCurrent = i * 2;
        int bottomCurrent = topCurrent + 1;
        int topNext = topCurrent + 2;
        int bottomNext = bottomCurrent + 2;

        // First triangle
        indices.push_back(topCurrent);
        indices.push_back(bottomCurrent);
        indices.push_back(topNext);

        // Second triangle
        indices.push_back(topNext);
        indices.push_back(bottomNext);
        indices.push_back(bottomCurrent);
	}

    Mesh* terrain = new Mesh(name);
    terrain->SetDrawMode(GL_TRIANGLES);
    terrain->InitFromData(vertices, indices);
    return terrain;
}
Mesh* element::updateTerrain(const std::string& name, glm::vec3 color, float length) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    float x = 0, y = 0;
    for (int i = 0; i < heightMap.size(); i++) {
        x = i * xStep;
        y = heightMap[i];

        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color)); 		// top vertex
        vertices.push_back(VertexFormat(glm::vec3(x, 45, 0), glm::vec3(0.49f, 0.38f, 0.31f))); 		// bottom vertex
    }

    for (int i = 0; i < vertices.size()/2 - 1; i++) {
        int topCurrent = i * 2;
        int bottomCurrent = topCurrent + 1;
        int topNext = topCurrent + 2;
        int bottomNext = topCurrent + 3;

        // First triangle
        indices.push_back(topCurrent);
        indices.push_back(bottomCurrent);
        indices.push_back(topNext);

        // Second triangle
        indices.push_back(topNext);
        indices.push_back(bottomCurrent);
        indices.push_back(bottomNext);
    }

    Mesh* terrain = new Mesh(name);
    terrain->SetDrawMode(GL_TRIANGLES);
    terrain->InitFromData(vertices, indices);
    return terrain;
}
Mesh* element::deformTerrain(float x, float radius) {

	int x_index = (int)(x / xStep);
	int radius_index = (int)(radius / xStep);

	int start = std::max(0, x_index - radius_index);
	int end = std::min((int)heightMap.size() - 1, x_index + radius_index);

	for (int i = start; i <= end; i++) {
		float distance = abs(x - i * xStep);

		if (distance <= radius) {
			//float  deformation = sqrt(radius * radius - distance * distance);
			//heightMap[i] -= std::max(0.0f, heightMap[i] - deformation);
			float deformation = (cos(distance / radius * 3.14159) + 1) / 2 * maxDeformation;
			heightMap[i] -= deformation;
			if (heightMap[i] < 45) {
				heightMap[i] = 45;
			}
        }
	}

	return updateTerrain("terrain", glm::vec3(0, 1, 0), (float)window->GetResolution().x);
}

Mesh* element::createCanon(const std::string& name, glm::vec3 bottomMiddle, glm::vec3 color, float length) {
    float darkening_factor = 0.5f;
    glm::vec3 bottom = bottomMiddle;

    std::vector<VertexFormat> vertices = {
        VertexFormat(bottom + glm::vec3(0, -length / 16, 0), color),                                   // 0
        VertexFormat(bottom + glm::vec3(0,  length / 16, 0), color),                                   // 1
        VertexFormat(bottom + glm::vec3(length - length/4,  length / 16, 0), color),                   // 2
        VertexFormat(bottom + glm::vec3(length - length/4, -length / 16, 0), color),                   // 3
    };

    Mesh* tank = new Mesh(name);
    std::vector<unsigned int> indices = {
        0, 1, 2,
		2, 3, 0
    };

    tank->SetDrawMode(GL_TRIANGLES);
    tank->InitFromData(vertices, indices);
    return tank;
}
Mesh* element::createBedrock(const std::string& name, glm::vec3 color, float length) {
	std::vector<VertexFormat> vertices = {
		VertexFormat(glm::vec3(0, 0, 0), color),
		VertexFormat(glm::vec3(length, 0, 0), color),
		VertexFormat(glm::vec3(length, 45, 0), color),
		VertexFormat(glm::vec3(0, 45, 0), color)
	};

	std::vector<unsigned int> indices = {
		0, 1, 2,
		2, 3, 0
	};

	Mesh* bedrock = new Mesh(name);
	bedrock->SetDrawMode(GL_TRIANGLES);
	bedrock->InitFromData(vertices, indices);
	return bedrock;
}
Mesh* element::createSky(const std::string& name, glm::vec3 color) {
    float darkening_factor = 0.6f;

    std::vector<VertexFormat> vertices = {
        VertexFormat(glm::vec3(0, 0, 0), color),                                   // 0
        VertexFormat(glm::vec3(0, window->GetResolution().y, 0), color * darkening_factor),                                  // 1
        VertexFormat(glm::vec3(window->GetResolution().x, window->GetResolution().y, 0), color * darkening_factor),          // 2
        VertexFormat(glm::vec3(window->GetResolution().x, 0, 0), color),           // 3
    };

	std::vector<unsigned int> indices = {
		0, 1, 2,
		2, 3, 0
	};

	Mesh* sky = new Mesh(name);
	sky->SetDrawMode(GL_TRIANGLES);
	sky->InitFromData(vertices, indices);
	return sky;
}
Mesh* element::createLine(const std::string& name, glm::vec3 color) {
    std::vector<VertexFormat> lineVertices = {
        VertexFormat(glm::vec3(0, 0, 0), color),
        VertexFormat(glm::vec3(1, 0, 0), color)
    };
    std::vector<unsigned int> lineIndices = { 0, 1 };

    Mesh* line = new Mesh("line");
    line->SetDrawMode(GL_LINES);
    line->InitFromData(lineVertices, lineIndices);
    
	return line;
}
Mesh* element::createLineSegment(const std::string& name, glm::vec3 start, glm::vec3 end, glm::vec3 color) {
    std::vector<VertexFormat> lineVertices = {
        VertexFormat(start, color),
        VertexFormat(end, color)
    };
    std::vector<unsigned int> lineIndices = { 0, 1 };

    Mesh* line = new Mesh("line");
    line->SetDrawMode(GL_LINES);
    line->InitFromData(lineVertices, lineIndices);

    return line;
}
Mesh* element::createHealthBar(const std::string& name, glm::vec3 bottomMiddle, glm::vec3 color, float tankLength, float length) {
	std::vector<VertexFormat> vertices = {
		VertexFormat(bottomMiddle + glm::vec3(-tankLength + tankLength/4, tankLength + tankLength/2, 0), color),                         // 0
		VertexFormat(bottomMiddle + glm::vec3(-tankLength + tankLength / 4, tankLength + tankLength / 2 + tankLength / 8, 0), color),    // 1
		VertexFormat(bottomMiddle + glm::vec3(-tankLength + tankLength/4 + length, tankLength + tankLength/2 + tankLength/8, 0), color), // 2
		VertexFormat(bottomMiddle + glm::vec3(-tankLength + tankLength / 4 + length, tankLength + tankLength / 2, 0), color), 			 // 3
		VertexFormat(bottomMiddle + glm::vec3(-tankLength + tankLength / 4 + length, tankLength + tankLength / 2, 0), glm::vec3(1, 0, 0)), 			 // 4
		VertexFormat(bottomMiddle + glm::vec3(-tankLength + tankLength/4 + length, tankLength + tankLength/2 + tankLength/8, 0), glm::vec3(1, 0, 0)), // 5
		VertexFormat(bottomMiddle + glm::vec3( tankLength - tankLength / 4, tankLength + tankLength / 2 + tankLength / 8, 0), glm::vec3(1, 0, 0)),    // 6
		VertexFormat(bottomMiddle + glm::vec3(tankLength - tankLength / 4, tankLength + tankLength / 2, 0), glm::vec3(1, 0, 0)),    // 7
    };

	std::vector<unsigned int> indices = {
		0, 1, 2,
		2, 3, 0,
		4, 5, 6,
		6, 7, 4
	};

	Mesh* healthBar = new Mesh(name);
	healthBar->SetDrawMode(GL_TRIANGLES);
	healthBar->InitFromData(vertices, indices);
	return healthBar;
}
