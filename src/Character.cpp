/*
 * File:   Character.cpp
 * Author: buchse_a
 *
 * Created on April 27, 2015, 10:53 PM
 */

#include <iostream>
#include "global.hh"
#include "Core/Character.hh"
#include "Exception.hpp"
#include "Core/Clock.hh"
#include "Core/Level.hh"
#include "Core/IA.hpp"
#include "Core/Input.hh"

Character::Character(const Level * level, size_t nth, bool isPlayer, size_t x, size_t y, size_t z)
	: _level(level), _nth(nth), _isPlayer(isPlayer), _position(x, y, z), _solid(true), _alive(true), _killedBy(NULL), _ia(NULL), _elapsedTime(-1)
{
	_actions[CLOCK_TICK] = &Character::tick;
	_actions[LEVEL_BOMB_EXPLODED] = &Character::bombExploded;
	_actions[KEY_PRESSED] = &Character::keyPressed;

	_attributes = g_settings["entities"]["character"];

	if (!_isPlayer)
		_ia = new IA::IA<IA::HARD>(_level, this);

	this->notify(this, CHARACTER_SPAWNED);
}

Character::~Character()
{
	if (_ia)
		delete _ia;
}

Position
Character::position() const
{
	return (_position);
}

Position
Character::prevPosition() const
{
	return (_prevPosition);
}

Config&
Character::attributes()
{
	return (_attributes);
}

bool
Character::alive() const
{
	return (_alive);
}

size_t
Character::score() const
{
	return (_score);
}

void
Character::changeScore(int amount)
{
	_score += amount;
}

const Bomb*
Character::killedBy() const
{
	return (_killedBy);
}

bool
Character::isPlayer() const
{
	return (_isPlayer);
}

void
Character::tick(Subject* entity)
{
	Clock* clock = safe_cast<Clock*>(entity);

	if (_elapsedTime == -1)
		_elapsedTime = clock->deciseconds();

	if (static_cast<int>(clock->deciseconds()) - _elapsedTime >= 1)
	{
		_elapsedTime++;

		if (_ia && !_isPlayer)
			_ia->playTurn();

		if (_queuedActions.size() > 0
			&& (_queuedActions.front() == Character::MOVE_UP || _queuedActions.front() == Character::MOVE_DOWN
			 || _queuedActions.front() == Character::MOVE_LEFT || _queuedActions.front() == Character::MOVE_RIGHT)
			&& _attributes["can_move"] == true
			&& _attributes["speed"] != 0
			&& _elapsedTime % (1000 / static_cast<int>(_attributes["speed"])) == 0)
		{ // Triggering movement
			Character::Action movement = _queuedActions.front();
			_queuedActions.pop();

			this->move(movement);
		}

		if (_queuedActions.size() > 0 && _queuedActions.front() == Character::DROP_BOMB)
		{ // Dropping a bomb
			_queuedActions.pop();

			if (_attributes["bombs"]["available"] == true
				&& _bombs.size() < static_cast<size_t>(_attributes["bombs"]["amount"]))
			{
				this->dropBomb();
			}
		}

	}
}

void
Character::bombExploded(Subject* entity)
{
	Bomb* bomb = safe_cast<Bomb*>(entity);

	auto it = std::find(_bombs.begin(), _bombs.end(), bomb);
	if (it != _bombs.end())
		_bombs.erase(it);

	if (bomb->hasHit(_position))
	{ // The character got hit by the bomb
		_alive = false;
		_killedBy = bomb;
		this->notify(this, CHARACTER_DIED);
//		delete this;
	}
}

void
Character::keyPressed(Subject* entity)
{
	Input* input = safe_cast<Input*>(entity);

	std::map<Input::Key, Character::Action> keys;
	keys[Input::UP] = Character::MOVE_UP;
	keys[Input::DOWN] = Character::MOVE_DOWN;
	keys[Input::RIGHT] = Character::MOVE_RIGHT;
	keys[Input::LEFT] = Character::MOVE_LEFT;
	keys[Input::SPACE] = Character::DROP_BOMB;

	if (input->genericKey() > Input::KEYS_GENERIC_START && input->genericKey() < Input::KEYS_GENERIC_END)
	{
		this->clearActions();
		this->pushAction(keys[input->genericKey()]);
	}
}

void
Character::move(Character::Action action)
{
	Position tmp = _position;
	switch (action)
	{
		case Character::MOVE_DOWN:
			tmp.incY();
			break;
		case Character::MOVE_UP:
			tmp.decY();
			break;
		case Character::MOVE_LEFT:
			tmp.decX();
			break;
		case Character::MOVE_RIGHT:
			tmp.incX();
			break;
		default:
			break;
	}

	auto bombs = _level->bombs();
	if ((_level->map().at(tmp)->solid() == false && bombs[tmp].empty()) || _solid == false)
	{ // The block where we want to move isn't solid and their's no bomb there
		_prevPosition = _position;
		switch (action)
		{
			case Character::MOVE_DOWN:
				_position.incY();
				break;
			case Character::MOVE_UP:
				_position.decY();
				break;
			case Character::MOVE_LEFT:
				_position.decX();
				break;
			case Character::MOVE_RIGHT:
				_position.incX();
				break;
			default:
				break;
		}
		this->notify(this, CHARACTER_MOVED);
	}
}

void
Character::dropBomb()
{
	size_t range = _attributes["bombs"]["range"];
	double duration = g_settings["entities"]["bomb"]["duration"];
	double duration_modifier = _attributes["bombs"]["duration_modifier"];

	Bomb* bomb = new Bomb(_position, range, duration * duration_modifier, this);

	_bombs.push_back(bomb);

	this->notify(bomb, BOMB_DROPPED);
}

void
Character::pushAction(Character::Action action)
{
	if (_queuedActions.size() == 0)
		_queuedActions.push(action);
}

void
Character::clearActions()
{
	while (!_queuedActions.empty())
		_queuedActions.pop();
}

/*
void
Character::toConfig(Config & cfg) const
{
	int	index;

	index = 0;
	_position.toConfig(cfg[_nth]["position"]);
	_prevPosition.toConfig(cfg[_nth]["prevPosition"]);
	cfg[_nth]["attributes"] = _attributes;
	for (std::list<Bomb*>::const_iterator it = _bombs.begin(); it != _bombs.end(); it++; index++)
		(*it)->toConfig(cfg[_nth]["bombs"][index]);
}
*/
