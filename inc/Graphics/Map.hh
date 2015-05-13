/*
 * File:   Map.hh
 * Author: buchse_a
 *
 * Created on May 6, 2015, 3:36 PM
 */

#ifndef MAP_HH
#define	MAP_HH

#include "SdlContext.hh"
#include "Game.hh"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "BasicShader.hh"
#include "Clock.hh"
#include "Input.hh"
#include "Graphics/Object.hh"
#include "Graphics/Level.hh"
#include "Graphics/Cube.hh"
#include "Graphics/Ground.hh"
#include "Graphics/Marvin.hh"
#include "Graphics/Bomb.hh"
#include "Geometry.hh"
#include "Texture.hh"
#include <vector>

namespace Graphics
{

	class Map
	{
	public:
		Map();
		virtual ~Map();
		virtual bool initialize(std::vector<Object*> *, unsigned int, unsigned int, std::vector<std::vector<int> >);
		virtual bool update();
		virtual void draw();
	};

};


#endif	/* MAP_HH */

