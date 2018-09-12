#include "Host.h"



Host::Host()
{
}

void Host::setName(std::string _name)
{
	this->name = _name;
}
std::string Host::getName()
{
	return name;
}

Host::~Host()
{
}
