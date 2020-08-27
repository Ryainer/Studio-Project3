#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Vector3.h"

struct GameObject
{
	enum GAMEOBJECT_TYPE
	{
		GO_NONE = 0,
		GO_BALL,
		GO_CUBE,
		GO_WALL,
		GO_PILLAR,
		GO_ASTEROID,
		GO_SHIP,
		GO_BULLET,
		GO_ENEMY,
		GO_ENEMY_BULLET,
		GO_MISSILE,
		GO_BLACKHOLE,
		GO_REPELLER,
		GO_SMALLSHIP,
		GO_HEALTHPOWERUP,
		GO_BULLETPOWERUP,
		GO_MINIASTEROID,
		GO_BOSS,
		GO_BOSS_MISSILE,
		GO_MINION_BULLET,
		GO_WBC,
		GO_RBC,
		GO_WBC_PROJECTILES,
		GO_PROJECTILE,
		GO_BOOMERANG,
		GO_TOTAL, //must be last
	};
	GAMEOBJECT_TYPE type;
	Vector3 pos;
	Vector3 vel;
	Vector3 scale;
	Vector3 dir;
	Vector3 momentum;
	Vector3 normal;
	float momentofinertia;
	float angularVelocity;
	float health;
	float bounceTime;
	bool active;
	float mass;
	float angle;

	float range;//range for abilities, use when applicable
	double cooldown;//cooldown for abilities. should be 0 unless otherwise stated.
	double iframesRead;//this iframe will be used as a reference
	double iframesWrite;//this iframe's value will be changed to check if a player/enemy has recently taken damage
	//these new variables can be changed if you deem them too rigid/useless/hard to use
	//iframes for enemies and players, cooldown for abilities

	GameObject(GAMEOBJECT_TYPE typeValue = GO_BALL);
	~GameObject();
};

#endif