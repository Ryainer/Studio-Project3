#pragma once
#include"GameObject.h"
class GeneralClass
{
	static const int BULLET_SPEED = 50;

public:
	GeneralClass();
	~GeneralClass();

	bool generalAIchck(GameObject* go1, GameObject* go2);
	GameObject* generalAIresponse(GameObject* go2, GameObject* PC);
	void setAIGO(GameObject* go);
	GameObject* getAIGO();
	Vector3 getDir();
	bool getPanic();
	void setPanic(bool panic);
	bool getSelfdestruct();
	void setSelfdestruct(bool self_destruct);
protected:
	GameObject* go;
	Vector3 RBCdir;
	bool panic;
	bool self_destruct;
};

