#ifndef GRAPHICS_ITEM_HPP_
# define	GRAPHICS_ITEM_HPP_

#include <iostream>
#include <string>
#include "Model.hh"
#include "Graphics/Object.hh"
#include "Geometry.hh"
#include "Texture.hh"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Core/Item.hh"

namespace Graphics
{

	class Item : public Object
	{
	public:
		Item(const ::Item*, std::vector<gdl::Model*>, size_t);
		virtual ~Item();

		virtual bool  initialize();
		virtual void  draw(gdl::AShader &shader, gdl::Clock const &clock);

		bool	operator==(const ::Item*) const;

	private:
		const ::Item*	_item;
		gdl::Texture	_texture;
		gdl::Model*		_model;
	};

};

#endif
