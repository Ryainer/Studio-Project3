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

	Math::InitRNG();

	//Exercise 1: initialize m_objectCount
	m_objectCount = 0;
	m_ghost = new GameObject(GameObject::GO_BALL);

	//octagon
	//GameObject* wall = FetchGO();
	//wall->type = GameObject::GO_WALL;
	//wall->scale.Set(2,10,1);
	//wall->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0.f);
	//wall->dir.Set(0, 1, 0);

	//GameObject* pillar = FetchGO();
	//pillar->type = GameObject::GO_PILLAR;
	//pillar->scale.Set(3.f,3.f,1.f);
	//pillar->pos.Set(60.f, m_worldHeight / 2, 0.f);

	//GameObject* pillar2 = FetchGO();
	//pillar2->type = GameObject::GO_PILLAR;
	//pillar2->scale.Set(3.f, 3.f, 1.f);
	//pillar2->pos.Set(70.f, m_worldHeight / 2, 0.f);

	//float angle = Math::QUARTER_PI;
	//float wallLength = 20.f;
	//float radius = wallLength * 0.5f / tan(angle * 0.5f);
	//for (int i = 0; i < 8; ++i)
	//{
	//	GameObject* go = FetchGO();
	//	go->active = true;
	//	go->scale.Set(2.f, wallLength + 0.9f, 1.f);
	//	go->type = GameObject::GO_WALL;
	//	go->pos = Vector3(radius * cosf(i * angle) + m_worldWidth / 2, radius * sinf(i * angle) + m_worldHeight / 2, 0.f);
	//	go->dir = Vector3(cosf(i * angle), sinf(i * angle), 0.f);
	//	go->vel.Set(0, 0, 0);
	//}


	 estimatedTime = 0.f;
	 timeTaken = 0.f;
	 timeActive =  false;
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
	 {
		float temp1 = (go1->pos - go2->pos).Dot(go2->dir);
		float temp2 = (go1->vel).Dot(go2->dir);
		if ((temp1 > 0.f && temp2 > 0.f) || (temp1 < 0.f && temp2 < 0.f)) {
			return false;
		}
		
		Vector3 rPos = go1->pos - go2->pos;

		Vector3 axisX = go2->dir;
		Vector3 axisY(-axisX.y, axisX.x);

		return
			go2->scale.x * 0.5 + go1->scale.x > fabs(rPos.Dot(axisX)) &&
			go2->scale.y * 0.5 + go1->scale.x > fabs(rPos.Dot(axisY));
	 }
	case GameObject::GO_PILLAR:
	 {
		float combinedRadius = go1->scale.x + go2->scale.x;

		return (go1->pos - go2->pos).Length() < combinedRadius &&
			(go1->pos - go2->pos).Dot(go1->vel) < 0;
	 }
	}

	

	return false;
}

float SceneCollision::CheckCollison2(GameObject* go1, GameObject* go2) const
{
	if (go1->type != GameObject::GO_BALL)
		return false;

	switch (go2->type)
	{
	case GameObject::GO_BALL:
	 {
		Vector3 relVel = go1->vel - go2->vel;

		Vector3 disp = go1->pos - go2->pos;

		float combinedRadius = go1->scale.x + go2->scale.x;

		if (relVel.Dot(disp) > 0) // if balls are spearating , no collision return -1
		{
			return -1.f;
		}

		//discriminant
		float a = relVel.Dot(relVel);
		float b = (2 * relVel).Dot(disp);
		float c = disp.Dot(disp) - combinedRadius * combinedRadius;
		float determinant = b * b - 4 * a * c;
		if (determinant < 0)
			return -1;

		float t = ((-b) - sqrt(determinant)) / (2 * a);

		if (t < 0)
		{
			t = ((-b) + sqrt(determinant)) / (2 * a);
		}

		return t;
	 }
	}

	return -1;
}

void SceneCollision::doCollisionResponse(GameObject* go1, GameObject* go2)
{
	m1 = go1->mass;
	m2 = go2->mass;
	u1 = go1->vel;
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
	 }
	 break;
	case GameObject::GO_WALL:
	 {
		go1->vel = u1 - (2 * u1.Dot(go2->dir)) * go2->dir;
	 }
	 break;
	case GameObject::GO_PILLAR:
		Vector3 CollisionDir = go2->pos - go1->pos;
		Vector3 vec = (u1.Dot(CollisionDir) / CollisionDir.LengthSquared()) * CollisionDir;
		go1->vel = u1 - 2 * vec;
		go1->vel = 0.9f * go1->vel;
	}


	v1 = go1->vel;
	v2 = go2->vel;

	
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

	
	
	if(Application::IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed('0'))
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


		m_ghost->scale.Set(3, 3, 3);
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

		estimatedTime = FLT_MAX;

		int goListSize = m_goList.size();
		for (int i = 0; i < goListSize; ++i)
		{
			if (m_goList[i] == go)
			{
				continue;
			}
			float t = CheckCollison2(go, m_goList[i]);
			if (t > 0 && t < estimatedTime)
			{
				estimatedTime = t;
			}
			
		}
		
		timeTaken = 0;
		timeActive = true;
		
	}
	static bool bRButtonState = false;
	if (!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;
		m_ghost->active = true;
		m_ghost->scale.Set(size, size, size);
		m_ghost->pos.Set(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
		//Vector3 Mousepos(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
		GameObject* go = FetchGO();
		go->pos = m_ghost->pos;
		go->vel = m_ghost->pos - Mousepos;
		go->scale.Set(3, 3, 3);
		go->mass = 27;
		m_objectCount++;
		m_ghost->active = false;
		//Exercise 10: spawn large GO_BALL

	
	}

	if (bRButtonState)
	{
		float size = Math::Clamp((Mousepos - m_ghost->pos).Length(), 2.f, 10.f);
		m_ghost->scale.Set(size, size, size);
		m_ghost->mass = size * size * size;
	}

	

	static bool spacePressed = false;

	if (Application::IsKeyPressed(VK_SPACE) && !spacePressed)
	{
		spacePressed = true;

		GameObject* go = FetchGO();
		go->type = GameObject::GO_BALL;
		go->pos.Set(m_worldWidth * 0.9, 10, 0);
		go->vel.Set(0, 10, 0);
		go->scale.Set(5, 5, 5);
		go->mass = 125;
	}
	else if (!Application::IsKeyPressed(VK_SPACE))
	{
		spacePressed = false;
	}


	

	//Physics Simulation Section

	int size = m_goList.size();
	for (int i = 0; i < size; ++i)
	{
		GameObject* go = m_goList[i];
		if (go->active)
		{
			go->pos += go->vel * dt * m_speed;
		}
		if (go->type == GameObject::GO_BALL)
		{
			if (go->pos.x > m_worldWidth || go->pos.x < 0 || go->pos.y < 0 || go->pos.y > m_worldHeight)
			{
				ReturnGO(go);
				continue;
			}

			if (((go->pos.x + go->scale.x > m_worldWidth) && go->vel.x > 0) || ((go->pos.x - go->scale.x < 0) && go->vel.x < 0))
			{
				go->vel.x = -go->vel.x;
			}

			if (((go->pos.y + go->scale.y > m_worldHeight) && go->vel.y > 0) || ((go->pos.y - go->scale.y < 0) && go->vel.y < 0))
			{
				go->vel.y = -go->vel.y;
			}
		}
		
						
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
			//if (go2->active && CheckCollision(actor, actee, 0))
			float t = CheckCollison2(actor, actee);
			if(t >= 0 && t <= dt)
			{
				timeActive = false; 
				doCollisionResponse(actor, actee);
			}
		}
	}
	if (timeActive)
		timeTaken += dt;

}


void SceneCollision::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
	case GameObject::GO_PILLAR:
		//Exercise 4: render a sphere using scale and pos
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_WALL:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2f(go->dir.y, go->dir.x)), 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CUBE], false);
		modelStack.PopMatrix();
		//Exercise 11: think of a way to give balls different colors
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

	//std::ostringstream ss2;
	//ss2.precision(3);
	//ss2 << "Speed: " << m_speed;
	//RenderTextOnScreen(meshList[GEO_TEXT], ss2.str(), Color(0, 1, 0), 3, 0, 6);
	
	std::ostringstream ss;
	ss.precision(5);
	ss.str("");
	ss << "Time Taken: " << timeTaken;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 9);

	ss.precision(5);
	ss.str("");
	ss << "Est Time: " << estimatedTime;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 6);

	ss.precision(5);
	ss.str("");
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);

	/*ss.str("");
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
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 18);
	
	RenderTextOnScreen(meshList[GEO_TEXT], "Collision", Color(0, 1, 0), 3, 0, 0);*/
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
