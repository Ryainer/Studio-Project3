
#include "GameObject.h"

GameObject::GameObject(GAMEOBJECT_TYPE typeValue) 
	: type(typeValue),
	scale(1, 1, 1),
	active(false),
	mass(1.f),
	momentofinertia(1.f),
	dir(1,0,0),
	angularVelocity(0),
	health(3),
	bounceTime(0),
	angle(0),
	normal(0,0,0),
	range(0)
{
}

GameObject::~GameObject()
{
}