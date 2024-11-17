#include "tankWars/tankWars.h"

#include <vector>
#include <iostream>
#include <windows.h>
#include <thread>

#pragma comment(lib, "winmm.lib")


using namespace std;
using namespace m1;

tankWars::tankWars()
{
	tankLength = 64;
    p1.x = 0;
	p1.y = 0;
	lineToggle   = true;
	gameIsPaused = false;
	soundToggle  = false;
	ammoLimitToggle = 5;
	p1.health = 100.0f;
	p2.health = 100.0f;

	shakeDuration = 2.0f;
	shakeTime = 0.0f;
	shakeIntensity = 10.0f;
	shake = false;
}

tankWars::~tankWars()
{
}

std::vector<char> soundBuffer;

void preloadSound(const char* soundFile) {
	// Open the file
	FILE* file = fopen(soundFile, "rb");
	if (!file) {
		std::cerr << "Failed to open sound file!" << std::endl;
		return;
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file);

	// Read file into buffer
	soundBuffer.resize(fileSize);
	fread(soundBuffer.data(), 1, fileSize, file);
	fclose(file);
}

void playPreloadedSound() {
	if (!soundBuffer.empty()) {
		PlaySound(reinterpret_cast<LPCSTR>(soundBuffer.data()), NULL, SND_MEMORY | SND_ASYNC);
	}
}

glm::mat3 tankWars::bodyMovement(glm::mat3* matrix, float* x, float* y, float *angle) {

	*matrix = glm::mat3(1);

	float xstep = constructor.getXStep();
	int index = (int)(*x / xstep);

	if (index < 0) {
		index = 0;
	}
	if (index + 1 > constructor.getHeightMap().size() - 1) {
		index = constructor.getHeightMap().size() - 2;
	}
	if (*x > res.width - tankLength / 2) {
		*x = res.width - tankLength / 2;
	}
	float Ax = index * xstep;
	float Ay = constructor.getHeightMap().at(index);

	float Bx = (index + 1) * xstep;
	float By = constructor.getHeightMap().at(index + 1);

	float t = (*x - Ax) / (Bx - Ax);
	*y = Ay + t * (By - Ay);

	float Vx = Bx - Ax;
	float Vy = By - Ay;

	*angle = atan2(Vy, Vx);

	*matrix *= movements2D::Translate(*x, *y);
	*matrix *= movements2D::Rotate(*angle);

	return *matrix;
}
glm::mat3 tankWars::canonMovement(glm::mat3* matrix, float* angle, glm::mat3* canonMatrix) {
	*matrix *= movements2D::Translate(0, tankLength / 1.4545f);
	*matrix *= movements2D::Rotate(*angle);
	*canonMatrix = *matrix;
	*matrix *= movements2D::Translate(0, -tankLength / 1.4545f);

	return *matrix;
}
void tankWars::renderBullets(float x, float y, float angle, std::vector<projectile>* ammo, float deltaTimeSeconds, bool player, glm::mat3 canonMatrix) { // true - player 1, false - player 2	
	
	//glm::vec3 canonStart  = glm::vec3(x, y + tankLength / 1.4545f, 0);
	glm::vec3 canonStart = canonMatrix * glm::vec3(0, 0, 1);
	glm::vec3 direction = glm::vec3(cos(angle), sin(angle), 0);
	glm::vec3 position  = canonStart + direction * (tankLength / 1.4545f); // initial position is at canon end

	float magnitude = 350.0f;

	glm::vec3 velocity = direction * bulletSpeed;

	std::vector<glm::vec3> trajectory;

	float timeStep = 0.0352741f;
	float maxSteps = 300;

	// simulate path of projectile
	for (int i = 0; i < maxSteps; i++) {
		
		trajectory.push_back(position);

		position += velocity * timeStep;
		velocity.y -= magnitude * timeStep;

		int index = (int)(position.x / constructor.getXStep());

		if (index >= 0 && index < constructor.getHeightMap().size()) { // stop simulation if projectile hits terrain
			if (position.y <= constructor.getHeightMap().at(index)) {
				break;
			}
		}
		else {
			break;
		}
	}

	glm::mat3 matrix;
	
	// render projectiles
	if (!ammo->empty()) {
		for (auto& p : *ammo) {
			int index = (int)(p.position.x / constructor.getXStep());
			if (p.index >= trajectory.size()) {
				p.active = false;
			}
			else if (index >= 0 && index < constructor.getHeightMap().size()) { // stop simulation if projectile hits terrain
				if (p.position.y <= constructor.getHeightMap().at(index)) {
					p.active = false;
					terrain = constructor.deformTerrain(p.position.x, 190.0f);
				}
			}
			else {
				p.active = false;
			}
			if (p.active) {
				matrix = glm::mat3(1);
		
				p.position += p.velocity * deltaTimeSeconds;
				p.velocity.y -= magnitude * deltaTimeSeconds;
				
				matrix *= movements2D::Translate(p.position.x, p.position.y);
				RenderMesh2D(meshes["projectile"], shaders["VertexColor"], matrix);

				float distanceToEnemy = 0;
				float collisionDistance = tankLength;
				if (player) {
					distanceToEnemy = glm::length(glm::vec3(p2.x, p2.y, 0) - p.position);
					if (distanceToEnemy < collisionDistance) {
						p2.health -= 20.0;
						p2.healthBar = constructor.createHealthBar("healthBar", tankSpawnPoint, glm::vec3(0, 1, 0), tankLength, p2.health / 100 * (tankLength + tankLength / 2));

						if (soundToggle) {
							playPreloadedSound();
						}

						p.active = false;
					}
				}
				else {
					distanceToEnemy = glm::length(glm::vec3(p1.x, p1.y, 0) - p.position);
					if (distanceToEnemy < collisionDistance) {
						p1.health -= 20.0;
						p1.healthBar = constructor.createHealthBar("healthBar", tankSpawnPoint, glm::vec3(0, 1, 0), tankLength, p1.health / 100 * (tankLength + tankLength / 2));
						
						if (soundToggle) {
							playPreloadedSound();
						}

						p.active = false;
					}
				}
			}

		}
		ammo->erase(std::remove_if(ammo->begin(), ammo->end(), [](projectile& p) { return !p.active; }), ammo->end());
	}

	// render trajectory
	for (int i = 0; i < trajectory.size() - 1 && lineToggle; i++) {

		glm::vec3 p1 = trajectory[i];
		glm::vec3 p2 = trajectory[i + 1];

		Mesh* line = constructor.createLineSegment("line", p1, p2, glm::vec3(1, 1, 1));

		RenderMesh2D(line, shaders["VertexColor"], glm::mat3(1));
	}
}
void tankWars::flyTank(player* p, float deltaTimeSeconds, std::string id) {

	std::vector<glm::vec3> trajectory;
	glm::vec3 position = glm::vec3(p->x, p->y, 0);
	glm::vec3 direction = glm::vec3(cos(p->tankAngle), sin(p->tankAngle), 0);
	p->velocity = direction * bulletSpeed;

	float speed = bulletSpeed;
	float timeStep = 0.01f;
	float maxSteps = 300;
	float magnitude = 350.0f;

	p->x += p->velocity.x * deltaTimeSeconds;
	p->y += p->velocity.y * deltaTimeSeconds;
	p->velocity.y -= magnitude * deltaTimeSeconds;
	p->tankAngle -= deltaTimeSeconds;
	p->scaleFactor += deltaTimeSeconds * 10;
	glm::mat3 matrix = glm::mat3(1);
	matrix *= movements2D::Translate(p->x, p->y);
	matrix *= movements2D::Rotate(p->tankAngle * 10);
	matrix *= movements2D::Scale(p->scaleFactor, p->scaleFactor);
	RenderMesh2D(p->tank, shaders["VertexColor"], matrix);
	RenderMesh2D(meshes["canon" + id], shaders["VertexColor"], canonMovement(&matrix, &p->canonAngle, &p->canonMatrix));

	if (p->scaleFactor > 15) {
		resetGame();
	}
}
void tankWars::shakeScene(float deltaTimeSeconds) {

	shakeTime += deltaTimeSeconds;
		if (shakeTime < shakeDuration) {
			float xshake = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2 * shakeIntensity;;
			float yshake = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2 * shakeIntensity;;

			GetSceneCamera()->SetPosition(glm::vec3(xshake, yshake, 50));
		}
		else {
			shakeTime = 0.0f;
			shake = true;
			GetSceneCamera()->SetPosition(glm::vec3(0, 0, 50));
		}
}

void tankWars::resetGame() {
	p1.x = p1.index * constructor.getXStep();
	p1.y = constructor.getHeightMap().at(p1.index);
	p1.canonAngle = 0;

	p2.x = p2.index * constructor.getXStep();
	p2.y = constructor.getHeightMap().at(p2.index);
	p2.canonAngle = 3.14159f;

	p1_ammo.clear();
	p2_ammo.clear();

	p1.health = 100.0f;
	p2.health = 100.0f;

	p1.healthBar = constructor.createHealthBar("healthBar", tankSpawnPoint, glm::vec3(0, 1, 0), tankLength, p1.health / 100 * (tankLength + tankLength / 2));
	p2.healthBar = constructor.createHealthBar("healthBar", tankSpawnPoint, glm::vec3(0, 1, 0), tankLength, p2.health / 100 * (tankLength + tankLength / 2));

	shake = false;
	shakeTime = 0.0f;
	GetSceneCamera()->SetPosition(glm::vec3(0, 0, 50));

	p1.active = true;
	p2.active = true;

	p1.scaleFactor = 1;
	p2.scaleFactor = 1;

	terrain = constructor.createTerrain("terrain", glm::vec3(0, 1, 0), (float)window->GetResolution().x);
}

void tankWars::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

	playerColors = { 
		glm::vec3(1, 0, 0),				// red
		glm::vec3(0.98f, 0.74f, 0.01f), // yellow
		glm::vec3(0.1f, 0.2f, 1),		// blue
		glm::vec3(0, 1, 0),				// green
		glm::vec3(0, 1, 1),				// cyan
		glm::vec3(1, 1, 1),				// white
		glm::vec3(0.77f, 0.01f, 0.98f)	// purple
	};			

	preloadSound("src\\lab_m1\\tankWars\\shot.wav");

	skyColor = glm::vec3(0.2588f, 0.8078f, 0.9608f);

	res.height = resolution.y;
	res.width  = resolution.x;

	terrain = constructor.createTerrain("terrain", glm::vec3(0, 1, 0), (float)resolution.x);
	AddMeshToList(terrain);

	p1.index = 64;
	p2.index = 820;

	p1.colorIndex = 0;
	p2.colorIndex = 2;

	p1.x = p1.index * constructor.getXStep();
	p1.y = constructor.getHeightMap().at(p1.index);
	p1.canonAngle = 0;

	p2.x = p2.index * constructor.getXStep();
	p2.y = constructor.getHeightMap().at(p2.index);
	p2.canonAngle = 3.14159f;

    tankSpawnPoint = glm::vec3(0,0, 0);

	p1_ammo = std::vector<projectile>();
	p2_ammo = std::vector<projectile>();

	p1.scaleFactor = 1;
	p2.scaleFactor = 1;

    p1.tank = constructor.createTank("tank1", tankSpawnPoint, playerColors.at(p1.colorIndex), tankLength, true);
    //AddMeshToList(p1.tank);

	p2.tank = constructor.createTank("tank2", tankSpawnPoint, playerColors.at(p2.colorIndex), tankLength, true);
	//AddMeshToList(p2.tank);

	Mesh* canon1 = constructor.createCanon("canon1", tankSpawnPoint + glm::vec3(0, tankLength / 1.4545f, 0), glm::vec3(0, 0, 0), tankLength);
	AddMeshToList(canon1);

	Mesh* canon2 = constructor.createCanon("canon2", tankSpawnPoint + glm::vec3(0, tankLength / 1.4545f, 0), glm::vec3(0, 0, 0), tankLength);
	AddMeshToList(canon2);

	Mesh* projectile = constructor.createProjectile("projectile", glm::vec3(0, 0, 0), tankLength / 16, glm::vec3(0.003f, 0.43f, 0.13f));
	AddMeshToList(projectile);

	p1.healthBar = constructor.createHealthBar("healthBar", tankSpawnPoint, glm::vec3(0, 1, 0), tankLength, p1.health / 100 * (tankLength + tankLength / 2));
	p2.healthBar = constructor.createHealthBar("healthBar", tankSpawnPoint, glm::vec3(0, 1, 0), tankLength, p2.health / 100 * (tankLength + tankLength / 2));

	Mesh* line = constructor.createLine("line", glm::vec3(1, 1, 1));
	AddMeshToList(line);

	Mesh* bedrock = constructor.createBedrock("bedrock", glm::vec3(0.49f, 0.38f, 0.31f), resolution.x);
	AddMeshToList(bedrock);

	sky = constructor.createSky("sky", skyColor);
	AddMeshToList(sky);
}

void tankWars::FrameStart()
{
	// Clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0.2588f, 0.8078f, 0.9608f, 1);
	//glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// Sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void tankWars::Update(float deltaTimeSeconds)
{
	if (gameIsPaused) {
		//return;
	}
	if (p1.health > 0) {
		RenderMesh2D(p1.tank, shaders["VertexColor"], bodyMovement(&p1_matrix, &p1.x, &p1.y, &p1.tankAngle));
		RenderMesh2D(meshes["canon1"], shaders["VertexColor"], canonMovement(&p1_matrix, &p1.canonAngle, &p1.canonMatrix));
		RenderMesh2D(p1.healthBar, shaders["VertexColor"], bodyMovement(&p1_matrix, &p1.x, &p1.y, &p1.tankAngle));
		renderBullets(p1.x, p1.y, p1.canonAngle + p1.tankAngle, &p1_ammo, deltaTimeSeconds, true, p1.canonMatrix);
	}
	else {
		// TODO: player 2 wins - render text
		if (!shake) {
			shakeScene(deltaTimeSeconds);
		}
		if (p1.active) {
			flyTank(&p1, deltaTimeSeconds, "1");
		}
	}
	if (p2.health > 0) {
		RenderMesh2D(p2.tank, shaders["VertexColor"], bodyMovement(&p2_matrix, &p2.x, &p2.y, &p2.tankAngle));
		RenderMesh2D(meshes["canon2"], shaders["VertexColor"], canonMovement(&p2_matrix, &p2.canonAngle, &p2.canonMatrix));
		RenderMesh2D(p2.healthBar, shaders["VertexColor"], bodyMovement(&p2_matrix, &p2.x, &p2.y, &p2.tankAngle));
		renderBullets(p2.x, p2.y, p2.canonAngle + p2.tankAngle, &p2_ammo, deltaTimeSeconds, false, p2.canonMatrix);
	}
	else {
		// TODO: player 1 wins - render text
		if (!shake) {
			shakeScene(deltaTimeSeconds);
		}
		if (p2.active) {
			flyTank(&p2, deltaTimeSeconds, "2");
		}
	}

	RenderMesh2D(meshes["bedrock"], shaders["VertexColor"], glm::mat3(1));
	RenderMesh2D(terrain, shaders["VertexColor"], glm::mat3(1));
	RenderMesh2D(sky, shaders["VertexColor"], glm::mat3(1));
}

void tankWars::FrameEnd()
{
}

void tankWars::OnInputUpdate(float deltaTime, int mods)
{
	float speed = 300;
	// movement of player one
    if (window->KeyHold(GLFW_KEY_D)) {
		p1.x += speed * deltaTime;
    }
	if (window->KeyHold(GLFW_KEY_A)) {
		p1.x -= speed * deltaTime;
	}

	if (p1.x < -tankSpawnPoint.x + tankLength/2) {
		p1.x = -tankSpawnPoint.x + tankLength/2;
	}
	if (p1.x > res.width - tankLength/2) {
		p1.x = res.width - tankLength/2;
	}
	//player one cannon
	if (window->KeyHold(GLFW_KEY_W)) {
		p1.canonAngle += 2 * deltaTime;
		if (p1.canonAngle > 3.14159f) {
			p1.canonAngle = 3.14159f;
		}
	}
	if (window->KeyHold(GLFW_KEY_S)) {
		p1.canonAngle -= 2 * deltaTime;
		if (p1.canonAngle < 0) {
			p1.canonAngle = 0;
		}
	}

	// movement of player two
	if (window->KeyHold(GLFW_KEY_RIGHT)) {
		p2.x += speed * deltaTime;
	}
	if (window->KeyHold(GLFW_KEY_LEFT)) {
		p2.x -= speed * deltaTime;
	}

	if (p2.x < -tankSpawnPoint.x + tankLength / 2) {
		p2.x = -tankSpawnPoint.x + tankLength / 2;
	}
	if (p2.x > res.width - tankLength / 2) {
		p2.x = res.width - tankLength / 2;
	}

	//player two cannon
	if (window->KeyHold(GLFW_KEY_UP)) {
		p2.canonAngle -= 2 * deltaTime;
		if (p2.canonAngle < 0) {
			p2.canonAngle = 0;
		}
	}
	if (window->KeyHold(GLFW_KEY_DOWN)) {
		p2.canonAngle += 2 * deltaTime;
		if (p2.canonAngle > 3.14159f) {
			p2.canonAngle = 3.14159f;
		}
	}
}

void playSound(const char* soundFile) {
	PlaySound(TEXT(soundFile), NULL, SND_FILENAME | SND_ASYNC);
}



void tankWars::OnKeyPress(int key, int mods)
{
	if (key == GLFW_KEY_SPACE) {
		if (p1_ammo.size() < ammoLimitToggle) {
			p1_ammo.push_back(projectile(
				p1.canonMatrix * glm::vec3(0, 0, 1) + glm::vec3(cos(p1.tankAngle + p1.canonAngle), sin(p1.tankAngle + p1.canonAngle), 0) * (tankLength - tankLength / 4),
				glm::vec3(cos(p1.tankAngle + p1.canonAngle), sin(p1.tankAngle + p1.canonAngle), 0) * bulletSpeed,
				true, 0));
		}
	}
	if (key == GLFW_KEY_ENTER) {
		if (p2_ammo.size() < ammoLimitToggle) {
			p2_ammo.push_back(projectile(
				p2.canonMatrix * glm::vec3(0, 0, 1) + glm::vec3(cos(p2.tankAngle + p2.canonAngle), sin(p2.tankAngle + p2.canonAngle), 0) * (tankLength - tankLength / 4),
				glm::vec3(cos(p2.tankAngle + p2.canonAngle), sin(p2.tankAngle + p2.canonAngle), 0) * bulletSpeed,
				true, 0)); 
		}
	}
	if (key == GLFW_KEY_L) {		// toggle trajectory line rendering
		lineToggle = !lineToggle;
	}
	if (key == GLFW_KEY_U) {		// toggle ammo limit
		ammoLimitToggle == 5 ? ammoLimitToggle = 100 : ammoLimitToggle = 5;
	}
	if (key == GLFW_KEY_M) {		// toggle sound
		soundToggle = !soundToggle;
	}
	if (key == GLFW_KEY_P) {		// pause game
		gameIsPaused = !gameIsPaused;
	}	
	if (key == GLFW_KEY_O) {		// toggle sky color
		skyColor == glm::vec3(0.2588f, 0.8078f, 0.9608f) ? skyColor = glm::vec3(0, 0.006f, 0.67f) : skyColor = glm::vec3(0.2588f, 0.8078f, 0.9608f);
		sky = constructor.createSky("sky", skyColor);
	}
	if (key == GLFW_KEY_R) {		// reset game
		resetGame();
	}

	if (key == GLFW_KEY_1) {		// change player 1 color
		p1.colorIndex = (p1.colorIndex + 1) % playerColors.size();
		p1.tank = constructor.createTank("tank1", tankSpawnPoint, playerColors.at(p1.colorIndex), tankLength, true);
	}
	if (key == GLFW_KEY_2) {		// change player 2 color
		p2.colorIndex = (p2.colorIndex + 1) % playerColors.size();
		p2.tank = constructor.createTank("tank2", tankSpawnPoint, playerColors.at(p2.colorIndex), tankLength, true);
	}
}

void tankWars::OnKeyRelease(int key, int mods)
{
}

void tankWars::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void tankWars::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void tankWars::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void tankWars::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void tankWars::OnWindowResize(int width, int height)
{
	res.width = window->GetResolution().x;
	res.height = window->GetResolution().y;
	cout << "Window resized to " << width << "x" << height << "\n";
}

