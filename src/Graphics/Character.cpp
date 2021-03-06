#include "Graphics/Character.hh"

Graphics::Character::Character(::Character const * character, gdl::Model* model, const size_t id, const size_t charactersCount)
	: Object(character->position()), _character(character), _model(model), _frame(0), _anim(false), _id(id + 1), _charactersCount(charactersCount)
{
	_position.decX(0.5);
	_position.decY(0.5);
	scale(glm::vec3(0.0025, 0.0025, 0.0025));
	_color.x = 1.0;
	_color.y = 1.0;
	_color.z = 1.0;
	if (_id <= _charactersCount / 3.0)
		_color.x = (1.0 / (_charactersCount / 3.0)) * _id;
	else if (_id <= _charactersCount / 3.0 * 2.0)
		_color.y = (1.0 / (_charactersCount / 3.0)) * (_id - (_charactersCount / 3.0));
	else
		_color.z = (1.0 / (_charactersCount / 3.0)) * (_id - (_charactersCount / 3.0) * 2);
}

Graphics::Character::~Character()
{

}

bool
Graphics::Character::initialize()
{
	_model->createSubAnim(0, "RESET", 0, 0);
  	_model->createSubAnim(0, "START", 20, 30);
  	_model->createSubAnim(0, "RUN", 37, 53);
  	_model->createSubAnim(0, "END", 54, 100);
  	return (true);
}

bool
Graphics::Character::operator==(const ::Character* other) const
{
	return (other == _character);
}

int
Graphics::Character::getAngle(const::Character::Action key)
{
	switch(key)
		{
			case ::Character::Action::MOVE_UP:
				return (180);
			case ::Character::Action::MOVE_DOWN:
				return (0);
			case ::Character::Action::MOVE_LEFT:
				return (-90);
			case ::Character::Action::MOVE_RIGHT:
				return (90);
			default:
				return (0);
		}
	return (0);
}

void
Graphics::Character::irotate(int angle)
{
	_rotation = glm::vec3(0, angle, 0);
	if (angle == 0)
		return;
	if (angle == 90)
		{
			double position = _position.x();
			_position.setX(_position.y() * -1 + 1);
			_position.setY(position);
		}
	else if (angle == -90)
		{
			double position = _position.x();
			_position.setX(_position.y());
			_position.setY(position * -1 + 1);
		}
	else
		{
			_position.setX(_position.x() * -1 + 1);
			_position.setY(_position.y() * -1 + 1);
		}
}

void
Graphics::Character::update(gdl::Clock const &clock, gdl::Input &input)
{
	(void)clock;
	(void)input;

	if (_character->position().y() != _position.y() || _character->position().x() != _position.x())
	{
		_position = _character->position();
		irotate(getAngle(_character->direction()));
		_position.decX(0.5);
		_position.decY(0.5);
		_position.incZ(0.5);
	}
	_anim = _character->moving();
}

void
Graphics::Character::draw(gdl::AShader &shader, gdl::Clock const &clock)
{
	_texture.bind();
	if (_anim == true)
	{
		if (_frame == 0 || _frame > 25)
		{
			if (_frame > 25)
				_frame = 1;
			_model->setCurrentSubAnim("START");
		}
		else if (_frame == 15)
			_model->setCurrentSubAnim("RUN");
		_frame += 1;
	}
	else
	{
		if (_frame > 0 && _frame < 8)
		{
			_model->setCurrentSubAnim("END");
			_frame = 8;
		}
		else if (_frame > 20)
		{
			_model->setCurrentSubAnim("RESET");
			_frame = 0;
		}
		if (_frame != 0)
			_frame += 2;
	}
	shader.bind();
	shader.setUniform("color", _color);
	_model->draw(shader, getTransformation(), clock.getElapsed());
}

void Graphics::Character::setAnim(bool anim)
{
	_anim = anim;
}