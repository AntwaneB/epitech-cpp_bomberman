/*
 * File:   IA.hh
 * Author: buchse_a
 *
 * Created on May 7, 2015, 1:01 AM
 */

#ifndef IA_HH
#define	IA_HH

#include <vector>
#include <random>
#include "Level.hh"
#include "Character.hh"
#include "Position.hh"
#include "Map.hh"
#include "Block.hh"
#include "Core/Lua.hh"

class Bomb;
class Item;

#define VERBOSE false

namespace IA
{
	class Area
	{
	public:
		 Area(bool, bool);
		 Area();
		 ~Area();

		 bool    explosion() const;
		 bool    bomb() const;
		 int     enemy() const;
		 int     history() const;
		 bool    wall() const;
		 bool    destructible() const;
		 void    setDirection(Character::Action);
		 void 	 setDestructible(bool);
		 void 	 setWall(bool);
		 void    setExplosion(bool);
		 void    setBomb(bool);
		 void    incEnemy();
		 void    incHistory();
		 void 	 reset();
		 Character::Action 		direction() const;

	private:
		 bool    _destructible;
		 bool    _wall;
		 bool    _bomb;
		 int     _enemy;
		 bool    _explosion;
		 int     _history;
		 Character::Action _direction;

		 /* data */
	};

	template<Difficulty T>
	class IA
	{
		public:
			IA(Level const* level, Character* character);
			virtual ~IA();
			void playTurn();

		private:
			bool	scanMapForEnemy(Character::Action & action);
         	bool    scanMapForEnemyThroughDestructible(Character::Action &);
         	bool	isAroundSafe() const;
			void	scanMap();
			void 	refreshPosition();
			bool	BombOpportunity();
			bool	BombDetection();
			bool 	scanMapForEscape(Character::Action &);
			bool	simulateEscape();
			void 	debugStrategieMap() const;
         	void    displayAction(Character::Action) const;
         	void    debugStrategieMapDirections() const;
         	Character::Action 	checkDestinationSafe(Character::Action) const;
			Character::Action 	checkAlignment(Character::Action) const;
			Character::Action 	Move();
			Character::Action 	escapeBomb();

		private:
			int 	 _myX;
		 	int 	 _myY;
		 	bool	 _xCentered;
		 	bool 	 _yCentered;
		 	bool 	 _strategyMapInitialized;
		 	Character::Action 	_lastAction;
			std::vector<std::vector<Area> > 	_strategyMap;
			std::list<Position<> >              _searchNodes;
			Character*							_self;
			const Level*						_level;
			lua_State* 						_L;
	};
}

template<IA::Difficulty T>
IA::IA<T>::IA(Level const* level, Character* character)
	: _strategyMapInitialized(false), _level(level)
{
	if (VERBOSE)
	{
		std::cout << "Starting IA constructor..." << std::endl;
	}
	_self = character;
	_xCentered = false;
	_yCentered = false;
	_lastAction = Character::WAIT;
	_strategyMapInitialized = false;
	if (VERBOSE)
	{
		std::cout << "END IA constructor" << std::endl;
	}
}

template<IA::Difficulty T>
IA::IA<T>::~IA()
{
	lua_close(_L);
}

template<IA::Difficulty T>
void IA::IA<T>::debugStrategieMap() const
{
	if (VERBOSE)
	{
		for (std::vector<std::vector<Area> >::const_iterator it = _strategyMap.begin(); it != _strategyMap.end(); ++it)
		{
			for (std::vector<Area>::const_iterator i = it->begin(); i != it->end(); ++i)
			{
				Area a = *i;
					if (a.explosion())
						 std::cout << "x";
				else if (a.destructible() == false && a.wall() == false)
					std::cout << "_";
				else if (a.destructible())
					std::cout << "d";
				else if (a.wall())
					std::cout << "1";
				else if (a.enemy() > 0)
					std::cout << "e";
				else if (a.bomb())
					std::cout << "b";
			}
			std::cout << std::endl;
		}
	}
}

template<IA::Difficulty T>
void IA::IA<T>::debugStrategieMapDirections() const
{
	if (VERBOSE)
	{
		Character::Action currentDirection;
		 int         x = 0;
		 int         y = 0;

		 for (std::vector<std::vector<Area> >::const_iterator it = _strategyMap.begin(); it != _strategyMap.end(); ++it)
		 {
			  y = 0;
			  for (std::vector<Area>::const_iterator i = it->begin(); i != it->end(); ++i)
			  {
					Area a = *i;
					currentDirection = a.direction();
					if (currentDirection == Character::MOVE_UP)
						 std::cout << "U";
					else if (currentDirection == Character::MOVE_RIGHT)
						 std::cout << "R";
					else if (currentDirection == Character::MOVE_DOWN)
						 std::cout << "D";
					else if (currentDirection == Character::MOVE_LEFT)
						 std::cout << "L";
					else if (currentDirection == Character::WAIT)
						 std::cout << "W";
					else if (a.destructible() == false && a.wall() == false)
						 std::cout << "1";
					else if (_myX == x && _myY == y)
						 std::cout << "@";
			  y++;
			  }
			  std::cout << std::endl;
		 x++;
		 }
	}
}

template<IA::Difficulty T>
bool    IA::IA<T>::scanMapForEnemy(Character::Action & action)
{
    std::vector<int>        searchX = {0, 1, 0, -1};
    std::vector<int>        searchY = {-1, 0, 1, 0};
    Character::Action   	currentDirection;
    int                     currentX;
    int                     currentY;
    int                     mapHeight = _level->map().height();
    int                     mapWidth = _level->map().width();
    int                     i = 0;

    if (VERBOSE)
	{
        std::cout << "Starting scanMapForEnemy()" << std::endl;
        debugStrategieMap();
        debugStrategieMapDirections();
 	}
    if (_searchNodes.size() == 0)
    {
        if (VERBOSE)
            std::cout << "  scanMapForEnemy() END : No possible path to enemy !" << std::endl;
        action = Character::WAIT;
        return (true);
    }
    else
    {
        currentX = _searchNodes.front().x();
        currentY = _searchNodes.front().y();
        _searchNodes.pop_front();
        currentDirection = (_strategyMap[currentY][currentX]).direction();
        while (i < 4)
        {
            if ((currentX + searchX[i]) >= 0 && (currentX + searchX[i]) < mapWidth && (currentY + searchY[i]) >= 0
                && (currentY + searchY[i]) < mapHeight
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].bomb() == false
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == false
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].direction() == Character::WAIT)
                {
	            if (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].wall() == false
	            	&& _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == false
	                    && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].enemy() > 0)
                    {
                        if (VERBOSE)
                            std::cout << "  scanMapForEnemy() END: direction to nearest enemy found at " << currentX + searchX[i] << "/" << currentY + searchY[i]<< std::endl;
                        action = currentDirection;
                        return (true);
                    }
	            else if (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].wall() == false
	            	&& _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == false
	                            && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].enemy() == 0)
                    {
                        if (VERBOSE)
                        {
                            std::cout << "  scanMapForEnemy() : no enemy found at " << currentX << "/" << currentY;
                            std::cout << ". extending search zone to " << currentX + searchX[i] << "/" << currentY + searchY[i] << std::endl;
                        }
                         _strategyMap[currentY + searchY[i]][currentX + searchX[i]].setDirection(currentDirection);
                         _searchNodes.push_back(Position<>(currentX + searchX[i], currentY + searchY[i]));
                    }
                }
             i++;
        }
    }
    if (VERBOSE)
        std::cout << "  scanMapForEnemy() END" << std::endl;
    return (false);
}

template<IA::Difficulty T>
void IA::IA<T>::scanMap()
{
	std::vector<std::vector<Block*> > 	map = _level->map().map();
	std::vector<int> 					searchX = {0, 1, 0, -1};
	std::vector<int> 					searchY = {1, 0, -1, 0};
	std::vector<bool> 					continueCheckingDirection = {true, true, true, true};
	unsigned int 						y = 0;
	unsigned int 						x = 0;
	int 								iRange = 1;
	int 								iSearch = 0;

	if (VERBOSE)
	{
		std::cout << "Starting scanMap()..." << std::endl;
		debugStrategieMap();
	}
	if (_strategyMapInitialized == false)
		_strategyMap.resize(_level->map().height());
	while (y < _level->map().height())
	{
		if (_strategyMapInitialized == false)
			_strategyMap[y].resize(_level->map().width());
		x = 0;
		while (x < _level->map().width())
		{
			if (_strategyMapInitialized == false)
				_strategyMap[y][x] = Area(map[y][x]->destructible(), map[y][x]->solid());
			else
			{
				_strategyMap[y][x].setDestructible(map[y][x]->destructible());
				_strategyMap[y][x].setWall(map[y][x]->solid());
				_strategyMap[y][x].reset();
			}
			x++;
		}
		y++;
	}
	_strategyMapInitialized = true;
	y = 0;
	std::map<Position<>, std::list<Character*> > players = _level->characters();
	for (std::map<Position<>, std::list<Character*> >::iterator i = players.begin(); i != players.end(); ++i)
	{
		x = 0;
		for (std::list<Character*>::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			Character* c = *j;
			Position<> posPlayer = c->position();
			_strategyMap[static_cast<int>(std::round(posPlayer.y() - 0.5))][static_cast<int>(std::round(posPlayer.x() - 0.5))].incEnemy();
			x++;
		}
		y++;
	}
	std::map<Position<>, std::list<Bomb*> > bombs = _level->bombs();
	y = 0;
	for (std::map<Position<>, std::list<Bomb*> >::iterator it = bombs.begin(); it != bombs.end(); ++it)
	{
		x = 0;
		for (std::list<Bomb*>::iterator i = it->second.begin(); i != it->second.end(); ++i)
		{
			Position<> p = (*i)->position();
			int 	bombX = p.x();
			int 	bombY = p.y();
			int 	bombRange = (*i)->range();
			if (VERBOSE)
			{
				std::cout << "  scanMap() BMB : GRID position of bomb x/y : " << bombX << "/" << bombY << ". range:" << (*i)->range();
			}
			_strategyMap[bombY][bombX].setBomb(true);
			_strategyMap[bombY][bombX].setExplosion(true);
			iRange = 0;
			continueCheckingDirection = {true, true, true, true};
			while (iRange <= bombRange)
			{
				iSearch = 0;
				while (iSearch < 4)
				{
					if ((bombX + (searchX[iSearch] * iRange)) > 0 && (bombX + (searchX[iSearch] * iRange)) < static_cast<int>(_level->map().width())
					&& (bombY + (searchY[iSearch] * iRange)) > 0 && (bombY + (searchY[iSearch] * iRange)) < static_cast<int>(_level->map().height())
					&& continueCheckingDirection[iSearch] == true)
					{
						_strategyMap[bombY + (searchY[iSearch] * iRange)][bombX + (searchX[iSearch] * iRange)].setExplosion(true);
						if (_strategyMap[bombY + (searchY[iSearch] * iRange)][bombX + (searchX[iSearch] * iRange)].destructible() == true
							|| _strategyMap[bombY + (searchY[iSearch] * iRange)][bombX + (searchX[iSearch] * iRange)].wall() == true)
							continueCheckingDirection[iSearch] = false;
					}
					iSearch++;
				}
				iRange++;
			}
		}
	}
    if (VERBOSE)
        std::cout << "  scanMap() END" << std::endl;
}

template<IA::Difficulty T>
void IA::IA<T>::playTurn()
{
	if (VERBOSE)
	{
		std::cout << "BEGIN IA" << std::endl;
    }
    refreshPosition();

	scanMap();
    bool isInDanger = BombDetection();
	Character::Action action;

	if (isInDanger)
	{
		if (VERBOSE)
			std::cout << "  IA in danger!" << std::endl;
		action = escapeBomb();
	}
	else
	{
		if (BombOpportunity() && simulateEscape() != Character::WAIT)
			action = Character::DROP_BOMB;
		else
			action = Move();
	}
	if (isInDanger == false)
	{
		action = checkDestinationSafe(action);
	}
	action = checkAlignment(action);
	_self->pushAction(action);
    if (VERBOSE)
    {
        debugStrategieMap();
        debugStrategieMapDirections();
        std::cout << "  END IA with finalAction : ";
        displayAction(action);
        std::cout << std::endl << std::endl;
    }
    _searchNodes.clear();
}

template<IA::Difficulty T>
Character::Action IA::IA<T>::escapeBomb()
{
    std::vector<Character::Action>  searchActions = {Character::MOVE_UP, Character::MOVE_RIGHT, Character::MOVE_DOWN, Character::MOVE_LEFT};
    std::vector<int>                searchX = {0, 1, 0, -1};
    std::vector<int>                searchY = {-1, 0, 1, 0};
    Character::Action               finalAction = Character::WAIT;
    bool                            escapeDirectionFound = false;
    int                             mapHeight = _level->map().height();
    int                             mapWidth = _level->map().width();
    int                             i = 0;
    int                             counter = 0;

    if (VERBOSE)
    {
    	refreshPosition();
        std::cout << "Starting escapeBomb()" << std::endl;
        std::cout << "  myPos : " << _myX << "/" << _myY << std::endl;
    }
    _strategyMap[_myY][_myX].setDirection(Character::MOVE_UP);
    while (i < 4)
    {
        if ((_myX + searchX[i]) >= 0 && (_myX + searchX[i]) < mapWidth && (_myY + searchY[i]) >= 0 && (_myY + searchY[i]) < mapHeight
                && _strategyMap[_myY + searchY[i]][_myX + searchX[i]].wall() == false
                && _strategyMap[_myY + searchY[i]][_myX + searchX[i]].destructible() == false
                && _strategyMap[_myY + searchY[i]][_myX + searchX[i]].direction() == Character::WAIT
                && _strategyMap[_myY + searchY[i]][_myX + searchX[i]].bomb() == false)
        {
            if (_strategyMap[_myY + searchY[i]][_myX + searchX[i]].explosion() == false)
            {
                if (VERBOSE)
                {
                    std::cout << "  escapeBomb() END : success with i:" << i <<" action: ";
                    displayAction(searchActions[i]);
                    std::cout << "with x/y block found: " <<  _myX + searchX[i] << "/" <<_myY + searchY[i] <<std::endl;
                }
                return (searchActions[i]);
            }
            else
            {
            counter++;
            _strategyMap[_myY + searchY[i]][_myX + searchX[i]].setDirection(searchActions[i]);
            _searchNodes.push_back(Position<>(_myX + searchX[i], _myY + searchY[i]));
            }
        }
        i++;
    }
    if (VERBOSE)
        std::cout << "  escapeBomb() : found " << counter << " possible direction(s)" << std::endl;
    while (escapeDirectionFound == false)
    {
         escapeDirectionFound = scanMapForEscape(finalAction);
    }
    if (VERBOSE)
        std::cout << "  escapeBomb() END" << std::endl;
    return finalAction;
}

template<IA::Difficulty T>
bool    IA::IA<T>::scanMapForEscape(Character::Action & action)
{
    std::vector<int>    searchX = {0, 1, 0, -1};
    std::vector<int>    searchY = {-1, 0, 1, 0};
    Character::Action   currentDirection;
    int                 currentX;
    int                 currentY;
    int                 mapHeight = _level->map().height();
    int                 mapWidth = _level->map().width();
    int                 i = 0;

    if (VERBOSE)
        std::cout << "Starting scanMapForEscape()" << std::endl;
    if (_searchNodes.size() == 0)
    {
        if (VERBOSE)
             std::cout << "  scanMapForEscape() END: No possible escape path !" << std::endl;
        action = Character::WAIT;
        return (true);
    }
    else
    {
        currentX = _searchNodes.front().x();
        currentY = _searchNodes.front().y();
        _searchNodes.pop_front();
        currentDirection = (_strategyMap[currentY][currentX]).direction();
        while (i < 4)
        {
            if ((currentX + searchX[i]) >= 0 && (currentX + searchX[i]) < mapWidth && (currentY + searchY[i]) >= 0
                && (currentY + searchY[i]) < mapHeight
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].direction() == Character::WAIT
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].bomb() == false)
                {
                if (VERBOSE)
                    std::cout << "  scanMapForEscape() checking if " << currentX + searchX[i] << "/" << currentY + searchY[i] << "is explosion-free" << std::endl;

                if (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].wall() == false
                    && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].destructible() == false
                        && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == false)
                    {
                        if (VERBOSE)
                            std::cout << "  scanMapForEscape() END : direction to nearest explosion-free node found :" ;
                        action = currentDirection;
                        displayAction(currentDirection);
                        return (true);
                    }
                else if (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].wall() == false
                    && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].destructible() == false
                        && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == true)
                    {
                        if (VERBOSE)
                        {
                            std::cout << "  scanMapForEscape() : explosion zone at :" << currentX << "/" << currentY;
                            std::cout << ". Extending search zone to " << currentX + searchX[i] << "/" << currentY + searchY[i] << std::endl;
                        }
                        _strategyMap[currentY + searchY[i]][currentX + searchX[i]].setDirection(currentDirection);
                        _searchNodes.push_back(Position<>(currentX + searchX[i], currentY + searchY[i]));
                    }
                }
             i++;
        }
    }
    if (VERBOSE)
        std::cout << "  scanMapForEscape() END : No success" << std::endl;
    return (false);
}

template<IA::Difficulty T>
bool IA::IA<T>::BombDetection()
{
    if (VERBOSE)
    	std::cout << "Starting BombDetection() : " << _myX << "/" << _myY << std::endl;
    if (_strategyMap[_myY][_myX].explosion() == true || _strategyMap[_myY][_myX].bomb() == true)
    {
    	if (VERBOSE)
    	std::cout << "  BombDetection() END : true" << std::endl;
        return true;
    }
    else
    {
    	if (VERBOSE)
    	std::cout << "  BombDetection() END : false" << std::endl;
        return false;
	}
}

template<IA::Difficulty T>
void IA::IA<T>::displayAction(Character::Action action) const
{
	if (VERBOSE)
	{
		if(action == Character::MOVE_UP)
			std::cout << "MOVE_UP";
		else if(action == Character::MOVE_RIGHT)
			std::cout << "MOVE_RIGHT";
		else if(action == Character::MOVE_DOWN)
			std::cout << "MOVE_DOWN";
		else if(action == Character::MOVE_LEFT)
			std::cout << "MOVE_LEFT";
		else if(action == Character::WAIT)
			std::cout << "WAIT";
		else if(action == Character::DROP_BOMB)
			std::cout << "DROP_BOMB";
		else
			std::cout << "ERROR NOT AN ACTION***********" << std::endl;
	}
}

template<IA::Difficulty T>
bool IA::IA<T>::BombOpportunity()
{
	std::vector<bool>  continueCheckingDirection = {true, true, true, true};
	std::vector<int>   searchX = {0, 1, 0, -1, 0, 2, 0, -2};
	std::vector<int>   searchY = {-1, 0, 1, 0, -2, 0, 2, 0};
	int                mapHeight = _level->map().height();
	int                mapWidth = _level->map().width();
	int                i = 0;

	if (VERBOSE)
	    std::cout << "Starting BombOpportunity()..." << std::endl;
	i = 0;
	while (i < 8)
	{
	      if ((_myX + searchX[i]) >= 0 && (_myX + searchX[i]) < mapWidth && (_myY + searchY[i]) >= 0
	            && (_myY + searchY[i]) < mapHeight && _strategyMap[_myY + searchY[i]][_myX + searchX[i]].enemy() == true
	            && continueCheckingDirection[i % 4] == true)
	      {
	      	if (_strategyMap[_myY + searchY[i]][_myX + searchX[i]].destructible() == true
	      		|| _strategyMap[_myY + searchY[i]][_myX + searchX[i]].wall() == true)
	      		continueCheckingDirection[i % 4] = false;
	        if (VERBOSE)
	            std::cout << "  BombOpportunity() END: advise to DROP_BOMB! (enemy nearby)" << std::endl;
	        return (true);
	      }
	    i++;
	}
	if (VERBOSE)
	    std::cout << "  BombOpportunity() END: useless to drom bomb here (nobody around)" << std::endl;
	return (false);
}

template<IA::Difficulty T>
bool    IA::IA<T>::scanMapForEnemyThroughDestructible(Character::Action & action)
{
    std::vector<int>        searchX = {0, 1, 0, -1};
    std::vector<int>        searchY = {-1, 0, 1, 0};
    Character::Action       currentDirection;
    int                     currentX;
    int                     currentY;
    int                     mapHeight = _level->map().height();
    int                     mapWidth = _level->map().width();
    int                     i = 0;

    if (VERBOSE)
        std::cout << "Starting scanMapForEnemyThroughDestructible()" << std::endl;
    if (_searchNodes.size() == 0)
    {
        if (VERBOSE)
            std::cout << "  scanMapForEnemyThroughDestructible() END : No possible path to enemy !" << std::endl;
        if (_strategyMap[_myY][_myX].enemy() > 1)
        	action = Character::DROP_BOMB;
        else
        	action = Character::WAIT;
        return (true);
    }
    else
    {
        currentX = _searchNodes.front().x();
        currentY = _searchNodes.front().y();
        _searchNodes.pop_front();
        currentDirection = (_strategyMap[currentY][currentX]).direction();
        while (i < 4)
        {
            if ((currentX + searchX[i]) >= 0
            	&& (currentX + searchX[i]) < mapWidth
            	&& (currentY + searchY[i]) >= 0
                && (currentY + searchY[i]) < mapHeight
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].direction() == Character::WAIT
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].bomb() == false
                && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == false
                && (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].wall() == false
                    || _strategyMap[currentY + searchY[i]][currentX + searchX[i]].destructible() == true))
                {
                if (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].enemy() != 0)
                    {
                        if (VERBOSE)
                        {
                            std::cout << "  scanMapForEnemyThroughDestructible() END : direction to nearest enemy found";
                            std::cout << " at x/y : " << currentX + searchX[i] << "/" << currentY + searchY[i] << " : ";
                            displayAction(currentDirection);
                        }
                        action = currentDirection;
                        return (true);
                    }
                else if (_strategyMap[currentY + searchY[i]][currentX + searchX[i]].enemy() == 0)
                    {
                        if (VERBOSE)
                        {
                            std::cout << "  scanMapForEnemyThroughDestructible() : no enemy found at " << currentX << "/" << currentY;
                            std::cout << ". Extending search zone to " << currentX + searchX[i] << "/" << currentY + searchY[i] << std::endl;
                        }
                         _strategyMap[currentY + searchY[i]][currentX + searchX[i]].setDirection(currentDirection);
                         _searchNodes.push_back(Position<>(currentX + searchX[i], currentY + searchY[i]));
                    }
                }
             i++;
        }
    }
    if (VERBOSE)
        std::cout << "  scanMapForEnemyThroughDestructible() END : No success" << std:: endl;
    return (false);
}

template<IA::Difficulty T>
void IA::IA<T>::refreshPosition()
{
	double	xDiff = (_self->position().x() - 0.5) - static_cast<long>(_self->position().x());
	double 	yDiff = (_self->position().y() - 0.5) - static_cast<long>(_self->position().y());

	if (xDiff <= 0.15 && xDiff >= -0.15)
	{
		_xCentered = true;
	}
	else
	{
		_xCentered = false;
	}
	if (yDiff <= 0.15 && yDiff >= -0.15)
	{
		_yCentered = true;
	}
	else
	{
		_yCentered = false;
	}
	_myX = static_cast<int>(std::round(_self->position().x() - 0.5));
	_myY = static_cast<int>(std::round(_self->position().y() - 0.5));
	if (VERBOSE)
	{
		std::cout << "Character ACTUAL position x/y : " << _self->position().x() << "/" << _self->position().y() << std::endl;
		std::cout << "Corrected GRID   position x/y : " << _myX << "/" << _myY << std::endl;
	}
}

template<IA::Difficulty T>
Character::Action IA::IA<T>::checkAlignment(Character::Action suggestedAction) const
{
	if (VERBOSE)
	{
		std::cout << "starting checkAlignment() with :";
		displayAction(suggestedAction);
		std::cout << std::endl;
	}
	if (suggestedAction == Character::MOVE_UP || suggestedAction == Character::MOVE_DOWN)
	{
		if (_xCentered == true)
		{
			if (VERBOSE)
			{
				std::cout << "checkAlignment() END : x axis is centered. x = ";
				std::cout << (_self->position().x());
				std::cout << ". ";
				displayAction(suggestedAction);
				std::cout << " is validated for final Action" << std::endl;
			}
			return(suggestedAction);
		}
		else if ((_self->position().x() - static_cast<long>(_self->position().x())) < 0.5)
		{
			if (VERBOSE)
			{
				std::cout << "    checkAlignment() : need alignment before movement. x = " << _self->position().x() << ". Action corrected to MOVE_RIGHT" << std::endl;
			}
			return (Character::MOVE_RIGHT);
		}
		else
		{
			if (VERBOSE)
			{
				std::cout << "    checkAlignment() : need alignment before movement. x = " << _self->position().x() << ". Action corrected to MOVE_LEFT" << std::endl;
			}
			return (Character::MOVE_LEFT);
		}
	}
		else if (suggestedAction == Character::MOVE_LEFT || suggestedAction == Character::MOVE_RIGHT)
	{
		if (_yCentered == true)
		{
			if (VERBOSE)
			{
				std::cout << "    checkAlignment() END : y axis is centered. y = ";
				std::cout << (_self->position().y());
				std::cout << ". ";
				displayAction(suggestedAction);
				std::cout << " is validated for final Action" << std::endl;
			}
			return (suggestedAction);
		}
		else if ((_self->position().y() - static_cast<long>(_self->position().y())) < 0.5)
		{
			if (VERBOSE)
			{
				std::cout << "    checkAlignment() : need alignment before movement. y = " << _self->position().y() << ". Action corrected to MOVE_DOWN" << std::endl;
			}
			return (Character::MOVE_DOWN);
		}
		else
		{
			if (VERBOSE)
			{
				std::cout << "    scheckAlignment() : need alignment before movement. y = " << _self->position().y() << ". Action corrected to MOVE_UP"<< std::endl;
			}
			return (Character::MOVE_UP);
		}
	}
	return (suggestedAction);
}

template<IA::Difficulty T>
bool IA::IA<T>::isAroundSafe() const
{
    std::vector<int>        searchX = {0, 1, 0, -1};
    std::vector<int>        searchY = {-1, 0, 1, 0};
    int                     mapHeight = _level->map().height();
    int                     mapWidth = _level->map().width();
    int 	currentY = _myY;
	int 	currentX = _myX;
    int 	i = 0;


    while (i < 4)
    {
    	if ((currentX + searchX[i]) >= 0
        	&& (currentX + searchX[i]) < mapWidth
        	&& (currentY + searchY[i]) >= 0
            && (currentY + searchY[i]) < mapHeight
            && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].wall() == false
            && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].destructible() == false
            && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].explosion() == false
            && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].bomb() == false
            && _strategyMap[currentY + searchY[i]][currentX + searchX[i]].enemy() == 0)
            {
            	return (true);
            }
    	i++;
    }
    return (false);
}

template<IA::Difficulty T>
Character::Action IA::IA<T>::checkDestinationSafe(Character::Action suggestedAction) const
{
	std::map<Character::Action, Position<int>> 				directions;
	std::map<Character::Action, Position<int>>::iterator 	directionIterator;
	Position<int> 											pointedPosition;
	int                     mapHeight = _level->map().height();
    int                     mapWidth = _level->map().width();

	directions[Character::MOVE_UP] = Position<int>(_myX, _myY - 1);
	directions[Character::MOVE_RIGHT] = Position<int>(_myX + 1, _myY);
	directions[Character::MOVE_DOWN] = Position<int>(_myX, _myY + 1);
	directions[Character::MOVE_LEFT] = Position<int>(_myX - 1, _myY);
	if (suggestedAction == Character::DROP_BOMB || suggestedAction == Character::WAIT)
	{
		return (suggestedAction);
	}
	directionIterator = directions.find(suggestedAction);
	if (directionIterator != directions.end())
	{
		pointedPosition = directionIterator->second;
		if (pointedPosition.x() >= 0 && pointedPosition.x() < mapWidth && pointedPosition.y() >= 0 && pointedPosition.y() <mapHeight
			&& (_strategyMap[pointedPosition.y()][pointedPosition.x()].explosion() == true
			|| _strategyMap[pointedPosition.y()][pointedPosition.x()].bomb() == true))
		{
			if (VERBOSE)
			{
				std::cout << " checkDestinationSafe() : action corrected from ";
				displayAction(suggestedAction);
				std::cout << " to WAIT" << std::endl;
			}
			return (Character::WAIT);
		}
	}
	return (suggestedAction);
}

template<IA::Difficulty T>
bool IA::IA<T>::simulateEscape()
{
	std::vector<int>   searchX = {0, 1, 0, -1, 0, 2, 0, -2};
	std::vector<int>   searchY = {-1, 0, 1, 0, -2, 0, 2, 0};
	int                mapHeight = _level->map().height();
	int                mapWidth = _level->map().width();
	int                i = 0;

	_strategyMap[_myY][_myX].setBomb(true);
	while (i < 8)
	{
      if ((_myX + searchX[i]) >= 0 && (_myX + searchX[i]) < mapWidth && (_myY + searchY[i]) >= 0
            && (_myY + searchY[i]) < mapHeight)
     	{
     		_strategyMap[_myY + searchY[i]][_myX + searchX[i]].setExplosion(true);
     	}
     	i++;
	}
	if (escapeBomb() == Character::WAIT)
	{
		scanMap();
		return (false);
	}
	scanMap();
	return (true);
}

#endif	/* IA_HPP */