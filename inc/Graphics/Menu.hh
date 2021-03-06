/*
 * File:   Menu.hh
 * Author: buchse_a
 *
 * Created on May 6, 2015, 9:20 PM
 */

#ifndef MENU_HH
#define	MENU_HH

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <map>
#include "Core/Menu.hh"
#include "Core/Config.hh"
#include "Core/Observer.hpp"
#include "SdlContext.hh"
#include "Game.hh"

namespace Graphics
{
	class Menu : public EventHandler<Graphics::Menu>, public Subject
	{
	public:
		Menu();
		virtual ~Menu();

		void	init(::Menu*);
		void	run();
		void	update();
		void	draw();

	private:
		void	exited(Subject*);

	private:
		sf::RenderWindow	_window;

		::Menu*			_menu;
		bool				_run;

		//Textures
		sf::Texture		_assetsTexture;
		sf::Texture		_backgroundTexture;
		sf::Font			_font;

		//Sprite
		std::list<sf::Sprite>					_sprites;
		std::map<Config::Param*, sf::Text>	_texts;
		std::map<Config::Param*, std::list<sf::Text> >	_collections;
		sf::Sprite*									_background = NULL;
		sf::Sprite									_cursor;
	};
};

#endif	/* MENU_HH */
