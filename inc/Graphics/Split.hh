#ifndef SPLIT_HH
#define SPLIT_HH

#include <list>
#include "Input.hh"
#include "Graphics/Object.hh"
#include "Graphics/Character.hh"
#include "Graphics/Bomb.hh"
#include "Graphics/Item.hh"
#include "Graphics/Map.hh"
#include "Graphics/Cube.hh"
#include "Geometry.hh"
#include "Texture.hh"
#include "Core/Config.hh"
#include "Core/Level.hh"
#include "Core/Character.hh"

namespace Graphics
{

	class Split
	{

	public:
		Split(const ::Level*, int id, size_t splitsCount);
		virtual ~Split();

		virtual bool initialize(std::vector<gdl::Model*>);
		virtual void update(gdl::Clock, gdl::Input);
		virtual void draw(gdl::Clock);

		void moveCamera();

	public:
		const ::Level*							_level;
		gdl::BasicShader						_shader;
		const ::Character*					_player;
		std::list<Graphics::Character*>	_characters;
		std::list<Graphics::Bomb*>			_bombs;
		std::list<Graphics::Item*>			_items;
		Graphics::Map*							_map;
		std::vector<gdl::Model*> 			_models;
		double									_x;
		double									_y;
		size_t									_splitsCount;
	};

};

#endif	/* SPLIT_HH */