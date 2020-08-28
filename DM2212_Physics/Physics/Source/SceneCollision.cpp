#include "SceneCollision.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

using namespace irrklang;

const float SceneCollision::ROTATION_POWER = 3.f;
static float i_frames = 0.f;
SceneCollision::SceneCollision()
{
}

SceneCollision::~SceneCollision()
{
}

void SceneCollision::Init()
{
	SceneBase::Init();

	// Start sound engine 
	engine = createIrrKlangDevice();
	factory = new CMyFileFactory(); // File factory is to allow us to use our sound files

	if (!engine)
	{
		return;
	}

	engine->addFileFactory(factory);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	size = 2;

	Math::InitRNG();

	//Exercise 1: initialize m_objectCount
	m_objectCount = 1; //this includes the player, enemies and neutral entities
	m_ghost = new GameObject(GameObject::GO_BALL);

	//ported over
	m_ship = new GameObject(GameObject::GO_SHIP);
	m_ship->active = true;
	m_ship->type = GameObject::GO_SHIP;
	m_ship->scale.Set(5.5f, 5.5f, 5.5f);
	m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0.f);
	m_ship->vel.Set(0.f, 0.f, 0.f);

	m_ship->momentofinertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	m_ship->angularVelocity = 0.f;

	m_ship->health = 20;

	m_lives = 3;

	AI = new GeneralClass();

	minionCounter = 0;
	cooldown = 10.f;
	enemyHealth = 3;

	asteroidCounter = 0;
	enemyCounter = 0;
	counter = 0;
	miniasteroidCounter = 0;
	bosscounter = 0;

	scenechanger = false;

	m_torque.Set(0, 0, 0);

	angle = 0;

	biomass = 0;

	elapsedtime = prevElapsed = 0;

	bounceTime = 0.0;

	enemy_remaining = 1;

	//ported part ends here

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

int SceneCollision::GetLevel()
{
	return Level;
}

int SceneCollision::getAsteroidRemainder()
{
	return asteroid_remaining;
}

int SceneCollision::getEnemiesRemainder()
{
	return enemy_remaining;
}

int SceneCollision::getMissilesRemainder()
{
	return missiles_remaining;
}

int SceneCollision::getBossRemainder()
{
	return boss_remaining;
}

bool SceneCollision::CheckCollision(GameObject* go1, GameObject* go2, float dt)
{
	if (go1->type != GameObject::GO_PROJECTILE)
		return false;
	else if (go1->type != GameObject::GO_BOOMERANG)
		return false;

	switch(go2->type)
	{
	case GameObject::GO_BALL:
	case GameObject::GO_BOOMERANG:
	case GameObject::GO_PROJECTILE:
	{
		Vector3 relativeVel = go1->vel - go2->vel;
		Vector3 displacementVel = go2->pos - go1->pos;

		if (go1->iframesWrite != go1->iframesRead)//if there has been a change in iframes, cotinue changing
		{
			go1->iframesWrite -= dt;

		}

		if (relativeVel.Dot(displacementVel) <= 0) return false;
		return displacementVel.LengthSquared() <= (go1->scale.x + go2->scale.x) * (go1->scale.x + go2->scale.x);
	}
	break;
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
	 break;
	case GameObject::GO_PILLAR:
	 {
		float combinedRadius = go1->scale.x + go2->scale.x;

		return (go1->pos - go2->pos).Length() < combinedRadius &&
			(go1->pos - go2->pos).Dot(go1->vel) < 0;
	 }
	 break;
	}

	

	return false;
}

float SceneCollision::CheckCollison2(GameObject* go1, GameObject* go2, float dt) const
{
	

	switch (go1->type)
	{
	case GameObject::GO_PROJECTILE:
	case GameObject::GO_BOOMERANG:
	 {
		switch (go2->type)
		{
		case GameObject::GO_WBC:
		case GameObject::GO_RBC:
		case GameObject::GO_TCELLS:
		case GameObject::GO_DEADCELLS:
		  {
			 Vector3 relVel = go1->vel - go2->vel;

			 Vector3 disp = go1->pos - go2->pos;

			 float combinedRadius = go1->scale.x + go2->scale.x;

			 if (relVel.Dot(disp) > 0) // if balls are spearating , no collision return -1
			 {
				return -1.f;
			 }


			 if (go1->iframesWrite != go1->iframesRead)//if there has been a change in iframes, cotinue changing
			 {
				go1->iframesWrite -= dt;

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
	 }
	 break;
	default:
		return -1;
		break;
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
	 {
		Vector3 CollisionDir = go2->pos - go1->pos;
		Vector3 vec = (u1.Dot(CollisionDir) / CollisionDir.LengthSquared()) * CollisionDir;
		go1->vel = u1 - 2 * vec;
		go1->vel = 0.9f * go1->vel;
		break;
	 }
	

	case GameObject::GO_WBC:
	case GameObject::GO_RBC:
	case GameObject::GO_TCELLS:
	case GameObject::GO_DEADCELLS:
	 {
		if (go1->active && go2->active)
		{
			ReturnGO(go1);
			go2->health -= 1;
			biomass++;
			std::cout << "biomass: " << biomass << std::endl;
			std::cout << go2->health << std::endl;
		}
	 }
	}
	 

	v1 = go1->vel;
	v2 = go2->vel;

	
}

//ported over
GameObject* SceneCollision::GetClosestGo(GameObject* current) const
{
	GameObject* closestGO = nullptr;
	float closestDist = FLT_MAX;
	int goSize = m_goList.size();
	for (int i = 0; i < goSize; ++i)
	{
		if (m_goList[i]->active && (m_goList[i]->type == GameObject::GO_ASTEROID || m_goList[i]->type == GameObject::GO_ENEMY))
		{
			float currentDist = (m_goList[i]->pos - current->pos).LengthSquared();
			if (currentDist < closestDist)
			{
				closestGO = m_goList[i];
				closestDist = currentDist;
			}
		}
	}

	return closestGO;
}

/*GameObject* SceneAsteroid::Enemyclosest(GameObject* ship) const //for boss ship missile to track player
{
	GameObject* atkship = nullptr;
	float closestDist = FLT_MAX;

	float currentDist = (m_ship->pos - ship->pos).LengthSquared();
	if (currentDist < closestDist)
	{
		atkship = m_ship;
		closestDist = currentDist;
	}

	return atkship;
}
*/

void SceneCollision::UpdateMinion(double dt)
{
	for (size_t i = 0; i < m_goList.size(); ++i)
	{
		GameObject* go = m_goList[i];
		if (go->active && go->type == GameObject::GO_SMALLSHIP)
		{
			for (size_t j = 0; j < m_goList.size(); ++j)
			{
				GameObject* instance = m_goList[j];

				if (instance != go && instance->active && instance->type == GameObject::GO_SMALLSHIP)
				{

					Vector3 dispVec = go->pos - instance->pos;
					float sqDist = dispVec.LengthSquared();
					float G = 5.f;

					try
					{
						Vector3 Force = G * go->mass * instance->mass / sqDist * -dispVec.Normalized();
						instance->vel += Force * (1.f / go->mass) * dt * m_speed;

					}
					catch (DivideByZero e)
					{

					}
				}
			}

			Vector3 DirVec = (m_ship->pos - (m_ship->dir * 10)) - go->pos;
			try
			{
				DirVec.Normalized();
			}
			catch (DivideByZero d)
			{
				DirVec.Set(1, 0, 0);
			}
			go->vel += DirVec * 10.f * static_cast<float>(dt) * m_speed;
			if (go->vel.LengthSquared() > 20 * 20)
			{
				go->vel.Normalize() *= 20;
			}

		}

	}
}

static Vector3 RotateVector(const Vector3& vec, float radian)
{
	return Vector3(vec.x * cos(radian) + vec.y * -sin(radian), vec.x * sin(radian) + vec.y * cos(radian), 0.f);

}

float SceneCollision::ForceBtwObjects(GameObject* one, GameObject* two)
{

	float r = (one->pos - two->pos).LengthSquared();

	return  (10.f * ((one->mass * two->mass) / (r)));
}

//porting ends here

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

	 elapsedtime += dt;
	
	if(Application::IsKeyPressed('9'))
	{
		m_speed = Math::Max(0.f, m_speed - 0.1f);
	}
	if(Application::IsKeyPressed('0'))
	{
		m_speed += 0.1f;
	}
	m_force.SetZero();
	m_torque.SetZero();
	if (Application::IsKeyPressed('W'))
	{
		m_force += m_ship->dir * 100.f;

	}
	if (Application::IsKeyPressed('A'))
	{
		m_force += m_ship->dir * 5;
		m_torque += Vector3(0, -m_ship->scale.y, 0).Cross(Vector3(5, 0, 0));
	}
	if (Application::IsKeyPressed('S'))
	{
		m_force -= m_ship->dir * 100;

	}
	if (Application::IsKeyPressed('D'))
	{
		m_force += m_ship->dir * 5;
		m_torque += Vector3(0, m_ship->scale.y, 0).Cross(Vector3(5, 0, 0));
	}

	if (Application::IsKeyPressed(('C')))
	{
		m_ship->consume = true;	
		AI->setSelfdestruct(false);
	}

	//Mouse Section
	static bool bLButtonState = false;
	Vector3 Mousepos(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
	
	float timedifference = elapsedtime - prevElapsed;
	if (!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		m_ghost->active = true;
		m_ghost->pos.Set((x / w) * m_worldWidth, (h - y) / h * m_worldHeight, 0);
		m_ghost->scale.Set(1, 1, 1);
	}
	else if (bLButtonState && !Application::IsMousePressed(0) && timedifference > 0.5f)
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
		GameObject* go = FetchGO();
		go->pos = m_ship->pos;//set pos of projectile
		go->pos.Set(m_ship->pos.x, m_ship->pos.y, 0.f);//set pos of projectile
		go->type = GameObject::GAMEOBJECT_TYPE::GO_PROJECTILE;
		go->scale.Set(1, 1, 1);
		Vector3 posDelta;//vector the store the change in position
		posDelta = m_ghost->pos - go->pos;//setting the aforementioned vector
		Vector3 projDir = posDelta.Normalized();//find the direction of the projectile
		projDir = posDelta.Normalized();//see above
		go->vel = 40 * projDir;//sets velocity
		go->range = 30.f;//set the range of the projectile
		prevElapsed = elapsedtime;
		go->active = true;
		// other usual init stuff go here
		engine->play2D("../Physics/Sounds/gunshot.wav");
		//raycasting stuff for debugging
		estimatedTime = FLT_MAX;

		int goListSize = m_goList.size();
		for (int i = 0; i < goListSize; ++i)
		{
			if (m_goList[i] == go)
			{
				continue;
			}
			float t = CheckCollison2(go, m_goList[i], dt);
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
		m_ghost->pos.Set((x / w) * m_worldWidth, (h - y) / h * m_worldHeight, 0);
		m_ghost->scale.Set(1, 1, 1);
	}
	else if (bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
		//Vector3 Mousepos(x * (m_worldWidth / w), (h - y) * (m_worldHeight / h), 0);
		GameObject* go = FetchGO();
		go->pos = m_ship->pos;//set pos of projectile
		go->pos.Set(m_ship->pos.x, m_ship->pos.y, 0.f);//set pos of projectile
		go->type = GameObject::GAMEOBJECT_TYPE::GO_BOOMERANG;
		go->scale.Set(1, 1, 1);
		Vector3 posDelta;//vector the store the change in position
		posDelta = m_ghost->pos - go->pos;//setting the aforementioned vector
		Vector3 projDir = posDelta.Normalized();//find the direction of the projectile
		projDir = posDelta.Normalized();//see above
		go->vel = 40 * projDir;//sets velocity
		go->range = 30.f;//set the range of the projectile
		prevElapsed = elapsedtime;
		go->active = true;


	}

	if (bRButtonState)
	{
		float size = Math::Clamp((Mousepos - m_ghost->pos).Length(), 2.f, 10.f);
		m_ghost->scale.Set(size, size, size);
		m_ghost->mass = size * size * size;
	}
	//ported for projectiles
	if (Application::IsKeyPressed(VK_SPACE))
	{
		float timedifference = elapsedtime - prevElapsed;

		if (timedifference > 0.2f)
		{
			GameObject* Bullets = FetchGO();
			Bullets->type = GameObject::GO_BULLET;
			Bullets->active = true;
			Bullets->scale.Set(1, 1, 1);
			Bullets->pos.Set(m_ship->pos.x, m_ship->pos.y, 0);
			Bullets->vel = m_ship->dir.Normalized() * BULLET_SPEED;
			Bullets->iframesRead = Bullets->iframesWrite = 5.f;
			//Bullets->health = 1;
			prevElapsed = elapsedtime;
		}
	}

	static float bounceTime = 0;
	if (Level <= 3 && getEnemiesRemainder() > 0 && bounceTime <= 0.f) //spawns enemy follows similar algo to asteroids
	{
		if (enemyCounter <= getEnemiesRemainder())
		{
			bounceTime = dt * 10;
			GameObject* go = FetchGO();
			//go->active = true;
			go->type = GameObject::GO_WBC;
			go->scale.Set(3.5f, 3.5f, 0);
			go->dir.Set(1, 1, 0);
			go->health = 3;
			enemyHealth = go->health;
			go->pos.Set(Math::RandFloatMinMax(2.f, m_worldWidth), m_worldHeight, 0);
			go->vel.Set(2.5f, -2.5f, 0.f);
			++enemyCounter;
		}

		if (GetLevel() == 2 && enemycheck == true)
		{
			enemyCounter = 0;
			enemycheck = false;
		}
		else if (GetLevel() == 3 && enemycheck == false)
		{
			enemyCounter = 0;
			enemycheck = true;
		}

	}
	bounceTime -= dt;


	//ported controls for player(just rename)
	m_ship->vel += (1.f / (float)m_ship->mass) * m_force;
	if (m_ship->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
	{
		m_ship->vel.Normalize() *= MAX_SPEED;
	}
	m_ship->pos += m_ship->vel * dt * m_speed;

	Vector3 acceleration = m_force * (1.f / (float)m_ship->mass);

	//pract3 exe4

	float angularAcceleration = m_torque.z * (1.f / m_ship->momentofinertia);
	m_ship->angularVelocity += angularAcceleration * dt * m_speed;
	m_ship->angularVelocity = Math::Clamp(m_ship->angularVelocity, -ROTATION_POWER, ROTATION_POWER);
	m_ship->angularVelocity *= 0.97f;
	m_ship->dir = RotateVector(m_ship->dir, m_ship->angularVelocity * dt * m_speed);
	
	i_frames -= dt;

	if (m_ship->angularVelocity >= 3.f)
	{
		m_ship->angularVelocity = 0.5f;
	}
	else if (m_ship->angularVelocity < -3.f)
	{
		m_ship->angularVelocity = -0.5f;
	}
	//wraps ship around the place
	if (m_ship->pos.y >= m_worldHeight)
	{
		m_ship->pos.y -= m_worldHeight;
	}
	else if (m_ship->pos.y < 0)
	{
		m_ship->pos.y += m_worldHeight;
	}

	if (m_ship->pos.x >= m_worldWidth)
	{
		m_ship->pos.x -= m_worldWidth;
	}
	else if (m_ship->pos.x < 0)
	{
		m_ship->pos.x += m_worldWidth;
	}

	if (m_ship->health <= 0)
	{
		m_lives--;
		m_ship->health = 20;
	}

	//advances to the next level
	if (GetLevel() == 1 && getAsteroidRemainder() <= -1 && getEnemiesRemainder() <= -1)
	{
		Level++;
	}

	//Physics Simulation Section

	int size = m_goList.size();
	for (int i = 0; i < size; ++i)
	{
		GameObject* go = m_goList[i];
		if (go->active)
		{
			go->pos += go->vel * dt * m_speed;

			if (go->type == GameObject::GAMEOBJECT_TYPE::GO_ASTEROID)
			{
				//do rand to drop powerups
				float distance = sqrt(((go->pos.x - m_ship->pos.x) * (go->pos.x - m_ship->pos.x)) + ((go->pos.y - m_ship->pos.y) * (go->pos.y - m_ship->pos.y)));


				if (distance < 2)
				{
					go->active = false;
					m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0.f);
					m_lives--;
					asteroid_remaining -= 1;


				}


				//Exercise 13: asteroids should wrap around the screen like the ship
				if (go->pos.y >= m_worldHeight)
				{
					go->pos.y = 0;
				}
				else if (go->pos.y < 0)
				{
					go->pos.y = m_worldHeight;
				}

				if (go->pos.x >= m_worldWidth)
				{
					go->pos.x = 0;
				}
				else if (go->pos.x < 0)
				{
					go->pos.x = m_worldWidth;
				}

			}

			else if (go->type == GameObject::GO_MISSILE)
			{
				if (go->pos.x > m_worldWidth || go->pos.x < 0 || go->pos.y < 0 || go->pos.y > m_worldHeight)
				{
					go->active = false;

					continue;
				}


				GameObject* closestGO = GetClosestGo(go);
				if (closestGO != nullptr)
				{
					Vector3 tempDist = closestGO->pos - go->pos;
					try
					{
						go->dir = tempDist.Normalized();
					}
					catch (DivideByZero)
					{
						//dont change dir
					}

					go->vel += 1.f / go->mass * go->dir * 50 * dt * m_speed;
					if (go->vel.LengthSquared() > MISSILE_SPEED * MISSILE_SPEED)
						go->vel.Normalize() *= MISSILE_SPEED;
				}
				for (int i = 0; i < m_goList.size(); ++i)
				{
					GameObject* Other = m_goList[i];
					if (Other->active == true && Other->type == GameObject::GO_ASTEROID)
					{
						Vector3 dispvec = go->pos - Other->pos;
						float combinedRadius = go->scale.x + Other->scale.x;

						if (dispvec.LengthSquared() <= combinedRadius * combinedRadius)
						{
							go->active = false;
							Other->active = false;
							asteroid_remaining -= 1;
							//m_score += 2;
							break;
						}
					}
					else if (Other->active == true && Other->type == GameObject::GO_ENEMY)
					{
						Vector3 tempDist = go->pos - Other->pos;

						if (tempDist.Length() < go->scale.x + Other->scale.x)
						{
							//m_score += 2;
							go->active = false;
							//enemy_remaining -= 1;
							--Other->health;
							break;
						}
					}
					else if (Other->active == true && Other->type == GameObject::GO_BOSS)
					{
						Vector3 tempDist = go->pos - Other->pos;

						if (tempDist.Length() < go->scale.x + Other->scale.x)
						{
							//m_score += 2;
							go->active = false;
							//enemy_remaining -= 1;
							Other->health -= 2;
							break;
						}
					}
				}
			}

		

			else if (go->type == GameObject::GO_PROJECTILE)
			{
			  Vector3 temp;
			  temp = (float)dt * go->vel;
			  float othertemp;
			  othertemp = temp.Length();
			  go->range -= othertemp;
			  if (go->range <= 0)
			  {
				 go->active = false;
			  }
			}
			//todo: merge these
			else if (go->type == GameObject::GO_BOOMERANG)
			{
			  Vector3 temp;
			  temp = (float)dt * go->vel;
			  float othertemp;
			  othertemp = temp.Length();
			  go->range -= othertemp;
			  if (go->range <= 0)
			  {
				 Vector3 posDelta;
				 posDelta = m_ship->pos - go->pos;
				 Vector3 dirDelta;
				 dirDelta = posDelta.Normalized();
				 go->vel = dirDelta * 80;

			  }

			}

			else if (go->type == GameObject::GO_RBC)
			{
			 if (AI->generalAIchck(m_ship, go) == true)
			 {
				 AI->generalAIresponse(go, m_ship);
			 }


			 if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
				 go->vel.Normalize() *= MAX_SPEED;

			 // Wrap
			 if (AI->getPanic() != true)
			 {
				 go->vel.Set(Math::RandFloatMinMax(-2.5f, 3.f), 2.5f, 0.f);

				 if (go->pos.y < 0)
				 {
					 go->pos.y += m_worldHeight;
				 }
				 else if (go->pos.y >= m_worldHeight)
				 {
					 go->pos.y -= m_worldHeight;
				 }
				 if (go->pos.x < 0)
				 {
					 go->pos.x += m_worldWidth;
				 }
				 else if (go->pos.x >= m_worldWidth)
				 {
					 go->pos.x -= m_worldWidth;
				 }
			 }
			 else if (AI->getPanic() == true)
			 {
				 if (go->pos.y < 0)
				 {
					 AI->setPanic(false);
					 ReturnGO(go);
				 }
				 else if (go->pos.y >= m_worldHeight)
				 {
					 AI->setPanic(false);
					 ReturnGO(go);
				 }
				 if (go->pos.x < 0)
				 {
					 AI->setPanic(false);
					 ReturnGO(go);
				 }
				 else if (go->pos.x >= m_worldWidth)
				 {
					 AI->setPanic(false);
					 ReturnGO(go);
				 }
			 }


			 if (go->health <= 0)
			 {
				 ReturnGO(go);
			 }

			 if (m_ship->consume != true && AI->getSelfdestruct() == true)
			 {
				 AI->setSelfdestruct(false);
				 ReturnGO(go);
				 m_ship->health -= 2;
			 }
			}

			else if (go->type == GameObject::GO_WBC)
			{
			 if (go->bounceTime > 0.f)
			 {
				go->bounceTime -= dt;
			 }
			 else
			 {
				go->bounceTime = dt * 50 * (rand() % 2 + 1);

				GameObject* go2 = FetchGO();
				go2->active = true;
				go2->type = GameObject::GO_WBC_PROJECTILES;
				go2->scale.Set(.5f, .5f, 0);
				go2->pos = go->pos;
				go2->vel.Set(go->dir.x * BULLET_SPEED, go->dir.y * BULLET_SPEED, 0);
			 }


			  Vector3 tempDist = m_ship->pos - go->pos;
			  go->dir = tempDist.Normalized();

			 if (AI->generalAIchck(m_ship, go) == true)
			 {
				AI->generalAIresponse(go, m_ship);
			 }

			 if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
				 go->vel.Normalize() *= MAX_SPEED;

			  // Wrap
			 if (go->pos.y < 0)
			 {
				go->pos.y += m_worldHeight;
			 }
			 else if (go->pos.y >= m_worldHeight)
			 {
				go->pos.y -= m_worldHeight;
			 }
			 if (go->pos.x < 0)
			 {
				go->pos.x += m_worldWidth;
			 }
			 else if (go->pos.x >= m_worldWidth)
			 {
				go->pos.x -= m_worldWidth;
			 }

			 if (go->health <= 0)
			 {
				ReturnGO(go);
			 }

			 if (m_ship->consume != true && AI->getSelfdestruct() == true)
			 {
				ReturnGO(go);
				AI->setSelfdestruct(false);
			 }
			}

			else if (go->type == GameObject::GO_TCELLS)
			{
			 if (AI->generalAIchck(m_ship, go) == true)
			 {
				AI->generalAIresponse(go, m_ship);
			 }


			 if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
				 go->vel.Normalize() *= MAX_SPEED;

			// Wrap
			 if (go->pos.y < 0)
			 {
				go->pos.y += m_worldHeight;
			 }
			 else if (go->pos.y >= m_worldHeight)
			 {
				go->pos.y -= m_worldHeight;
			 }
			 if (go->pos.x < 0)
			 {
				go->pos.x += m_worldWidth;
			 }
			 else if (go->pos.x >= m_worldWidth)
			 {
				go->pos.x -= m_worldWidth;
			 }

			 if (go->health <= 0)
			 {
			 	ReturnGO(go);
			 }

			 if (m_ship->consume != true && AI->getSelfdestruct() == true)
			 {
				ReturnGO(go);
				AI->setSelfdestruct(false);
				m_ship->health -= 2;
			 }

			}

			else if (go->type == GameObject::GO_DEADCELLS)
			{
			 if (AI->generalAIchck(m_ship, go) == true)
			 {
				AI->generalAIresponse(go, m_ship);
			 }

			 // Wrap
			 if (go->pos.y < 0)
			 {
				go->pos.y += m_worldHeight;
			 }
			 else if (go->pos.y >= m_worldHeight)
			 {
				go->pos.y -= m_worldHeight;
			 }
			 if (go->pos.x < 0)
			 {
				go->pos.x += m_worldWidth;
			 }
			 else if (go->pos.x >= m_worldWidth)
			 {
				go->pos.x -= m_worldWidth;
			 }
			}

			else if (go->type == GameObject::GO_WBC_PROJECTILES)
		    {
		      if (i_frames <= 0.f)
		      {

			     Vector3 tempDist = go->pos - m_ship->pos;

			    if (tempDist.LengthSquared() <= (m_ship->scale.x + go->scale.x) * (m_ship->scale.x + go->scale.x))
			    {
				  --m_ship->health;
				  i_frames = 1;
				  std::cout << "Ouch " << m_ship->health << std::endl;
				  go->active = false;

			    }
		      }
		    }


			//Exercise 16: unspawn bullets when they leave screen
			else if (go->type == GameObject::GAMEOBJECT_TYPE::GO_BULLET)
			{

			 if (go->pos.x > m_worldWidth || go->pos.x < 0 || go->pos.y < 0 || go->pos.y > m_worldHeight)
			 {
				go->active = false;
				continue;
			 }

			 //Exercise 18: collision check between GO_BULLET and GO_ASTEROID

			 for (int i = 0; i < m_goList.size(); ++i)
			 {
				GameObject* Other = m_goList[i];
				if (Other->active == true && Other->type == GameObject::GO_ASTEROID)
				{
					Vector3 dispvec = go->pos - Other->pos;
					float combinedRadius = go->scale.x + Other->scale.x;

					if (dispvec.LengthSquared() <= combinedRadius * combinedRadius)
					{
						go->active = false;
						Other->active = false;
						asteroid_remaining -= 1;
						//m_score += 2;
						//powerupchck = Math::RandIntMinMax(1, 5);

						for (int miniasteroid = 0; miniasteroid < 1; ++miniasteroid)
						{
							GameObject* mini_asteroid = FetchGO();
							//Missile->active = true;
							mini_asteroid->type = GameObject::GO_MINIASTEROID;
							mini_asteroid->scale.Set(2.f, 2.f, 2.f);
							mini_asteroid->pos.Set(Other->pos.x - (Math::RandIntMinMax(1, 3)),
								Other->pos.y - (Math::RandIntMinMax(1, 3)), 0);
							mini_asteroid->vel.Set(Math::RandIntMinMax(-1, 2), Math::RandIntMinMax(-1, 2), 0);


						}


						break;
					}

					
				
				
				}
				else if (Other->active == true && Other->type == GameObject::GO_ENEMY)
				{
					Vector3 tempDist = go->pos - Other->pos;

					if (tempDist.Length() < go->scale.x + Other->scale.x)
					{
						//m_score += 2;
						go->active = false;

						--Other->health;
						enemyHealth = Other->health;
						break;
					}
				}
				else if (Other->active == true && Other->type == GameObject::GO_MINIASTEROID)
				{
					Vector3 dispvec = go->pos - Other->pos;
					float combinedRadius = go->scale.x + Other->scale.x;

					if (dispvec.LengthSquared() <= combinedRadius * combinedRadius)
					{
						go->active = false;
						Other->active = false;
						//m_score += 1;

						break;
					}
				}
				else if (Other->active == true && Other->type == GameObject::GO_BOSS)
				{
					Vector3 tempDist = go->pos - Other->pos;

					if (tempDist.Length() < go->scale.x + Other->scale.x)
					{

						go->active = false;
						--Other->health;
						enemyHealth = Other->health;
						break;
					}
				}

				
			 }


		

			}

			

			GameObject* go2 = nullptr;
			for (int j = i + 1; j < size; ++j)
			{
				go2 = m_goList[j];
				GameObject* actor(go), * actee(go2);
				if (go->type != GameObject::GO_PROJECTILE)
				{
					actor = go2;
					actee = go;
				}
				else if (go->type != GameObject::GO_BOOMERANG)
				{
					actor = go2;
					actee = go;
				}
				//if (go2->active && CheckCollision(actor, actee, 0))
				float t = CheckCollison2(actor, actee, dt);
				if (t >= 0 && t <= dt)
				{
					timeActive = false;
					doCollisionResponse(actor, actee);
				}

				//if (go2->active == true && CheckCollision(actor, actee, dt))
				//{
				//	if (actor->iframesRead == actor->iframesWrite)//this checks if either the player of enemy is invulnerable or not
				//	{
				//		doCollisionResponse(actor, actee);
				//		actor->iframesWrite -= dt;//after the response is made, then start to invulnerability period
				//	}

				//}
				if (actor->iframesWrite < 0)//if invulnerability period is up, reset the iframes
				{
					actor->iframesWrite = actor->iframesRead;
					std::cout << "hit me" << std::endl;
				}
			}
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
		
						
		
	}
	if (timeActive)
	{
		timeTaken += dt;
	}
		

	if (m_lives <= 0) //when you die
	{
		m_ship->vel.SetZero();
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			go->active = false;
		}
		m_goList.clear();
	}

	if (m_ship->health >= 20) //chck for the health packs so that they dont overexceed
	{
		m_ship->health = 20;
	}
	m_ship->vel *= 0.97;
}

//if need to include z scale for overlapping planes: GameObject *go, float z
void SceneCollision::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
	case GameObject::GO_PILLAR:
	case GameObject::GO_PROJECTILE:
	case GameObject::GO_BOOMERANG:
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
	case GameObject::GO_SHIP:
		//Exercise 4a: render a sphere with radius 1
		//Exercise 17a: render a ship texture or 3D ship model
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(89.5f), 1.f, 0.f, 0.f);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 1.f, 0.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_VIRUS], false);
		modelStack.PopMatrix();
		//Exercise 17b:	re-orientate the ship with velocity
		break;
	case GameObject::GO_WBC_PROJECTILES:
	case GameObject::GO_BULLET:
		//Exercise 4a: render a sphere with radius 1
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_RBC:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_RBC], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_WBC:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_WBC], false);
		modelStack.PopMatrix();
		//Exercise 17b:	re-orientate the ship with velocity
		break;
	case GameObject::GO_TCELLS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_TCELLS], false);
		modelStack.PopMatrix();
		//Exercise 17b:	re-orientate the ship with velocity
		break;
	case GameObject::GO_DEADCELLS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_DEADCELLS], false);
		modelStack.PopMatrix();
		//Exercise 17b:	re-orientate the ship with velocity
		break;

		//un comment whatever model u want to test
		/*	case GameObject::GO_SHIP:
		//Exercise 4a: render a sphere with radius 1
		//Exercise 17a: render a ship texture or 3D ship model
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Rotate(Math::RadianToDegree(89.5f), 1.f, 0.f, 0.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_RBC], false);
		modelStack.PopMatrix();
		//Exercise 17b:	re-orientate the ship with velocity
		break;
	case GameObject::GO_SMALLSHIP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BUDDYSHIP], false);
		modelStack.PopMatrix();
	case GameObject::GO_ASTEROID:
	case GameObject::GO_MINIASTEROID:
		//Exercise 4b: render a cube with length 2
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ASTEROID], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BULLET:
	case GameObject::GO_ENEMY_BULLET:
	case GameObject::GO_MINION_BULLET:
	case GameObject::GO_PROJECTILE:
		//Exercise 4a: render a sphere with radius 1
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BLACKHOLE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BLACKHOLE], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_REPELLER:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_REPELLER], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_ENEMY:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_ENEMYSHIP], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_HEALTHPOWERUP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_HEALTHRECOVER], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BULLETPOWERUP:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BULLETSPRAY], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_BOSS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BOSS], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_MISSILE:
	case GameObject::GO_BOSS_MISSILE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_MISSILE], false);
		modelStack.PopMatrix();
		break;*/
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

	RenderGO(m_ship);

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

	if (m_lives <= 0)
	{
		ss.str("");
		ss.precision(5);
		ss << "YOU LOSE! CONTINUE?";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 25);
	}

	ss.str("");
	ss.precision(5);
	ss << "Object count: " << m_objectCount;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 12);

	/*

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
	engine->drop();
	factory->drop();
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
