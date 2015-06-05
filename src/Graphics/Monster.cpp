#include "Graphics/Monster.hh"

Graphics::Monster::Monster(::Monster const * character, gdl::Model* model)
	: Object(character->position()), _character(character), _model(model)
{
	//_position.decX(0.5);
	//_position.decY(0.5);
	scale(glm::vec3(0.005, 0.005, 0.005));
}

Graphics::Monster::~Monster()
{

}

bool
Graphics::Monster::initialize()
{
	/*_model->createSubAnim(0, "RESET", 0, 0);
  	_model->createSubAnim(0, "START", 20, 30);
  	_model->createSubAnim(0, "RUN", 37, 90);
  	_model->createSubAnim(0, "END", 90, 100);*/
  	return (true);
}

bool
Graphics::Monster::operator==(const ::Monster* other) const
{
	return (other == _character);
}

int
Graphics::Monster::getAngle(const::Character::Action key)
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
Graphics::Monster::irotate(int angle)
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
Graphics::Monster::update(gdl::Clock const &clock, gdl::Input &input)
{
	(void)clock;
	(void)input;

	if (_character->position().y() != _position.y() || _character->position().x() != _position.x())
	{
		_position = _character->position();
		irotate(getAngle(_character->direction()));
		//_position.decX(0.5);
		//_position.decY(0.5);
		_position.incZ(1.8);
	}
	_anim = _character->moving();
}

void
Graphics::Monster::draw(gdl::AShader &shader, gdl::Clock const &clock)
{
	//Couleur
	_texture.bind();
	/*if (_anim == true)
	{
		if (_frame == 0 || _frame > 25)
		{
			if (_frame > 25)
				_frame = 1;
			_model->setCurrentSubAnim("START");
		}
		if (_frame == 10)
			_model->setCurrentSubAnim("RUN");
		if (_frame == 25)
			_frame = 15;
		_frame += 1;
	}
	if (_anim == false)
	{
		if (_frame > 0 && _frame < 8)
		{
			_model->setCurrentSubAnim("END");
			_frame = 8;
		}
		if (_frame >= 8 && _frame <= 20)
			_frame += 1;
		if (_frame >= 20)
		{
			_model->setCurrentSubAnim("RESET");
			_frame = 0;
		}
		if (_frame != 0)
			_frame += 1;
	}*/
	shader.bind();
	shader.setUniform("color", glm::vec4(1,1,1,1));
	_model->draw(shader, getTransformation(), clock.getElapsed());
}

void Graphics::Monster::setAnim(bool anim)
{
	_anim = anim;
}