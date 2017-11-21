
#include "Engine.hpp"

Billboard::Billboard() {
	
}

Billboard::~Billboard() {
	
}

void Billboard::start() {
	if (this->gameObject->staticObject)
		this->gameObject->staticObject = false;
	
	this->transform = this->gameObject->getComponent<Transform>();
	
	GameObject* camera = this->gameObject->scene->mainCamera;
	this->transformCamera = camera->getComponent<Transform>();
	
}

void Billboard::update() {
	glm::vec3 cameraDirection = this->transformCamera->position - this->transform->position;
	cameraDirection = glm::vec3(cameraDirection.x, 0, cameraDirection.z);
	cameraDirection = glm::normalize(cameraDirection);
	this->transform->rotation = glm::rotation(this->game->worldFront, cameraDirection);
//	this->transform->rotation = glm::rota
}