#ifndef SCENE_ASTEROID_H
#define SCENE_ASTEROID_H

#include "GameObject.h"
#include <vector>
#include "SceneBase.h"

class SceneAsteroid : public SceneBase
{
	static const int MAX_SPEED = 10;
	static const int BULLET_SPEED = 50;
	static const int MISSILE_SPEED = 20;
	static const int MISSILE_POWER = 1;
	static const float ROTATION_POWER;

public:
	SceneAsteroid();
	~SceneAsteroid();

	virtual void Init();
	virtual void Update(double dt);
	virtual void Render();
	virtual void Exit();

	void RenderGO(GameObject *go, float z= 0);
	GameObject* GetClosestGo(GameObject* current) const;
	GameObject* Enemyclosest(GameObject* ship) const;
	float ForceBtwObjects(GameObject* one, GameObject* two);
	GameObject* FetchGO();

	void UpdateMinion(double dt);

	
	int GetLevel();

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
	GameObject *m_ship;
	Vector3 m_force;
	Vector3 m_torque;
	GameObject* m_ghost;
	int m_objectCount;
	int m_lives;
	int m_score;

	int enemyHealth;

	float elapsedtime;
	float prevElapsed;

	float PI;
	float angle;

	float counter;
	float cooldown;

	float powerupchck;
	
	int minionCounter;
	int asteroidCounter;
	int enemyCounter;
	int miniasteroidCounter;
	int bosscounter;

	int Level;

	int asteroid_remaining;
	int enemy_remaining;
	int missiles_remaining;
	int boss_remaining;

	bool enemycheck;
	bool asteroidcheck;
	bool lvlchck;
	bool missilechck;

	bool scenechanger;

	bool activatespray;
	int timeLimit;
	

	Vector3 BHAcceleration;
	Vector3 Otheracceleration;
	
};

#endif