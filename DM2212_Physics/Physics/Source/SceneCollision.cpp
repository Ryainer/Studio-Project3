#include "SceneCollision.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneCollision::SceneCollision()
{
}

SceneCollision::~SceneCollision()
{
}

void SceneCollision::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	size = 2;

	points = 0;

	lives = 4;

	//time to keep track off
	time = 100.f;
	freeze_time = 2.f;
	timer = 0.f;

	//counters to ensure limited num GOs spawn
	counter_dbl = 0;
	counter_pts = 0;
	counter_freeze = 0;
	counter_slow = 0;
	counter_boost = 0;
	counter_recover = 0;
	counter_life = 0;

	move = 0.f;

	time_recover = 3;

	GameOver = true;

	Math::InitRNG();

	movebck = false;

	launch = false;

	stop = false;

	m_gravity.Set(0, -9.8f, 0); //init gravity as 9.8ms-2 downwards

	//Exercise 1: initialize m_objectCount
	m_objectCount = 0;
	m_ghost = new GameObject(GameObject::GO_BALL);


	//left side
	makeThickWall(2, 180, Vector3(40.f, 10.f, 0.f), Vector3(1, 0, 0).Normalize());
	
	//topleft
	makeThickWall(2, 30, Vector3(54.f, 92.f, 0.f), Vector3(-0.5, 1, 0).Normalize());
	
	//top side
	makeThickWall(2, 55, Vector3(67.5f, 99.f, 0.f), Vector3(0, 1, 0).Normalize());
	
	////topright
	makeThickWall(2, 30, Vector3(82.f, 92.f, 0.f), Vector3(0.5, 1, 0).Normalize());
	
	////right side
	makeThickWall(2, 180, Vector3(95.f, 10.f, 0.f), Vector3(1, 0, 0).Normalize());
	
	//triangle bumper left side
	makeThinWall(2, 9, Vector3(51.f, 61.f, 0.f), Vector3(1, 0, 0).Normalize());
	makeThinWall(2, 10, Vector3(54.f, 61.f, 0.f), Vector3(1.5f, 1, 0).Normalize());
	makeThinWall(2, 6, Vector3(54.f, 56.f, 0.f), Vector3(0, 1, 0).Normalize());
	//triangle bumper right side
	makeThinWall(2, 8, Vector3(80.f, 41.f, 0.f), Vector3(1, 0, 0).Normalize());
	makeThinWall(2, 9, Vector3(77.f, 41.f, 0.f), Vector3(-1.5f, 1, 0).Normalize());
	makeThinWall(2, 5, Vector3(77.f, 37.f, 0.f), Vector3(0, 1, 0).Normalize());

	//partition btw game area and place where u shoot pinball out 
	makeThinWall(2, 20, Vector3(87.5f, 60.f, 0.f), Vector3(-1.5, 1, 0).Normalize());

	//short bumper left side
	makeThinWall(2, 10, Vector3(55.f, 23.f, 0.f), Vector3(0.5f , 1, 0).Normalize());

	//short bumper right side
	makeThinWall(2, 10, Vector3(85.f, 23.f, 0.f), Vector3(-0.5f, 1, 0).Normalize());
	
	//paddle to control ball
	m_paddle = makepaddle(3.5f, 10.5f, Vector3(m_worldWidth / 2, 10.f, 0.f), Vector3(0,1,0).Normalize(), move);

	//pinball
	m_pinball = FetchGO();
	m_pinball->active = true;
	m_pinball->type = GameObject::GO_BALL;
	m_pinball->scale.Set(1.5f,1.5f,1.5f);
	m_pinball->mass = 27.f;
	m_pinball->vel.SetZero();
	m_pinball->pos.Set(m_worldWidth/2, 75.f, 0.f);

	
}

GameObject* SceneCollision::FetchGO()
{
	//Exercise 2a: implement FetchGO()
	//Exercise 2b: increase object count every time an object is set to active
	for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject* go = (GameObject*)*it;
		if (go->active) continue; //chck if its active
		go->active = true;
		
		return go;
	}

	int previousMaxSize = m_goList.size();

	for (int i = 0; i < 10; ++i)
	{
		m_goList.push_back(new GameObject(GameObject::GO_BALL));
		
	}
	
	m_goList.at(previousMaxSize)->active = true;
	
	return m_goList.at(previousMaxSize);
}

void SceneCollision::ReturnGO(GameObject *go)
{
	//Exercise 3: implement ReturnGO()
	if (go->active == true)
	{
		go->active = false;
		m_objectCount--;
	}
}

bool SceneCollision::CheckCollision(GameObject* go1, GameObject* go2, float dt)
{
	if (go1->type != GameObject::GO_BALL)
		return false;
	switch(go2->type)
	{
	case GameObject::GO_BALL:
	 {
		Vector3 relativeVel = go1->vel - go2->vel;
		Vector3 displacementVel = go2->pos - go1->pos;
		if (relativeVel.Dot(displacementVel) <= 0) return false;
		return displacementVel.LengthSquared() <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x);
	 }
	case GameObject::GO_WALL:
	case GameObject::GO_PADDLEWALL:
	case GameObject::GO_POINTS:
	case GameObject::GO_DBLPOINT:
	case GameObject::GO_TIMERECOVER:
	case GameObject::GO_1UPLIFE:
	 {
		Vector3 rPos = go1->pos - go2->pos;

		Vector3 axisX = go2->dir;
		Vector3 axisY(-axisX.y, axisX.x);

		if (rPos.Dot(axisX) > 0) axisX = -axisX;

		return
			go1->vel.Dot(axisX) >= 0 &&
			go2->scale.x * 0.5 + go1->scale.x > -rPos.Dot(axisX) &&
			go2->scale.y * 0.5  > fabs(rPos.Dot(axisY));
	 }
	case GameObject::GO_PILLAR:
	case GameObject::GO_PADDLEPILLAR:
	case GameObject::GO_TIMESTOP:
	case GameObject::GO_SLOWDOWN:
	case GameObject::GO_BOOST:
	 {
		
		float combinedRadius = go1->scale.x + go2->scale.x;

		return (go1->pos - go2->pos).Length() < combinedRadius &&
			(go1->pos - go2->pos).Dot(go1->vel) < 0;
	 }
	}

	

	return false;
}

void SceneCollision::doCollisionResponse(GameObject* go1, GameObject* go2)
{
	m1 = go1->mass;
	m2 = go2->mass;
	u1 =  go1->vel;
	u2 = go2->vel;
	go1->momentum = (m1 * u1);
	go2->momentum = (m2 * u2);

	initMomentum = (m1 * u1) + (m2 * u2);
	finalMomentum = (m1 * v1) + (m2 * v2);

	switch (go2->type)
	{
	case GameObject::GO_BALL:
	 {
		
		Vector3 collisiondir = go1->pos - go2->pos;
		Vector3 vec = ((u1 - u2).Dot(collisiondir) / (collisiondir).LengthSquared()) * (collisiondir);

		go1->vel = u1 - (2 * m2 / (m1 + m2)) * vec;
		go2->vel = u2 - (2 * m1 / (m2 + m1)) * (-vec);
		go1->vel = 0.9f * go1->vel;
		go2->vel = 0.9f * go2->vel;
	 }
	 break;
	case GameObject::GO_WALL:
	 {
		
		go1->vel = u1 - (2 * u1.Dot(go2->dir)) * go2->dir;
		go1->vel = 0.9f * go1->vel;
	 }
	 break;
	case GameObject::GO_PADDLEWALL:
	 {
		go1->vel -= u1 - (2 * u1.Dot(go2->dir)) * go2->dir * 2.5f ;
		go1->vel = 4.5f * go1->vel;
	 }
	case GameObject::GO_PILLAR:
	 {
		Vector3 CollisionDir = go2->pos - go1->pos;
		Vector3 vec = (u1.Dot(CollisionDir) / CollisionDir.LengthSquared()) * CollisionDir;
		go1->vel = u1 - 2 * vec;
		go1->vel = 0.9f * go1->vel;
	 }
	 break;
	case GameObject::GO_PADDLEPILLAR:
	 {
		//added relative vel for pillar for the paddle
		Vector3 relativeVel = go2->vel - go1->vel;
		Vector3 collisiondir = (go2->pos - go1->pos).Normalize();
		Vector3 vec = ((relativeVel).Dot(collisiondir) / (collisiondir).LengthSquared()) * (collisiondir);
		
		go1->vel = (u1 + 2 * vec) * 2.5f;
	 }
	break;

	case GameObject::GO_TIMESTOP:
	 {
		if (go2->active)
		{
			freeze_time = 0;
			ReturnGO(go2);
			counter_freeze -= 1;
		}
	 }
	 break;
	case GameObject::GO_POINTS:
	 {
		if (go2->active)
		{
			points += 1;
			go1->vel -= 2.5f * go1->vel;
			ReturnGO(go2);
			counter_pts -= 1;
		}
	 }
	 break;
	case GameObject::GO_DBLPOINT:
	 {
		if (go2->active)
		{
			points += Math::RandIntMinMax(2, 5);
			go1->vel -= 1.5f * go1->vel;
			ReturnGO(go2);
		}
	 }
	case GameObject::GO_SLOWDOWN:
	 {
		if (go2->active)
		{
			go1->vel = 0.5f * go1->vel;
			ReturnGO(go2);
			counter_slow -= 1;
		}
	 }
	case GameObject::GO_BOOST:
	 {
		if (go2->active)
		{
			go1->vel = 1.f * go1->vel;
			ReturnGO(go2);
			counter_boost -= 1;
		}
	 }
	case GameObject::GO_TIMERECOVER:
	 {
		if (go2->active)
		{
			time += time_recover;
			go1->vel -= 2.5f * go1->vel;
			ReturnGO(go2);
			counter_recover -= 1;
		}
	 }
	case GameObject::GO_1UPLIFE:
	 {
		if (go2->active)
		{
			lives += 1;
			go1->vel -= 2.5f * go1->vel;
			ReturnGO(go2);
			
		}
	 }
	}


	v1 = go1->vel;
	v2 = go2->vel;

	
}

void SceneCollision::makeThinWall(float w, float h, const Vector3& pos, const Vector3& n)
{
	GameObject* go = FetchGO();
	go->type = GameObject::GO_WALL;
	go->pos = pos;
	go->dir = n;
	go->scale.Set(w, h, 1.f);
	

	Vector3 newN = Vector3(-n.y, n.x, 0);
	float halfW = w * 0.5f;
	float halfH = h * 0.5f;
	GameObject* pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->pos = pos + newN * halfH;
	pillar->scale.Set(halfW, halfW, 0);
	

	pillar = FetchGO();
	pillar->type = GameObject::GO_PILLAR;
	pillar->pos = pos - newN * halfH;
	pillar->scale.Set(halfW, halfW, 0);
}

void SceneCollision::makeThickWall(float w, float h, const Vector3& pos, const Vector3& n)
{
	GameObject* go = FetchGO();

	go->active = true;
	go->type = GameObject::GO_WALL;
	go->scale.Set(w, h, 1);
	go->pos = pos;
	go->dir = n;
	go->vel.SetZero();

	go = FetchGO();
	go->active = true;
	go->type = GameObject::GO_WALL;
	go->scale.Set(h, w, 1);
	go->pos = pos;
	go->dir.Set(-n.y, n.x, 0);
	go->vel.SetZero();

	Vector3 right(-n.y, n.x, 0);
	float halfHeight = h / 2;
	float halfWidth = w / 2;

	go = FetchGO();
	go->active = true;
	go->type = GameObject::GO_PILLAR;
	go->scale.Set(0.01f, 0.01f, 1);
	go->pos = pos + right * halfHeight + n * halfWidth;
	go->vel.SetZero();

	go = FetchGO();
	go->active = true;
	go->type = GameObject::GO_PILLAR;
	go->scale.Set(0.01f, 0.01f, 1);
	go->pos = pos - right * halfHeight + n * halfWidth;
	go->vel.SetZero();

	go = FetchGO();
	go->active = true;
	go->type = GameObject::GO_PILLAR;
	go->scale.Set(0.01f, 0.01f, 1);
	go->pos = pos + right * halfHeight - n * halfWidth;
	go->vel.SetZero();

	go = FetchGO();
	go->active = true;
	go->type = GameObject::GO_PILLAR;
	go->scale.Set(0.01f, 0.01f, 1);
	go->pos = pos - right * halfHeight - n * halfWidth;
	go->vel.SetZero();
}

std::vector<GameObject*> SceneCollision::makepaddle(float w, float h, const Vector3& pos, const Vector3& n, float velX)
{
	//make a vector to store and update all the objs at once
	std::vector<GameObject*> paddle;
	
	GameObject* go = FetchGO();
	go->type = GameObject::GO_PADDLEWALL;
	go->pos = pos;
	go->dir = n;
	go->scale.Set(w, h, 1.f);
	
	//go->pos += go->vel;
	paddle.push_back(go);


	Vector3 newN = Vector3(-n.y, n.x, 0);
	float halfW = w * 0.5f;
	float halfH = h * 0.5f;
	GameObject* pillar = FetchGO();
	pillar->type = GameObject::GO_PADDLEPILLAR;
	pillar->pos = pos + newN * halfH;
	pillar->scale.Set(halfW, halfW, 0);
	//pillar->pos += go->vel;
	paddle.push_back(pillar);

	pillar = FetchGO();
	pillar->type = GameObject::GO_PADDLEPILLAR;
	pillar->pos = pos - newN * halfH;
	pillar->scale.Set(halfW, halfW, 0);
	//pillar->pos += go->pos;
	paddle.push_back(pillar);

	

	return paddle;
}


void SceneCollision::Update(double dt)
{
	SceneBase::Update(dt);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	double x, y, w, h;

	Application::GetCursorPos(&x, &y);
	w = Application::GetWindowWidth();
	h = Application::GetWindowHeight();

	if (freeze_time >= 2.f)
	{
		time -= dt * 2.f;
		
	}
	else if (freeze_time < 2)
	{
		freeze_time += dt * 2.f;
	}
	if (time <= 0)
	{
		time = 0.f;
		GameOver = false;
	}

	if (Application::IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if (Application::IsKeyPressed('0'))
	{
		m_speed += 0.1f;
	}

	//Mouse Section
	static bool bLButtonState = false;
	Vector3 Mousepos(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;

		m_ghost->active = true;
		m_ghost->pos.Set(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);


		m_ghost->scale.Set(size, size, size);

	}
	else if (bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;

		GameObject* go = FetchGO();

		go->pos = m_ghost->pos;
		go->vel = m_ghost->pos - Mousepos;

		go->scale = m_ghost->scale;
		go->mass = m_ghost->mass;
		m_objectCount++;
		m_ghost->active = false;
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
		m_ghost->active = true;
		m_ghost->scale.Set(3, 3, 3);
		m_ghost->pos.Set(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
		//Vector3 Mousepos(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
		GameObject* go = FetchGO();
		go->type = m_ghost->type;
		go->pos = m_ghost->pos;
		go->vel = m_ghost->pos - Mousepos;
		go->scale.Set(1.5f, 1.5f, 1.5f);
		go->mass = 27;
		m_objectCount++;
		m_ghost->active = false;
		//Exercise 10: spawn large GO_BALL
	}

	if (bLButtonState)
	{
		float size = Math::Clamp((Mousepos - m_ghost->pos).Length(), 2.f, 10.f);
		m_ghost->scale.Set(size, size, size);
		m_ghost->mass = size * size * size;
	}
	//respawns ball
	if (lives > 0 &&!m_pinball->active)
	{
		m_pinball = FetchGO();
		m_pinball->active = true;
		m_pinball->type = GameObject::GO_BALL;
		m_pinball->scale.Set(1.5f, 1.5f, 1.5f);
		m_pinball->mass = 27.f;
		m_pinball->vel.SetZero();
		m_pinball->pos.Set(61.f, 70.5f, 0.f);
		lives--;
	}


	
	//affect vel and clamps speed
	if (m_pinball->active)
	{
		m_pinball->vel += m_gravity * dt * m_speed;
		Math::Clamp(m_pinball->vel.x, 5.f, 8.f);
		Math::Clamp(m_pinball->vel.y, 5.f, 8.f);
	}

	if (Application::IsKeyPressed('A'))
	{
		move -= dt * 5.5f;
		if (move <= -0.5f)
		{
			move = -0.5f;
		}
	}
	if (Application::IsKeyPressed('D'))
	{
		move += dt * 5.5f;
		if (move >= 0.5f)
		{
			move = 0.5f;
		}
	}
	else if (!Application::IsKeyPressed('A'))
	{
		move = 0.f;
	}
	if (Application::IsKeyPressed(VK_SPACE))
	{
		m_pinball->vel.y -= 3.f * dt;

	}
	int size = m_goList.size();
	for (int i = 0; i < m_paddle.size(); i++)
	{
		m_paddle[i]->vel.x = move;
		m_paddle[i]->pos += m_paddle[i]->vel;
		

		GameObject* go2 = nullptr;

		for (int j = 0; j < size; ++j)
		{
			go2 = m_goList[j];

			if (go2->type == GameObject::GO_WALL)
			{
				Vector3 rPos = m_paddle[i]->pos - go2->pos;

				Vector3 axisX = go2->dir;
				Vector3 axisY(-axisX.y, axisX.x);

				if (rPos.Dot(axisX) > 0)
					axisX = -axisX;
				
				if (m_paddle[i]->vel.Dot(axisX) >= 0 &&
					go2->scale.x * 0.5 + m_paddle[i]->scale.x > -rPos.Dot(axisX) &&
					go2->scale.y * 0.5 > fabs(rPos.Dot(axisY)))
				{
					//std::cout << "ah";
					movebck = true;
					
				}
			}

			if (movebck == true)
			{
				Vector3 pos;
				pos.Set(65.f, 10.f, 0);
				//pillar->pos = pos + newN * halfH;
				m_paddle[0]->pos = pos;
				Vector3 newN = Vector3(-m_paddle[0]->dir.y, m_paddle[0]->dir.x, 0);
				m_paddle[1]->pos = pos + newN * (10.5f * 0.5f);
				m_paddle[2]->pos = pos - newN * (10.5f * 0.5f);
				movebck = false;
			}
		}

	
	}
	//spawns GOs in the pinball field
	if (timer <= 2.f && counter_dbl <= 0)
	{
		Powerup_dblpoints = FetchGO();
		Powerup_dblpoints->active = true;
		Powerup_dblpoints->type = GameObject::GO_DBLPOINT;
		Powerup_dblpoints->scale.Set(2.5f, 2.5f, 0.f);
		Powerup_dblpoints->pos.Set(Math::RandFloatMinMax(55.f, m_worldWidth/2), 
			Math::RandFloatMinMax(45.f, (m_worldHeight / 2 - 10.f)), 0);
		Powerup_dblpoints->vel.SetZero();
		counter_dbl += 1;
	}
	if (timer <= 1.f && counter_life <= 0) 
	{
		lifeUP = FetchGO();
		lifeUP->active = true;
		lifeUP->type = GameObject::GO_1UPLIFE;
		lifeUP->scale.Set(1.f, 1.f, 1.f);
		lifeUP->pos.Set(Math::RandFloatMinMax(40.f, 85.f), Math::RandFloatMinMax(67.f, 85.f), 0.f);
		lifeUP->vel.SetZero();
		counter_life += 1;
	}
	else if(timer >= 2.f) //helps to ensure the GOs dont stay in one spot making it hard to catch
	{
	
		ReturnGO(Powerup_dblpoints);
		ReturnGO(lifeUP);
		counter_life = 0;
		timer = 0;
		counter_dbl = 0;
	}
	
	timer += dt * 1.5f;

	if (counter_freeze <= 0)
	{
		GameObject* PowerupTest_Time = FetchGO();
		PowerupTest_Time->active = true;
		PowerupTest_Time->type = GameObject::GO_TIMESTOP;
		PowerupTest_Time->scale.Set(1.5f, 1.5f, 0.f);
		PowerupTest_Time->pos.Set(45.f, Math::RandFloatMinMax(30.f, (m_worldHeight / 2 + 10.f)), 0);
		counter_freeze += 1;
	}
	
	if (counter_pts <= 2)
	{
		GameObject* PowerupTest_points = FetchGO();
		PowerupTest_points->active = true;
		PowerupTest_points->type = GameObject::GO_POINTS;
		PowerupTest_points->scale.Set(2.5f, 2.5f, 0.f);
		PowerupTest_points->pos.Set(Math::RandFloatMinMax(55.f, 85.f), Math::RandFloatMinMax(25.f, (m_worldHeight / 2)), 0);
		counter_pts += 1;
	}

	if (counter_slow <= 1)
	{
		damper = FetchGO();
		damper->active = true;
		damper->type = GameObject::GO_SLOWDOWN;
		damper->scale.Set(1.f, 1.f, 1.f);
		damper->vel.SetZero();
		damper->pos.Set(Math::RandFloatMinMax(55.f, m_worldWidth / 2 - 10.f), Math::RandFloatMinMax(42.f, m_worldHeight/2), 0.f);
		counter_slow += 1;
	}


	if (counter_boost <= 1)
	{
		GameObject* boost = FetchGO();
		boost->active = true;
		boost->type = GameObject::GO_BOOST;
		boost->scale.Set(1.5f, 1.5f, 1.5f);
		boost->vel.SetZero();
		boost->pos.Set(Math::RandFloatMinMax(65.f, m_worldWidth / 2), Math::RandFloatMinMax(52.f, m_worldHeight / 2), 0.f);
		counter_boost += 1;
	}

	if (counter_recover <= 1)
	{
		GameObject* recover = FetchGO();
		recover->active = true;
		recover->type = GameObject::GO_TIMERECOVER;
		recover->scale.Set(2.5f, 2.5f, 2.5f);
		recover->vel.SetZero();
		recover->pos.Set(Math::RandFloatMinMax(60.f, 75.f), Math::RandFloatMinMax(32.f, 90.f), 0.f);
		counter_recover += 1;
	}

	//Physics Simulation Section
	for (int i = 0; i < size; ++i)
	{
		GameObject* go = m_goList[i];
		if (go->active)
		{

			go->pos += go->vel * dt * m_speed ;
		}
		if (go->type == GameObject::GO_BALL)
		{
			go->vel += m_gravity * dt * m_speed;
		

			if (go->pos.x > m_worldWidth || go->pos.x < 0 || go->pos.y < 0 || go->pos.y > m_worldHeight)
			{
				ReturnGO(go);
				ReturnGO(m_pinball);
				continue;
			}

			if (((go->pos.x + go->scale.x > m_worldWidth) && go->vel.x > 0) || ((go->pos.x - go->scale.x < 0) && go->vel.x < 0))
			{
				go->vel.x = -go->vel.x;
			}

			if (((go->pos.y + go->scale.y > m_worldHeight) && go->vel.y > 0) )
			{
				go->vel.y = -go->vel.y;
			}
		}
	/*	if (go->type == GameObject::GO_PADDLEPILLAR)
		{
			if (go->pos.x > 76.f )
			{
				std::cout << "ahhhh";
				go->pos.x = 75.f;
			}
		}
		if (go->type == GameObject::GO_PADDLEWALL)
		{
			if (go->pos.x > 75.f )
			{
				std::cout << "ahhhh";
				go->pos.x = 74.f;
			}
		}*/
		

		GameObject* go2 = nullptr;
		for (int j = i + 1; j < size; ++j)
		{
			go2 = m_goList[j];
			GameObject* actor(go), * actee(go2);
			if (go->type != GameObject::GO_BALL)
			{
				actor = go2;
				actee = go;
			}
			if (go2->active && CheckCollision(actor, actee, 0))
			{
				doCollisionResponse(actor, actee);
			}

		}
	}


			//Exercise 8b: store values in auditing variables

				//Exercise 10: handle collision using momentum swap instead

				//Exercise 12: improve inner loop to prevent double collision

			//Exercise 13: improve collision detection algorithm [solution to be given later] 

}


void SceneCollision::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
	case GameObject::GO_PILLAR:
	case GameObject::GO_PADDLEPILLAR:
	case GameObject::GO_TIMESTOP:
		//Exercise 4: render a sphere using scale and pos
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_WALL:
	case GameObject::GO_PADDLEWALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2f(go->dir.y, go->dir.x)), 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CUBE], false);
		modelStack.PopMatrix();
		//Exercise 11: think of a way to give balls different colors
		break;
	case GameObject::GO_POINTS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_HEALTHRECOVER], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_DBLPOINT:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BULLETSPRAY], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SLOWDOWN:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BLACKHOLE], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BOOST:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_REPELLER], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_TIMERECOVER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ASTEROID], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_1UPLIFE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_LIFEUP], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneCollision::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Calculating aspect ratio
	/*m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();*/

	// Projection matrix : Orthographic Projection
	Mtx44 projection;
	projection.SetToOrtho(0, m_worldWidth, 0, m_worldHeight, -10, 10);
	projectionStack.LoadMatrix(projection);
	
	// Camera matrix
	viewStack.LoadIdentity();
	viewStack.LookAt(
						camera.position.x, camera.position.y, camera.position.z,
						camera.target.x, camera.target.y, camera.target.z,
						camera.up.x, camera.up.y, camera.up.z
					);
	// Model matrix : an identity matrix (model will be at the origin)
	modelStack.LoadIdentity();
	
	RenderMesh(meshList[GEO_AXES], false);

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
	}

	if (m_ghost->active == true)
	{
		RenderGO(m_ghost);
	}

	

	//On screen text

	//Exercise 5: Render m_objectCount

	//Exercise 8c: Render initial and final momentum

	std::ostringstream ss2;
	std::ostringstream ss;
	/*ss2.precision(3);
	ss2 << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss2.str(), Color(0, 1, 0), 3, 0, 6);
	
	
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);

	ss.str("");
	ss.precision(5);
	ss << "Object count: " << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 9);

	ss.str("");
	ss.precision(5);
	ss << "Init Momentum: " << initMomentum ;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 0, 11);

	ss.str("");
	ss.precision(5);
	ss << "Final Momentum: " << finalMomentum;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 0, 13);
	
	ss.str("");
	ss.precision(5);
	ss << "Initial KE: " << 0.5 * m1 * u1.Dot(u1) + 0.5 * m2 * u2.Dot(u2);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 15);

	ss.str("");
	ss.precision(5);
	ss << "Final KE: " << 0.5 * m1 * v1.Dot(u1) + 0.5 * m2 * v2.Dot(u2);
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 18);*/

	ss.str("");
	ss.precision(5);
	ss << "Time left: " << time;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 55);

	ss.str("");
	ss.precision(5);
	ss << "Points: " << points;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 50);

	ss.str("");
	ss.precision(5);
	ss << "Freeze Time left: " << freeze_time; //to remove
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 52);

	ss.str("");
	ss.precision(5);
	ss << "Lives: " << lives;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 48);

	if (GameOver == false || lives <= 0)
	{
		ss.str("");
		ss.precision(5);
		ss << "GAME OVER"; 
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 25.f, 25.f);
		ReturnGO(m_pinball);
	}
	
	RenderTextOnScreen(meshList[GEO_TEXT], "Collision", Color(0, 1, 0), 3, 0, 0);
}

void SceneCollision::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ghost)
	{
		delete m_ghost;
		m_ghost = NULL;
	}
}
