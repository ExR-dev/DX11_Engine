#include "Game.h"


Game::Game()
{
	// TODO
}

Game::~Game()
{
	// TODO
}


int Game::Update(const Data &data, const Time &time)
{
	// TODO
	return -1;
}

int Game::Render(const Data &data, const Time &time)
{
	// TODO
	_graphics.BeginRender();

	// Render scene here

	_graphics.EndRender();
	return 0;
}