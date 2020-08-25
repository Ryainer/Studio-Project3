#pragma once
#include"GameObject.h"
class GeneralClass
{
	static const int BULLET_SPEED = 50;

public:
	GeneralClass();
	~GeneralClass();

	bool generalAIchck(GameObject* go1, GameObject* go2);
	GameObject* generalAIresponse(GameObject* go2);
	void setAIGO(GameObject* go);
	GameObject* getAIGO();
	Vector3 getDir();
	bool getPanic();
protected:
	GameObject* go;
	Vector3 RBCdir;
	bool panic;
};

