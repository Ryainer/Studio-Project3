#pragma once
#include"GameObject.h"
#include"Vector3.h"

/*
An ability is defined as an action a player takes to do something. For example, shooting a projectile.
When shooting a projectile, a GameObject is made for said projectile.
This class deals with what happens when a projectile hits an enemy and how it affects them and the player
This also prevents a god file situation and i dont want to be guillotined for it
*/
class AbilityManager
{
private:

public:
	AbilityManager();
	~AbilityManager();

	void DoAbility(GameObject* go1, GameObject* go2, GameObject* player);
};

