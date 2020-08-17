#ifndef SCENE_COLLISION_H
#define SCENE_COLLISION_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneCollision : public SceneBase
{
public:
	SceneCollision();
	~SceneCollision();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go);

	GameObject* FetchGO();
	void ReturnGO(GameObject *go);

	bool CheckCollision(GameObject* go1, GameObject* go2, float dt);
	void doCollisionResponse(GameObject* go1, GameObject* go2);

	void makeThinWall(float w, float h, const Vector3& pos, const Vector3& n);
	void makeThickWall(float w, float h, const Vector3& pos, const Vector3& n);
	std::vector<GameObject*> makepaddle(float w, float h, const Vector3& pos, const Vector3& n, float velX);
	
	
protected:

	//Physics
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ghost;
	GameObject *Powerup_dblpoints;
	GameObject *lifeUP;
	GameObject *m_pinball;
	GameObject *m_spring;
	GameObject *damper;
	std::vector<GameObject*> m_paddle;
	
	int m_objectCount;

	//Auditing
	float m1, m2;
	Vector3 u1, u2, v1, v2;
	Vector3 initMomentum;
	Vector3 finalMomentum;

	Vector3 m_gravity;

	Vector3 initKE;
	Vector3 finalKE;

	float size;

	float time;
	float freeze_time;
	float charge_time;
	float timer;

	int points;
	int lives;

	int counter_dbl;
	int counter_pts;
	int counter_freeze;
	int counter_slow;
	int counter_boost;
	int counter_recover;
	int counter_life;

	float move;
	float time_recover;

	bool GameOver;

	bool movebck;
	bool launch;
	bool livelost;
	bool stop;
};

#endif