#pragma once

#include "components/simple_scene.h"
#include "core/gpu/mesh.h"
#include "utils/glm_utils.h"

namespace m1
{
    class element : public gfxc::SimpleScene
    {
    public:
        element();
        ~element();

        void addCircle(std::vector<VertexFormat>&, std::vector<unsigned int>&, glm::vec3, float, glm::vec3, int);
        Mesh* createTank(const std::string& name, glm::vec3 bottomMiddle, glm::vec3 color, float length, bool fill);
        Mesh* createCanon(const std::string& name, glm::vec3 bottomMiddle, glm::vec3 color, float length);
        Mesh* createProjectile(const std::string& name, glm::vec3 center, float radius, glm::vec3 color);
        Mesh* createHealthBar(const std::string& name, glm::vec3 bottomMiddle, glm::vec3 color, float tankLength, float length);

        Mesh* createTerrain(const std::string& name, glm::vec3 color, float resolution);
        Mesh* updateTerrain(const std::string& name, glm::vec3 color, float length);
        Mesh* deformTerrain(float x, float radius);
        Mesh* createBedrock(const std::string& name, glm::vec3 color, float length);

        Mesh* createLine(const std::string& name, glm::vec3 color);
        Mesh* createLineSegment(const std::string& name, glm::vec3 start, glm::vec3 end, glm::vec3 color);
        Mesh* createSky(const std::string& name, glm::vec3 color);

		std::vector<float> getHeightMap();
		float getRaiseFactor();
		float getXStep();

    private:
        struct sineWave
        {
            float a1, a2, a3;
			float f1, f2, f3;
        }wave;
        float xStep;
        float raiseFactor;
        std::vector<float> heightMap;

		float maxDeformation = 25.0f;

        
    };
}
