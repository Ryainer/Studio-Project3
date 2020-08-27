#ifndef SCENE_COLLISION_H
#define SCENE_COLLISION_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

#include <../irrKlang/includes/irrKlang.h>
#include "CMyFileFactory.h"

#include "GeneralClass.h"
#include "AbilityManager.h"

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
	int GetLevel();
	bool CheckCollision(GameObject* go1, GameObject* go2, float dt);
	float CheckCollison2(GameObject* go1, GameObject* go2 ) const;
	void doCollisionResponse(GameObject* go1, GameObject* go2);
	GameObject* GetClosestGo(GameObject* current) const;
	float ForceBtwObjects(GameObject* one, GameObject* two);
	void makeThinWall(float w, float h, const Vector3& pos, const Vector3& n);
	void makeThickWall(float w, float h, const Vector3& pos, const Vector3& n);
	
	void UpdateMinion(double dt);

	//change this to fit our project
	int getAsteroidRemainder();
	int getEnemiesRemainder();
	int getMissilesRemainder();
	int getBossRemainder();

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

	//Auditing
	float m1, m2;
	Vector3 u1, u2, v1, v2;
	Vector3 initMomentum;
	Vector3 finalMomentum;

	Vector3 initKE;
	Vector3 finalKE;

	float size;
	
	//ported over
	GameObject* m_ship;
	Vector3 m_force;
	Vector3 m_torque;
	float counter;
	float cooldown;

	int m_lives;
	int enemyHealth;
	float elapsedtime;
	float prevElapsed;

	int Level;

	float angle;

	//chng names of this ltr
	int asteroid_remaining;
	int enemy_remaining;
	int missiles_remaining;
	int boss_remaining;

	int minionCounter;
	int asteroidCounter;
	int enemyCounter;
	int miniasteroidCounter;
	int bosscounter;

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