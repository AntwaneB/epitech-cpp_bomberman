#ifndef SAVE_HH
#define SAVE_HH

#include "Core/Config.hh"
#include "Core/Character.hh"
#include "Core/Level.hh"
#include "Core/Map.hh"
#include "Core/Clock.hh"
#include "Core/Item.hh"
#include "Core/BonusItem.hh"
#include "Core/Position.hh"
#include "Core/Monster.hh"

class	Save
{
public:
	Save(const Level*);
	Save(const Level*, const std::string);
	~Save();

private:
	Config::Param			saveCharacter(const Character*) const;
	Config::Param			saveMonster(const Monster*) const;
	Config::Param			saveMap(const Map*) const;
	Config::Param			saveBlock(const Block*) const;
	Config::Param			saveClock(const Clock*) const;
	Config::Param			saveBomb(const Bomb*) const;
	Config::Param			saveBonusItem(const BonusItem*) const;
	Config::Param			savePosition(const Position<int>*) const;
	Config::Param			savePosition(const Position<double>*) const;

public:
	void			save(void) const;
	void			save(const std::string) const;

private:
	const Level*	_level;
	std::string		_filename;
};

#endif // SAVE_HH
