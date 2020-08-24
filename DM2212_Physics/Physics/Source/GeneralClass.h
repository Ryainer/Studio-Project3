#pragma once
#include"GameObject.h"
class GeneralClass
{
	static const int BULLET_SPEED = 50;

public:
	GeneralClass();
	~GeneralClass();

	bool generalAIchck(GameObject* go1, GameObject* go2);
	void generalAIresponse(GameObject* go2, GameObject*);
	void setAIGO(GameObject* go);
	GameObject* getAIGO();
	Vector3 getDir();
protected:
	GameObject* go;
	Vector3 RBCdir;
};

