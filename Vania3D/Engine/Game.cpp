
#include "Engine.hpp"

/*------------------------------------------------------------------------------
< Constructor >
------------------------------------------------------------------------------*/
Game::Game() {
	this->window = new Window("vania", SCREEN_WIDTH, SCREEN_HEIGHT);
	this->resources = new Resources();
}

Game* Game::getInstance() {
	static Game* game = new Game();
	return game;
}


/*------------------------------------------------------------------------------
< Destructor >
------------------------------------------------------------------------------*/
Game::~Game() {
	delete this->window;
	delete this->resources;
}


/*------------------------------------------------------------------------------
< start >
------------------------------------------------------------------------------*/
void Game::start() {
	this->resources->start();
}


/*------------------------------------------------------------------------------
< update >
------------------------------------------------------------------------------*/
void Game::update() {

}
