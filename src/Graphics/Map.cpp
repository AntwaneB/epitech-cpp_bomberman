#include "Graphics/Map.hh"
#include "global.hh"

Graphics::Map::Map(const ::Map& map)
	: _map(&map)
{
}

Graphics::Map::~Map()
{
	for (unsigned int y = 0; y < _map->height(); y++)
		for (unsigned int x = 0; x < _map->width(); x++)
			delete _blocks[y][x];
}

gdl::Texture*
Graphics::Map::loadTexture(const std::string & path)
{
	auto it = _textures.find(path);
	if (it != _textures.end())
	{
		return (it->second);
	}
	else
	{
		gdl::Texture* texture = new gdl::Texture;
		texture->load(path);
		_textures[path] = texture;
		return (_textures[path]);
	}
}

bool Graphics::Map::initialize()
{
	for (unsigned int y = 0; y < _map->height(); y++)
	{
		std::vector<Graphics::Object*> line;
		for (unsigned int x = 0; x < _map->width(); x++)
		{
			if (_map->map()[y][x] && _map->map()[y][x]->visible())
			{
				Object *cube = new Cube(Position<>(x, y, 1), _map->map()[y][x], loadTexture(_map->map()[y][x]->texture()));
				cube->initialize();
				line.push_back(cube);
			}
			else if (_map->map()[y][x] && !_map->map()[y][x]->visible())
			{
				Object *cube = new Cube(Position<>(x, y, 1), _map->map()[y][x], loadTexture(g_settings["maps"]["floor_texture"]));
				cube->initialize();
				line.push_back(cube);
			}
		}
		_blocks.push_back(line);
	}

	_ground = new Ground(Position<>((_map->width() - 1) / 2, (_map->height() - 1) / 2, 0.4));
	if (_ground->initialize() == false)
		return (false);
	_skybox = new Skybox();
	if (_skybox->initialize() == false)
		return (false);
	return (true);
}

bool Graphics::Map::update()
{
	for (unsigned int y = 0; y < _map->height(); y++)
		for (unsigned int x = 0; x < _map->width(); x++)
			if (!_map->map()[y][x]->visible() && _blocks[y][x] != NULL)
				_blocks[y][x] = NULL;
	return (true);
}

void Graphics::Map::draw(gdl::AShader &shader, gdl::Clock const &clock)
{
	_t +=1;
	_ground->draw(shader, clock);
	_skybox->draw(shader, clock);
	for (unsigned int y = 0; y < _blocks.size(); y++)
		for (unsigned int x = 0; x < _blocks[y].size(); x++)
			if (_blocks[y][x])
				_blocks[y][x]->draw(shader, clock);
}