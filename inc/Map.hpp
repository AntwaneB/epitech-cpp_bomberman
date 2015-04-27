/*
 * File:   Map.hpp
 * Author: buchse_a
 *
 * Created on April 27, 2015, 11:14 PM
 */

#ifndef MAP_HPP
#define	MAP_HPP

#include "IEntity.hpp"
#include "Observer.hpp"

class Map : public IEntity, public Observer, public Subject
{
public:
	Map();
	virtual ~Map();

	virtual void onNotify(IEntity const & entity, Event event);

private:

};

#endif	/* MAP_HPP */
