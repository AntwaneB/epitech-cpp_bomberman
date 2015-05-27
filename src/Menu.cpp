/*
 * File:   Menu.cpp
 * Author: buchse_a
 *
 * Created on May 5, 2015, 8:53 PM
 */

#include <iostream>
#include "Graphics/Menu.hh"
#include "Core/Menu.hh"
#include "Core/Input.hh"

Menu::Menu(const std::string & filename)
	: _filename(filename)
{
	_actions[KEY_PRESSED] = &Menu::keyPressed;

	_layout.importFile(_filename);
	if (_layout["content"].isEmpty())
		throw ConfigException("File " + filename + " is not valid");
}

Menu::~Menu()
{
}

void
Menu::save(void) const
{
	_layout.exportFile(_filename);
}

void
Menu::run()
{
	this->notify(this, MENU_STARTED);
}

void
Menu::keyPressed(Subject* entity)
{
	Input*	input = safe_cast<Input*>(entity);

	switch (input->genericKey())
	{
		case (Input::UP):
		{
			break;
		}
		case (Input::DOWN):
		{
			break;
		}
		case (Input::LEFT):
		{
			break;
		}
		case (Input::RIGHT):
		{
			break;
		}
		case (Input::SPACE):
		case (Input::ENTER):
		{
			break;
		}
		default:
		{
			break;
		}
	}
}

Config&
Menu::layout(void)
{
	return _layout;
}

/*
Config&
Menu::getArrow(void)
{
	return _cfg;
}

Config&
Menu::getCurrent(void)
{
	Config	items;
	Config*	tmp;

	items = _cfg["content"];
	tmp = NULL;

	for (auto it = items.begin(); it != items.end(); ++it)
		if ((it->second)["selectable"] == "true" && (it->second)["selected"] == "true")
			tmp = &(it->second);
	if (tmp)
		return (&tmp);
	else
		return (items.end())
}

Config&
Menu::getPrev(Config & current)
{
	Config	items;
	Config*	tmp;
	int		id;

	id = current["order"] - 1;
	if (id == 0)
		return getLast();

	items = _cfg["content"];
	tmp = &(current);

	for (auto it = items.begin(); it != items.end(); ++it)
		if ((it->second)["order"] == id )
			tmp = &(it->second);
	return (&tmp);
}

Config&
Menu::getNext(Config & current)
{
	return (_cfg);
}

Config&
Menu::getLast(void)
{
	Config	items;
	Config*	tmp;
	int		max;

	items = _cfg["content"];
	tmp = &(items.end());
	max = 0;

	for (auto it = items.begin(); it != items.end(); ++it)
		if ((it->second)["order"] >= max)
		{
			max = (it->second)["order"];
			tmp = &(*it);
		}
	return (*tmp);
}
*/
