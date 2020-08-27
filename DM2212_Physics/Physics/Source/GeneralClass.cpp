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
		break;
	 }
	case GameObject::GO_RBC:
	{
		Vector3 relativeVel = go1->vel - go2->vel;
		Vector3 displacementVel = go2->pos - go1->pos;
		if (relativeVel.Dot(displacementVel) <= 0)
			return false;

		if ((displacementVel.LengthSquared() - 1150.f) <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x))
		{
			return true;
		}
		break;
	}
	case GameObject::GO_TCELLS:
	{
		Vector3 relativeVel = go1->vel - go2->vel;
		Vector3 displacementVel = go2->pos - go1->pos;
		if (relativeVel.Dot(displacementVel) <= 0)
			return false;

		if ((displacementVel.LengthSquared() - 1000.f) <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x))
		{
			return true;
		}
		break;
	}
	case GameObject::GO_DEADCELLS:
	{
		Vector3 relativeVel = go1->vel - go2->vel;
		Vector3 displacementVel = go2->pos - go1->pos;
		if (relativeVel.Dot(displacementVel) <= 0)
			return false;

		if ((displacementVel.LengthSquared() - 650.f) <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x))
		{
			return true;
		}
		break;
	}
	default:
		break;
	}

	return false;
}

GameObject* GeneralClass::generalAIresponse(GameObject* go2, GameObject* PC)
{
	
	switch (go2->type)
	{
	case GameObject::GO_WBC:
	{
		if ((go2->pos - PC->pos).LengthSquared() < (PC->scale.x + go2->scale.x) * (PC->scale.x + go2->scale.x))
		{
			go2->vel.SetZero();
			self_destruct = true;
		}
		else
		{
			go2->vel += 1.f / go2->mass * go2->dir * 50;
		}
		return go2;
	}
	case GameObject::GO_RBC:
	{

		int chckCond = Math::RandIntMinMax(1, 25);
		if (0 >= (PC->scale.x + go2->scale.x) * (PC->scale.x + go2->scale.x))
		{
			go2->vel.SetZero();
			self_destruct = true;
		}
		else
		{
			if (chckCond != 13)
			{
				go2->vel = -(go2->vel * 35.f);
				panic = true;
			}
			else if (chckCond == 13)
			{
				go2->vel = (go2->vel * 35.f);
			}
		}
		return go2;

	}
	case GameObject::GO_TCELLS:
	{
		if (0 >= (PC->scale.x + go2->scale.x) * (PC->scale.x + go2->scale.x))
		{
			go2->vel.SetZero();
			self_destruct = true;
		}
		return go2;
	}
	default:
		break;
	}

	return go2;
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

bool GeneralClass::getPanic()
{
	return panic;
}

void GeneralClass::setPanic(bool panic)
{
	this->panic = panic;
}

bool GeneralClass::getSelfdestruct()
{
	return this->self_destruct;
}

void GeneralClass::setSelfdestruct(bool self_destruct)
{
	this->self_destruct = self_destruct;
}