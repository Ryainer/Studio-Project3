#ifndef SCENE_COLLISION_H
#define SCENE_COLLISION_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

#include <../irrKlang/includes/irrKlang.h>
#include "CMyFileFactory.h"

#include "GeneralClass.h"
#include "AbilityManager.h"

enum GAMESTATES
{
	S_MAIN,
	S_WIN,
	S_LOSE,
	S_GAME,
	S_CREDITS,
	S_INSTRUCTIONS
};

class SceneCollision : public SceneBase
{
	static const int MAX_SPEED = 10;
	static const int BULLET_SPEED = 50;
	static const int MISSILE_SPEED = 20;
	static const int MISSILE_POWER = 1;
	static const float ROTATION_POWER;

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
	float CheckCollison2(GameObject* go1, GameObject* go2, float dt) const;
	void doCollisionResponse(GameObject* go1, GameObject* go2);
	GameObject* GetClosestGo(GameObject* current) const;
	float ForceBtwObjects(GameObject* one, GameObject* two);

	
	void UpdateMinion(double dt);



protected:

	//Physics
	std::vector<GameObject *> m_goList;
	float m_speed;
	float m_worldWidth;
	float m_worldHeight;
	GameObject *m_ghost;
	
	int m_objectCount;

	float estimatedTime;
	float timeTaken;
	bool timeActive;

	float healthX = 20.f;

	//Auditing
	float m1, m2;
	Vector3 u1, u2, v1, v2;
	Vector3 initMomentum;
	Vector3 finalMomentum;

	Vector3 initKE;
	Vector3 finalKE;

	GAMESTATES g_eGameStates;

	float size;
	
	//ported over
	GameObject* m_virus;
	Vector3 m_force;
	Vector3 m_torque;
	float counter;
	float cooldown;

	int m_lives;
	int enemyHealth;
	float elapsedtime;
	float prevElapsed;

	int minioncounter;

	double elapsedTime;

	bool gameStart;

	int m_score;
	
	int Level;
	float m_angle;
	float angle;

	float timerCount;
	bool timerUp;
	bool activated;

	

	int randomenemyspawn;

	bool enemycheck;
	bool asteroidcheck;
	bool lvlchck;
	bool missilechck;

	bool scenechanger;


	double bounceTime;

	ISoundEngine* engine;
	CMyFileFactory* factory;

	//stuff for abilities pls no touch
	AbilityManager ab;

	GeneralClass* AI;

	//variables for game logic and mechanics go here
	int biomass;//this is the resource number used for upgrading abilities and going to the next level

	Vector3 BHAcceleration;
	Vector3 Otheracceleration;
};

#endif