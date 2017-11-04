
#include "Engine.hpp"

/*------------------------------------------------------------------------------
< Constructor >
------------------------------------------------------------------------------*/
MeshRenderer::MeshRenderer() {

}


/*------------------------------------------------------------------------------
< Destructor >
------------------------------------------------------------------------------*/
MeshRenderer::~MeshRenderer() {

}


/*------------------------------------------------------------------------------
< start >
------------------------------------------------------------------------------*/
void MeshRenderer::start() {
	// initialize static shader uniforms before rendering
	this->material->shader->use();
	this->material->setUniformLocations();
}


/*------------------------------------------------------------------------------
< update >
------------------------------------------------------------------------------*/
void MeshRenderer::update() {
	GameObject* gameObject = this->getGameObject();
	
	this->material->shader->use();
	// camera
	this->material->shader->setMat4("projection", this->camera->projection);
	this->material->shader->setMat4("view", this->camera->view);
	this->material->shader->setMat4("model", gameObject->getComponent<Transform>()->model);
	// texture
	this->material->bindTextures();
	// model
	std::vector<glm::mat4> pose = this->model->pose;
	for (unsigned int i = 0 ; i < pose.size() ; i++)
		this->material->shader->setMat4(("bones[" + std::to_string(i) + "]").c_str(), pose[i]);
	this->model->draw();
}


/*------------------------------------------------------------------------------
< add material >
------------------------------------------------------------------------------*/
void MeshRenderer::addModel(Model* model) {
	this->model = model;
}


/*------------------------------------------------------------------------------
< add material >
------------------------------------------------------------------------------*/
void MeshRenderer::addMaterial(Material* material) {
	this->material = material;
}


/*------------------------------------------------------------------------------
< add light probe >
------------------------------------------------------------------------------*/
void MeshRenderer::addLightProbe(LightProbe* lightProbe) {
	this->lightProbe = lightProbe;
}
