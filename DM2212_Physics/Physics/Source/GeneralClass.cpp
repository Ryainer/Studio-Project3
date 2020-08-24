#include "GeneralClass.h"

GeneralClass::GeneralClass()
{
	this->go = nullptr;
	RBCdir = (0, 0, 0);
}

GeneralClass::~GeneralClass()
{
	this->go = nullptr;
	RBCdir = (0, 0, 0);
}

bool GeneralClass::generalAIchck(GameObject* go1, GameObject* go2)
{
	if (go1->type != GameObject::GO_SHIP)
		return false;

	switch (go2->type)
	{
	case GameObject::GO_WBC:
	 {
		Vector3 dist = go1->pos - go2->pos;
		if (dist.Length() > (go1->scale.x + go2->scale.x) * 25)
		{
			return true;
		}
	 }
	case GameObject::GO_RBC:
	 {
		Vector3 dist = go1->pos - go2->pos;
		if (dist.Length() > (go1->scale.x + go2->scale.x) * 10)
		{
			return true;
		}
	 }
	}

	return false;
}

void GeneralClass::generalAIresponse(GameObject* go2, GameObject* PC)
{
	
	switch (go2->type)
	{
	 case GameObject::GO_WBC:
	 {
		 go->active = true;
		 go->type = GameObject::GO_WBC_PROJECTILES;
		 go->scale.Set(0.5f, 0.5f, 0);
		 go->pos = go2->pos;
		 go->vel.Set(go2->dir.x * BULLET_SPEED, go2->dir.y * BULLET_SPEED, 0);
	 }
	case GameObject::GO_RBC:
	 {
		int chckCond = Math::RandIntMinMax(1, 25);

		Vector3 tempDist = go2->pos - PC->pos;
		 
		if (chckCond != 13)
		{
			RBCdir = (tempDist.Normalized() * (-1));
		}
		else if (chckCond = 13)
		{
			RBCdir = tempDist.Normalized();
		}
		
		//go2->vel.y -= 2.5f * go2->vel.y;
	 }
	}
}

void GeneralClass::setAIGO(GameObject* go)
{
	this->go = go;
}

GameObject* GeneralClass::getAIGO()
{
	return this->go;
}

Vector3 GeneralClass::getDir()
{
	return RBCdir;
}
