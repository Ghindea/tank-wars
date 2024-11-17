#pragma once

#include "components/simple_scene.h"
#include "tankWars/movements.h"
#include "tankWars/elementConstructor.h"

namespace m1
{
	class tankWars : public gfxc::SimpleScene
	{
	public:
		tankWars();
		~tankWars();

        struct projectile
        {
            glm::vec3 position;
            glm::vec3 velocity;
			bool active;
            int index;

			projectile() : position(glm::vec3(0)), velocity(glm::vec3(0)) {}
			projectile(glm::vec3 pos, glm::vec3 vel, bool a, int i) : position(pos), velocity(vel), active(a), index(i) {}
        };

		struct player
		{
			glm::mat3 matrix, canonMatrix;
			std::vector<glm::vec3> trajectory;
			std::vector<projectile> ammo;
			float x, y;
			float canonAngle;
			float tankAngle;
			float health;
			int index, colorIndex;

			Mesh* healthBar;
			Mesh* tank;
			
            glm::vec3 velocity;
            float scaleFactor;
			bool active = true;
		}p1, p2;

		void Init() override;
        glm::mat3 bodyMovement(glm::mat3* matrix, float* x, float* y, float* angle);
        glm::mat3 canonMovement(glm::mat3* matrix, float* angle, glm::mat3* canonMatrix);
        void tankWars::renderBullets(float x, float y, float angle, std::vector<projectile>* ammo, float deltaTimeSeconds, bool player, glm::mat3 canonMatrix);
        void flyTank(player* p, float deltaTimeSeconds, std::string id);
        void shakeScene(float deltaTimeSeconds);
        void resetGame();


        float bulletSpeed = 500.0f;
		float shakeTime, shakeDuration, shakeIntensity;
		bool shake;

        Mesh* terrain;
        Mesh* sky;
		glm::vec3 skyColor;
		std::vector<glm::vec3> playerColors;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;


        element constructor;
        bool lineToggle;
		bool soundToggle;
        int ammoLimitToggle;
		bool gameIsPaused;


		glm::mat3 p1_matrix, p2_matrix;
        std::vector<glm::vec3> p1_trajectory, p2_trajectory;
        std::vector<projectile> p1_ammo, p2_ammo;
        float p1_x_axis, p1_y_axis;
		float p2_x_axis, p2_y_axis;
        float p1_canonAngle, p2_canonAngle;
        float p1_tankAngle, p2_tankAngle;
		float p1_health, p2_health;
        int indext1, indext2;



		float tankLength;
        glm::vec3 tankSpawnPoint;

        struct winSize {
			int width, height;
        }res;
	};
}