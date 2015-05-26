#include "Graphics/Split.hh"

Graphics::Split::Split(::Level const * level, int id, size_t splitsCount)
	: _level(level), _x(0), _y(0), _splitsCount(splitsCount)
{
	_map = new Map(level->map());
	if (id == 0)
		_player = _level->players().front();
	else
		_player = _level->players().back();
}

Graphics::Split::~Split()
{
	delete _map;

	for (auto it = _characters.begin(); it != _characters.end(); ++it)
		delete *it;
	for (auto it = _bombs.begin(); it != _bombs.end(); ++it)
		delete *it;
}

bool Graphics::Split::initialize(std::vector<gdl::Model*> models)
{
	_models = models;
	if (!_shader.load("./libgdl/shaders/basic.fp", GL_FRAGMENT_SHADER)
	|| !_shader.load("./libgdl/shaders/basic.vp", GL_VERTEX_SHADER)
	|| !_shader.build())
		return false;

	glm::mat4 projection = glm::perspective(13.0f, (1920.0f / _splitsCount) / 1080.0f, 0.1f, 150.0f);

	_shader.bind();
	moveCamera();
	_shader.setUniform("projection", projection);

	_map->initialize();

	for (auto it = _level->characters().begin(); it != _level->characters().end(); ++it)
	{
		for (auto iit = it->second.begin(); iit != it->second.end(); ++iit)
		{
			Character* character = new Graphics::Character(*iit, _models[0]);
			character->initialize();
			_characters.push_back(character);
		}
	}
	return (true);
}

void Graphics::Split::update(gdl::Clock clock, gdl::Input input)
{
	_map->update();

	// Creating new bombs
	auto bombs = _level->bombsRaw();
	for (auto it = bombs.begin(); it != bombs.end(); ++it)
	{
		bool found = false;
		for (auto iit = _bombs.begin(); iit != _bombs.end() && !found; ++iit)
			found = (*(*iit) == (*it));

		if (!found)
		{
			Bomb* bomb = new Graphics::Bomb(*it, _models[1]);
			bomb->initialize();
			_bombs.push_back(bomb);
		}
	}

	// Creating new items
	auto items = _level->itemsRaw();
	for (auto it = items.begin(); it != items.end(); ++it)
	{
		bool found = false;
		for (auto iit = _items.begin(); iit != _items.end() && !found; ++iit)
			found = (*(*iit) == (*it));

		if (!found)
		{
			Item* item = new Graphics::Item(*it, _models);
			item->initialize();
			_items.push_back(item);
		}
	}


	// Updating bombs
	bombs = _level->bombsRaw();
	for (auto it = _bombs.begin(); it != _bombs.end(); ++it)
	{
		bool exists = false;
		for (auto iit = bombs.begin(); iit != bombs.end() && !exists; ++iit)
		{
			exists = *it && *(*it) == *iit;
		}
		if (!exists)
		{
			delete (*it);
			it = _bombs.erase(it);
			--it;
		}
		else
			(*it)->update(clock, input);
	}

	// Updating items
	items = _level->itemsRaw();
	for (auto it = _items.begin(); it != _items.end(); ++it)
	{
		bool exists = false;
		for (auto iit = items.begin(); iit != items.end() && !exists; ++iit)
		{
			exists = *it && *(*it) == *iit;
		}
		if (!exists)
		{
			delete (*it);
			it = _items.erase(it);
			--it;
		}
		else
			(*it)->update(clock, input);
	}

	// Updating characters
	auto characters = _level->charactersRaw();
	for (auto it = _characters.begin(); it != _characters.end(); ++it)
	{
		bool exists = false;
		for (auto iit = characters.begin(); iit != characters.end() && !exists; ++iit)
		{
			exists = *it && *(*it) == *iit;
		}
		if (!exists)
		{
			delete (*it);
			it = _characters.erase(it);
			--it;
		}
		else
			(*it)->update(clock, input);
	}
}

void Graphics::Split::draw(gdl::Clock clock)
{
	_shader.bind();
	_map->draw(_shader, clock);

	for (auto it = _characters.begin(); it != _characters.end(); ++it)
		(*it)->draw(_shader, clock);
	for (auto it = _bombs.begin(); it != _bombs.end(); ++it)
		(*it)->draw(_shader, clock);
	for (auto it = _items.begin(); it != _items.end(); ++it)
		(*it)->draw(_shader, clock);
}

void Graphics::Split::moveCamera()
{
	_x = _player->position().x();
	_y = _player->position().y();
	double x = 1 + _x;
	double y = 1 + _y;
	glm::mat4 transformation = glm::lookAt(glm::vec3(0, 90, 0), glm::vec3(x, 0, y), glm::vec3(0, 1, -180));
	_shader.setUniform("view", transformation);
}
