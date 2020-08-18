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
	float CheckCollison2(GameObject* go1, GameObject* go2 ) const;
	void doCollisionResponse(GameObject* go1, GameObject* go2);

	void makeThinWall(float w, float h, const Vector3& pos, const Vector3& n);
	void makeThickWall(float w, float h, const Vector3& pos, const Vector3& n);
	
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

};

#endif