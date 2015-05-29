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
	: _level(level), _nth(nth), _isPlayer(isPlayer), _position(x + 0.5, y + 0.5, z), _solid(true), _alive(true),
	  _killedBy(NULL), _ia(NULL), _previousBomb(-1), _elapsedTime(-1), _elapsedCentiseconds(-1), _prevMovement(-1), _moving(false),
	  _direction(MOVE_DOWN), _score(0)
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

Position<double>
Character::position() const
{
	return (_position);
}

Position<double>
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

int
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

bool
Character::moving() const
{
	return (_moving);
}

Character::Action
Character::direction() const
{
	return (_direction);
}

void
Character::tick(Subject* entity)
{
	Clock* clock = safe_cast<Clock*>(entity);

	if (_elapsedTime == -1)
		_elapsedTime = clock->deciseconds();
	if (_elapsedCentiseconds == -1)
		_elapsedCentiseconds = clock->centiseconds();

	// Capping speed
	if (_attributes["speed"] > static_cast<int>(_attributes["max_speed"]))
		_attributes["speed"] = static_cast<int>(_attributes["max_speed"]);

	// Managing animations
	if (_moving && clock->seconds() >= _movingUntil)
	{
		_moving = false;
	}

	// Updating clocks
	if (static_cast<int>(clock->deciseconds()) - _elapsedTime >= 1)
	{
		_elapsedTime++;

		if (_ia && !_isPlayer)
			_ia->playTurn();
	}
	if (static_cast<int>(clock->centiseconds()) - _elapsedCentiseconds >= 1)
	{
		_elapsedCentiseconds += static_cast<int>(clock->centiseconds()) - _elapsedCentiseconds;
	}

	// Managing actions
	if (_queuedActions.size() > 0
		&& (_queuedActions.front() == Character::MOVE_UP || _queuedActions.front() == Character::MOVE_DOWN
		 || _queuedActions.front() == Character::MOVE_LEFT || _queuedActions.front() == Character::MOVE_RIGHT)
		&& _attributes["can_move"] == true
		&& _attributes["speed"] != 0
		&& _elapsedCentiseconds / (static_cast<int>(_attributes["max_speed"]) / static_cast<int>(_attributes["speed"])) > _prevMovement)
	{ // Triggering movement
		_prevMovement = _elapsedCentiseconds / (static_cast<int>(_attributes["max_speed"]) / static_cast<int>(_attributes["speed"]));
		Character::Action movement = _queuedActions.front();
		_queuedActions.pop();

		this->move(movement, *clock);
	}

	if (_queuedActions.size() > 0 && _queuedActions.front() == Character::DROP_BOMB)
	{ // Dropping a bomb
		_queuedActions.pop();

		if (_attributes["bombs"]["available"] == true
			&& _bombs.size() < static_cast<size_t>(_attributes["bombs"]["amount"])
			&& clock->seconds() - _previousBomb > 0.2)
		{
			_previousBomb = clock->seconds();
			this->dropBomb();
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
Character::move(Character::Action action, const Clock & clock)
{
	Position<double> newPos = _position;

	double step = 0.001 * static_cast<int>(_attributes["speed"]);

	double duration = 0.0 / 1000.0;

	switch (action)
	{
		case Character::MOVE_DOWN:
			newPos.incY(step);
			break;
		case Character::MOVE_UP:
			newPos.decY(step);
			break;
		case Character::MOVE_LEFT:
			newPos.decX(step);
			break;
		case Character::MOVE_RIGHT:
			newPos.incX(step);
			break;
		default:
			break;
	}
	Position<double> newPosRightDown(newPos);
	newPosRightDown.incX(0.25);
	newPosRightDown.incY(0.25);
	Position<double> newPosLeftUp(newPos);
	newPosLeftUp.decX(0.25);
	newPosLeftUp.decY(0.25);

	auto bombs = _level->bombs();
	if ((_level->map().at(newPosLeftUp)->solid() == false && _level->map().at(newPosRightDown)->solid() == false
		&& ((bombs[newPosLeftUp].empty() && bombs[newPosRightDown].empty())
			|| (!bombs[_position].empty())))
		|| _solid == false)
	{ // The block where we want to move isn't solid and their's no bomb there

		_prevPosition = _position;
		switch (action)
		{
			case Character::MOVE_DOWN:
				_position.incY(step);
				break;
			case Character::MOVE_UP:
				_position.decY(step);
				break;
			case Character::MOVE_LEFT:
				_position.decX(step);
				break;
			case Character::MOVE_RIGHT:
				_position.incX(step);
				break;
			default:
				break;
		}

		_moving = true;
		_movingUntil = clock.seconds() + duration * 2;
		_direction = action;

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
