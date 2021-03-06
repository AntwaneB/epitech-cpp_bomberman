/*
 ** App.cpp
 **
 ** Made by Antoine Buchser
 ** Login   <buchse_a@epitech.eu>
 **
 ** Started on February 6, 2015, 10:53 AM Antoine Buchser
 ** Updated on February 6, 2015, 10:53 AM Antoine Buchser
 */

#include <iostream>
#include "Exception.hpp"
#include "Core/Character.hh"
#include "Core/App.hh"
#include "Core/Level.hh"
#include "Core/Config.hh"
#include "Core/Save.hh"
#include "Graphics/Display.hh"
#include "Graphics/Map.hh"

Config g_settings;

App::App(int ac, char** av)
	: _ac(ac)
{
	_actions[LEVEL_GENERATED] = &App::runLevel;
	_actions[LEVEL_ENDED] = &App::endLevel;
	_actions[EXIT_TRIGGERED] = &App::exit;
	_actions[MENU_STARTED] = &App::menuStarted;

	for (int i = 0; i < _ac; i++)
		_av.push_back(av[i]);

	if (!this->validateArgs())
		throw ArgumentsException("usage:\n./bomberman");

	g_settings.importFile("config/default.xml");
}

bool
App::validateArgs() const
{
	return (true);
}

App::~App()
{
	if (this)
	{
	}
}

void
App::runLevel(Subject* entity)
{
	Level* level = safe_cast<Level*>(entity);

	level->addObserver(this);

	level->addObserver(_display);
	_display->addObserver(level);
	this->addObserver(level);

	level->run();
}

void
App::endLevel(Subject* entity)
{
	Level* level = safe_cast<Level*>(entity);

	if (level->winner())
	{
		Menu* menu = new Menu(level->winner()->isPlayer() ? "./menus/player_win.xml" : "./menus/player_loose.xml", level);
		menu->addObserver(this);
		menu->addObserver(_display);

		this->notify(menu, MENU_STARTED);
	}
}

void
App::exit(Subject* entity __attribute__((unused)))
{
	throw ExitException("Exiting normally");
}

void
App::menuStarted(Subject* entity)
{
	Menu* menu = safe_cast<Menu*>(entity);

	menu->addObserver(this);
	menu->addObserver(_display);
	_display->addObserver(menu);
}

int
App::run()
{
	try
	{
		_display = new Graphics::Display;
		this->addObserver(_display);
		_display->addObserver(this);

		Menu* mainMenu = new Menu("menus/intro.xml");
		mainMenu->addObserver(this);
		mainMenu->addObserver(_display);

		try
		{
			mainMenu->run();
		}
		catch (ExitException const & e)
		{
			delete mainMenu;
			throw e;
		}
	}
	catch (ExitException const & e)
	{
		delete _display;
		return (EXIT_SUCCESS);
	}
	catch (std::exception const & e)
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	return (0);
}
