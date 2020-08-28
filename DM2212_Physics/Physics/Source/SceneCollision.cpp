#include "SceneCollision.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

using namespace irrklang;

int selection = 0;

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

	g_eGameStates = S_MAIN;

	// Start sound engine 
	engine = createIrrKlangDevice();
	factory = new CMyFileFactory(); // File factory is to allow us to use our sound files

	if (!engine)
	{
		return;
	}

	elapsedTime = 0.0f;

	gameStart = false;

	engine->addFileFactory(factory);

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;	
	size = 2;
	m_score = 0;
	m_angle = 0;
	randomenemyspawn = 0;

	Math::InitRNG();

	m_objectCount = 1; //this includes the player, enemies and neutral entities
	m_ghost = new GameObject(GameObject::GO_BALL);

	//ported over
	m_virus = new GameObject(GameObject::GO_SHIP);
	m_virus->active = true;
	m_virus->type = GameObject::GO_SHIP;
	m_virus->scale.Set(5.5f, 5.5f, 5.5f);
	m_virus->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0.f);
	m_virus->vel.Set(0.f, 0.f, 0.f);
	m_virus->momentofinertia = m_virus->mass * m_virus->scale.x * m_virus->scale.x;
	m_virus->angularVelocity = 0.f;
	m_virus->health = 20;

	m_lives = 3;

	AI = new GeneralClass();

	minioncounter = 0;
	cooldown = 10.f;
	enemyHealth = 3;
	biomass = 0;
	counter = 0;

	scenechanger = false;

	m_torque.Set(0, 0, 0);
	angle = 0;

	elapsedtime = prevElapsed = 0;
	bounceTime = 0.0;
	timerCount = 0.f;
	timerUp = false;
	activated = false;
	estimatedTime = 0.f;
	timeTaken = 0.f;
	timeActive =  false;
}


GameObject* SceneCollision::FetchGO()
{
	
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
	
	if (go->active == true)
	{
		go->active = false;
		m_objectCount--;
	}
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
	case GameObject::GO_MINION_BULLET:
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
		ab.DoAbility(go1, go2, m_virus);//go1 is the projectile, go2 is the target
		if (!go2->active)
		{
			ReturnGO(go1);
			
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

			Vector3 DirVec = (m_virus->pos - (m_virus->dir * 10)) - go->pos;
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
	 static float bounceTime = 0;
	 elapsedtime += dt;
	 if (gameStart)
	 {
		 
		 m_force.SetZero();
		 m_torque.SetZero();
		 if (Application::IsKeyPressed('W'))
		 {
			 m_force += m_virus->dir * 100.f;

		 }
		 if (Application::IsKeyPressed('A'))
		 {
			 m_force += m_virus->dir * 5;
			 m_torque += Vector3(0, -m_virus->scale.y, 0).Cross(Vector3(5, 0, 0));
		 }
		 if (Application::IsKeyPressed('S'))
		 {
			 m_force -= m_virus->dir * 100;

		 }
		 if (Application::IsKeyPressed('D'))
		 {
			 m_force += m_virus->dir * 5;
			 m_torque += Vector3(0, m_virus->scale.y, 0).Cross(Vector3(5, 0, 0));
		 }

		 if (Application::IsKeyPressed(('C')))
		 {
			 m_virus->consume = true;
			 AI->setSelfdestruct(false);
		 }

		 //Mouse Section
		 static bool bLButtonState = false;
		 
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
			 go->pos = m_virus->pos;//set pos of projectile
			 go->pos.Set(m_virus->pos.x, m_virus->pos.y, 0.f);//set pos of projectile
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
			 engine->play2D("../Physics/Sounds/firing.wav");
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
			 GameObject* go = FetchGO();
			 go->pos = m_virus->pos;//set pos of projectile
			 go->pos.Set(m_virus->pos.x, m_virus->pos.y, 0.f);//set pos of projectile
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

		 //ported for projectiles
		 //if (Application::IsKeyPressed(VK_SPACE))
		 //{
          // float timedifference = elapsedtime - prevElapsed;
			// if (timedifference > 0.2f)
			// {
			//	 GameObject* Bullets = FetchGO();
			//	 Bullets->type = GameObject::GO_BULLET;
			//	 Bullets->active = true;
			//	 Bullets->scale.Set(1, 1, 1);
			//	 Bullets->pos.Set(m_virus->pos.x, m_virus->pos.y, 0);
			//	 Bullets->vel = m_virus->dir.Normalized() * BULLET_SPEED;
			//	 Bullets->iframesRead = Bullets->iframesWrite = 5.f;
			//	 //Bullets->health = 1;
			//	 prevElapsed = elapsedtime;
			// }
		 //}

		 static bool cButton = false;
		 if (!cButton && Application::IsKeyPressed(('C')))
		 {
			 cButton = true;
			 if (m_virus->consume == false)
			 {
				 m_virus->consume = true;
				 std::cout << "rdy to consume" << std::endl;
			 }
			 else if (m_virus->consume)
			 {
				 m_virus->consume = false;
			 }

			 AI->setSelfdestruct(false);
		 }
		 else if (cButton && !Application::IsKeyPressed('C'))
		 {
			 cButton = false;
		 }

		 if (AI->getmunchChck() == true)
		 {
			 AI->setmunchChck(false);
			 biomass += AI->getBiomass();
			 std::cout << "biomass: " << biomass << std::endl;
		 }

		 if (Application::IsKeyPressed('H'))
		 {
			 activated = true;
			 timerUp = false;
		 }

		 if (activated && timerUp == false)
		 {
			 std::cout << "BLACKHOLESPAWN" << std::endl;
			 GameObject* go = FetchGO();
			 go->active = false;
			 go->type = GameObject::GO_BLACKHOLE;
			 go->scale.Set(5, 5, 5);
			 go->vel = m_virus->vel;
			 go->mass = 3000;
			
			 go->pos.Set(m_virus->pos.x + m_virus->vel.x, m_virus->pos.y + m_virus->vel.y, 0);

			 timerCount++;
		 }
		 else
		 {
			 //std::cout << "BLACKHOLEDESPAWN" << std::endl;
			 timerCount = 0;
		 }
		 if (timerCount > 150.f)
		 {
			 timerUp = true;
			 activated = false;
		 }

		 // Keybind for Landmine Ability
		 static bool QbuttonState = false;
		 if (biomass >= 15)
		 {
			 if (!QbuttonState && Application::IsKeyPressed('Q'))
			 {
				 std::cout << "Q down" << std::endl;
				 QbuttonState = true;
			 }
			 else if (QbuttonState && !Application::IsKeyPressed('Q'))
			 {
				 std::cout << "Q up " << std::endl;
				 QbuttonState = false;
				 GameObject* mine = FetchGO();

				 mine->type = GameObject::GO_VIRUSMINE;
				 mine->scale.Set(2.5f, 2.5f, 2.5f);
				 mine->pos.Set(m_virus->pos.x, m_virus->pos.y, 0);
				 mine->vel = 0;
				 prevElapsed = elapsedtime;
				 biomass -= 15;
			 }
		 }
		 

		 // Keybind for Spilt Ability
		 static bool EbuttonState = false;
		 if(biomass >= 5)
		 {
			 if (!EbuttonState && Application::IsKeyPressed('E'))
			 {
				 std::cout << "E down" << std::endl;
				 EbuttonState = true;
			 }
			 else if (EbuttonState && !Application::IsKeyPressed('E'))
			 {
				 std::cout << "E up" << std::endl;

				 EbuttonState = false;
				 for (int i = 0; i < 5; ++i)
				 {
					 ++minioncounter;
					 GameObject* co2 = FetchGO();
					 co2->active = true;
					 co2->type = GameObject::GO_VIRUSBUDDY;
					 co2->scale.Set(1, 1, 1);
					 co2->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), Math::RandFloatMinMax(0, m_worldHeight), 0);
					 float angle = Math::TWO_PI / 10.f * minioncounter;
					 co2->offset.Set(10.f * cos(angle), 10.f * sin(angle));


				 }
				 engine->play2D("../Physics/Sounds/fruit-crack.wav");
				 m_virus->health -= 5;
				 biomass -= 5;
			 }
		 }
		


		 //spawns enemy
		 if (m_objectCount < 26)
		 {
			 randomenemyspawn = Math::RandIntMinMax(1, 50);//wbc,tcell,rbc,deadcell
			 if (randomenemyspawn % 13 == 0) 
			 {

				 bounceTime = dt * 10;
				 GameObject* go = FetchGO();
			
				 go->type = GameObject::GO_WBC;
				 go->scale.Set(3.5f, 3.5f, 0);
				 go->dir.Set(1, 1, 0);
				 go->health = 3;
				 enemyHealth = go->health;
				 go->pos.Set(Math::RandFloatMinMax(2.f, m_worldWidth), m_worldHeight, 0);
				 go->vel.Set(2.5f, -2.5f, 0.f);
		
				 m_objectCount++;


			 }

			 else if (randomenemyspawn % 25 == 0)
			 {

				 bounceTime = dt * 10;
				 GameObject* go = FetchGO();

				 go->type = GameObject::GO_TCELLS;
				 go->scale.Set(3.5f, 3.5f, 0);
				 go->dir.Set(1, 1, 0);
				 go->health = 3;
				 enemyHealth = go->health;
				 go->pos.Set(Math::RandFloatMinMax(15.f, m_worldWidth), m_worldHeight, 0);
				 go->vel.Set(2.5f, -2.5f, 0.f);

				 m_objectCount++;

			 }

			 else if (randomenemyspawn % 2 == 0)
			 {

				 bounceTime = dt * 10;
				 GameObject* go = FetchGO();
			
				 go->type = GameObject::GO_RBC;
				 go->scale.Set(3.5f, 3.5f, 0);
				 go->dir.Set(1, 1, 0);
				 go->health = 3;
				 enemyHealth = go->health;
				 go->pos.Set(Math::RandFloatMinMax(15.f, m_worldWidth), m_worldHeight, 0);
				 go->vel.Set(2.5f, -2.5f, 0.f);
	
				 m_objectCount++;

			 }

			 else if (randomenemyspawn == 4)
			 {
				 bounceTime = dt * 10;
				 GameObject* go = FetchGO();
			
				 go->type = GameObject::GO_DEADCELLS;
				 go->scale.Set(3.5f, 3.5f, 0);
				 go->dir.Set(1, 1, 0);
				 go->health = 3;
				 enemyHealth = go->health;
				 go->pos.Set(Math::RandFloatMinMax(15.f, m_worldWidth), m_worldHeight, 0);
				 go->vel.Set(2.5f, -2.5f, 0.f);
				
				 m_objectCount++;
			 }
		 }

		
	
		 m_virus->vel += (1.f / (float)m_virus->mass) * m_force;
		 if (m_virus->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
		 {
			 m_virus->vel.Normalize() *= MAX_SPEED;
		 }
		 m_virus->pos += m_virus->vel * dt * m_speed;

		 Vector3 acceleration = m_force * (1.f / (float)m_virus->mass);

		 float angularAcceleration = m_torque.z * (1.f / m_virus->momentofinertia);
		 m_virus->angularVelocity += angularAcceleration * dt * m_speed;
		 m_virus->angularVelocity = Math::Clamp(m_virus->angularVelocity, -ROTATION_POWER, ROTATION_POWER);
		 m_virus->angularVelocity *= 0.97f;
		 m_virus->dir = RotateVector(m_virus->dir, m_virus->angularVelocity * dt * m_speed);

		 i_frames -= dt;

		 if (m_virus->angularVelocity >= 3.f)
		 {
			 m_virus->angularVelocity = 0.5f;
		 }
		 else if (m_virus->angularVelocity < -3.f)
		 {
			 m_virus->angularVelocity = -0.5f;
		 }

		 //wraps ship around the place
		 if (m_virus->pos.y >= m_worldHeight)
		 {
			 m_virus->pos.y -= m_worldHeight;
		 }
		 else if (m_virus->pos.y < 0)
		 {
			 m_virus->pos.y += m_worldHeight;
		 }

		 if (m_virus->pos.x >= m_worldWidth)
		 {
			 m_virus->pos.x -= m_worldWidth;
		 }
		 else if (m_virus->pos.x < 0)
		 {
			 m_virus->pos.x += m_worldWidth;
		 }



		 //Physics Simulation Section

		 int size = m_goList.size();
		 for (int i = 0; i < size; ++i)
		 {
			 GameObject* go = m_goList[i];
			 if (go->active != true)
			 {
				 if (go->type == GameObject::GO_BLACKHOLE)
				 {
					 if (timerUp)
					 {
						 go->scale.SetZero();
						 go->active = true;
					 }

					 Vector3 offset = go->offset;
					 float theta = atan2(m_virus->dir.x, m_virus->dir.y);

					 offset.Set((offset.x * cos(theta)) - (offset.y * sin(theta)), (offset.x * sin(theta)) + (offset.y * cos(theta)), 0);


					 Vector3 displacement = m_virus->pos + offset - go->pos;
					 if (displacement.Length() > 10)
					 {
						 displacement.Normalize();
						 displacement *= 10.f;
					 }
					 go->vel += displacement * dt * m_speed;

					 if (displacement.Length() <= m_virus->scale.x) // slowing down speed
					 {
						 go->vel *= 0.9f;
					 }

					 if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
					 {
						 go->vel = go->vel.Normalized() * MAX_SPEED;
					 }




					 float eventhorizon = go->scale.x * 10.f;
					 for (std::vector<GameObject*>::iterator it2 = m_goList.begin(); it2 != m_goList.end(); ++it2)
					 {
						 GameObject* go2 = (GameObject*)*it2;
						 if (go2->active && go2->type != GameObject::GO_BLACKHOLE)
						 {
							 float distance = Vector3(go->pos - go2->pos).Length();
							 Vector3 ForcetoAdd = Vector3(0.005f * ((go->mass * go2->mass) * (1.f / distance * distance))).Length() * Vector3(go->pos - go2->pos).Normalized(); // G(m1 * m2) / r^2

							 if (distance < eventhorizon)
							 {
								 go2->vel += ForcetoAdd * (1.f / go2->mass) * dt * m_speed;
							 }
							 if ((go->pos - go2->pos).Length() < go->scale.x)
							 {
								 go2->active = false;
							 }
						 }
					 }



				 }
			 }
			 if (go->active)
			 {
				 go->pos += go->vel * dt * m_speed;
				 if (go->type == GameObject::GO_PROJECTILE)
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
						 posDelta = m_virus->pos - go->pos;
						 Vector3 dirDelta;
						 dirDelta = posDelta.Normalized();
						 go->vel = dirDelta * 80;
						 if ((go->pos - m_virus->pos).Length() < 1)
						 {
							 go->active = false;
						 }
						 else if ((go->pos - m_virus->pos).Length() > 60)
						 {
							 go->active = false;
						 }
					 }
					 
					 

				 }

				 else if (go->type == GameObject::GO_RBC)
				 {
					 if (go->pos.x < 0 || go->pos.y < 0 || go->pos.x > m_worldWidth || go->pos.y > m_worldHeight) // ensure the GO goes out of the screen first before wrapping otherwise looks weird
					 {
						 go->active = false;
						 m_objectCount--;
						 continue;
					 }

					 if (AI->generalAIchck(m_virus, go) == true)
					 {
						 AI->generalAIresponse(go, m_virus);
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
							 m_objectCount--;
							 ReturnGO(go);
						 }
						 else if (go->pos.y >= m_worldHeight)
						 {
							 AI->setPanic(false);
							 m_objectCount--;
							 ReturnGO(go);
						 }
						 if (go->pos.x < 0)
						 {
							 AI->setPanic(false);
							 m_objectCount--;
							 ReturnGO(go);
						 }
						 else if (go->pos.x >= m_worldWidth)
						 {
							 AI->setPanic(false);
							 m_objectCount--;
							 ReturnGO(go);
						 }
					 }


					 if (go->health <= 0)
					 {
						 ReturnGO(go);
						 biomass++;
						 m_objectCount--;
					 }

					 if (m_virus->consume != true && AI->getSelfdestruct() == true)
					 {
						 AI->setSelfdestruct(false);
						 ReturnGO(go);
						 m_objectCount--;
						 m_virus->health -= 2;
					 }
				 }

				 else if (go->type == GameObject::GO_WBC)
				 {
				  if (go->pos.x < 0 || go->pos.y < 0 || go->pos.x > m_worldWidth || go->pos.y > m_worldHeight) // ensure the GO goes out of the screen first before wrapping otherwise looks weird
				  {
					 go->active = false;
					 m_objectCount--;
					 continue;
				  }

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
						 go2->vel *= 0.5f;
					 }


					 Vector3 tempDist = m_virus->pos - go->pos;
					 go->dir = tempDist.Normalized();

					 if (AI->generalAIchck(m_virus, go) == true)
					 {
						 AI->generalAIresponse(go, m_virus);
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
						 biomass++;
						 m_objectCount--;
					 }

					 if (m_virus->consume != true && AI->getSelfdestruct() == true)
					 {
						 ReturnGO(go);
						 m_objectCount--;
						 AI->setSelfdestruct(false);
					 }
				 }

				 else if (go->type == GameObject::GO_TCELLS)
				 {
				  if (go->pos.x < 0 || go->pos.y < 0 || go->pos.x > m_worldWidth || go->pos.y > m_worldHeight) // ensure the GO goes out of the screen first before wrapping otherwise looks weird
				  {
					 go->active = false;
					 m_objectCount--;
					 continue;
				  }
					 Vector3 tempDist = m_virus->pos - go->pos;
					 go->dir = tempDist.Normalized();

					 if (AI->generalAIchck(m_virus, go) == true)
					 {
						 AI->generalAIresponse(go, m_virus);
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
						 biomass++;
						 m_objectCount--;
					 }

					 if (m_virus->consume != true && AI->getSelfdestruct() == true)
					 {
						 ReturnGO(go);
						 AI->setSelfdestruct(false);
						 m_objectCount--;
						 m_virus->health -= 2;
					 }

				 }

				 else if (go->type == GameObject::GO_DEADCELLS)
				 {
				  if (go->pos.x < 0 || go->pos.y < 0 || go->pos.x > m_worldWidth || go->pos.y > m_worldHeight) // ensure the GO goes out of the screen first before wrapping otherwise looks weird
				  {
					 go->active = false;
					 m_objectCount--;
					 continue;
				  }

					 if (AI->generalAIchck(m_virus, go) == true)
					 {
						 AI->generalAIresponse(go, m_virus);
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

					 if (go->health <= 0)
					 {
						 ReturnGO(go);
						 biomass++;
						 m_objectCount--;
					 }
				 }

				 else if (go->type == GameObject::GO_WBC_PROJECTILES)
				 {
					 if (i_frames <= 0.f)
					 {

						 Vector3 tempDist = go->pos - m_virus->pos;

						 if (tempDist.LengthSquared() <= (m_virus->scale.x + go->scale.x) * (m_virus->scale.x + go->scale.x))
						 {
							 --m_virus->health;
							 i_frames = 7;
							 std::cout << "Ouch " << m_virus->health << std::endl;
							 go->active = false;
						 }
					 }
				 }

				 else if (go->type == GameObject::GO_VIRUSBUDDY)
			     {
			        Vector3 offset = go->offset;
			        float theta = atan2(m_virus->dir.x, m_virus->dir.y);

			        offset.Set((offset.x * cos(theta)) - (offset.y * sin(theta)), (offset.x * sin(theta)) + (offset.y * cos(theta)), 0);


			        Vector3 displacement = m_virus->pos + offset - go->pos;
			        if (displacement.Length() > 10)
			        {
				      displacement.Normalize();
				      displacement *= 10.f;
			        }
			        go->vel += displacement * dt * m_speed;

			        if (displacement.Length() < m_virus->scale.x) // slowing down speed
			        {
				      go->vel *= 0.9f;
			        }

			        if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
			        {
				      go->vel = go->vel.Normalized() * MAX_SPEED;
			        }

			        if (go->bounceTime > -1.f)
			        {
				      go->bounceTime -= dt;
			        }
			        else
			        {
				      go->bounceTime = dt * 50 * (rand() % 2 + 1);

				      GameObject* go2 = FetchGO();
				      go2->active = true;
				      go2->type = GameObject::GO_MINION_BULLET;
				      go2->scale.Set(.5f, .5f, 0);
				      go2->pos = go->pos;
				      go2->vel = m_virus->dir.Normalized() * BULLET_SPEED;;

			        }

			     }

				 else if (go->type == GameObject::GO_VIRUSMINE)
			     {
			        for (int i = 0; i < m_goList.size(); ++i)
			        {    
				      GameObject* Other = m_goList[i];
				      Vector3 dispvec = go->pos - Other->pos;
				      float combinedRadius = go->scale.x + Other->scale.x;

				      if (Other->active == true && Other->type == GameObject::GO_RBC)
				      {
					     if (dispvec.Length() < go->scale.x + Other->scale.x)
					     {
						   //m_score += 2;
						   Other->active = false;
						   go->active = false;
					     }
				      }
					  else if (Other->active == true && Other->type == GameObject::GO_WBC)
					  {
						  if (dispvec.Length() < go->scale.x + Other->scale.x)
						  {
							  //m_score += 2;
							  Other->active = false;
							  go->active = false;
						  }
					  }
					  else if (Other->active == true && Other->type == GameObject::GO_DEADCELLS)
					  {
						  if (dispvec.Length() < go->scale.x + Other->scale.x)
						  {
							  //m_score += 2;
							  Other->active = false;
							  go->active = false;
						  }
					  }
					  else if (Other->active == true && Other->type == GameObject::GO_TCELLS)
					  {
						  if (dispvec.Length() < go->scale.x + Other->scale.x)
						  {
							  //m_score += 2;
							  Other->active = false;
							  go->active = false;
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
					 else if (go->type != GameObject::GO_MINION_BULLET)
					 {
						 actor = go2;
						 actee = go;
					 }
					
					 float t = CheckCollison2(actor, actee, dt);
					 if (t >= 0 && t <= dt)
					 {
						 if (actor->iframesRead == actor->iframesWrite)//this checks if either the player of enemy is invulnerable or not
						 {
							 timeActive = false;
							 doCollisionResponse(actor, actee);
							 actor->iframesWrite -= dt;//after the response is made, then start to invulnerability period
						 }
						
					 }

					 if (actor->iframesWrite < 0)//if invulnerability period is up, reset the iframes
					 {
						 actor->iframesWrite = actor->iframesRead;
						 std::cout << "hit me" << std::endl;
					 }
				 }
			 }
	
		 }
		 if (timeActive)
		 {
			 timeTaken += dt;
		 }



		 if (m_virus->health >= 20) //chck for the health packs so that they dont overexceed
		 {
			 m_virus->health = 20;
		 }
		 m_virus->vel *= 0.97;
	 }
	if (Application::IsKeyPressed(0x53))
	{
		if (bounceTime > elapsedTime)
		{
			return;
		}
		selection++;
		bounceTime = elapsedTime + 0.3;
	}
	else if (Application::IsKeyPressed(0x57))
	{
		if (bounceTime > elapsedTime)
		{
			return;
		}
		selection--;
		bounceTime = elapsedTime + 0.3;
	}
	if (selection != 2 && selection != 1)
	{
		selection = 0;
	}
	else if (selection != 0 && selection != 2)
	{
		selection = 1;
	}
	else if (selection != 0 && selection != 1)
	{
		selection = 2;
	}

	static bool spaceState = false;
	if (!spaceState && Application::IsKeyPressed(VK_RETURN))
	{
		std::cout << "space down" << std::endl;
		spaceState = true;
	}
	else if (spaceState && !Application::IsKeyPressed(VK_RETURN))
	{
		std::cout << "space up" << std::endl;
		spaceState = false;

		if (g_eGameStates == S_MAIN)
		{

			if (selection == 0)
			{
				g_eGameStates = S_GAME;
				gameStart = true;
			}
			else if (selection == 1)
			{
				g_eGameStates = S_INSTRUCTIONS;
			}
			else if (selection == 2)
			{
				g_eGameStates = S_CREDITS;
			}

		}
		else if (g_eGameStates == S_INSTRUCTIONS)
		{
			g_eGameStates = S_MAIN;
		}
		else if (g_eGameStates == S_CREDITS)
		{
			g_eGameStates = S_MAIN;
		}
	}
	if (biomass >= 100)
	{
		g_eGameStates = S_WIN;
	}

	bounceTime -= dt;
	//Ship dies lose screen
	if (m_virus->health == 0)
	{
		g_eGameStates = S_LOSE;
	}
}


void SceneCollision::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	
	case GameObject::GO_PROJECTILE:
	case GameObject::GO_BOOMERANG:
	case GameObject::GO_MINION_BULLET:
	
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_SHIP:
	
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(89.5f), 1.f, 0.f, 0.f);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 1.f, 0.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_VIRUS], false);
		modelStack.PopMatrix();
	
		break;
	case GameObject::GO_WBC_PROJECTILES:
	case GameObject::GO_BULLET:
	
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
		
		break;
	case GameObject::GO_TCELLS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_TCELLS], false);
		modelStack.PopMatrix();
	
		break;
	case GameObject::GO_DEADCELLS:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_DEADCELLS], false);
		modelStack.PopMatrix();
		
		break;
	case GameObject::GO_VIRUSBUDDY:
		m_angle = atan2(m_virus->dir.y, m_virus->dir.x);

		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Rotate(Math::RadianToDegree(m_angle) - 90, 0, 0, 1);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_VIRUS], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_VIRUSMINE:
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_CUBE], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneCollision::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	


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


	

	//On screen text

	std::ostringstream ss;

	if (g_eGameStates == S_GAME)
	{
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
			{
				RenderGO(go);
			}
		}
	}

	switch (g_eGameStates)
	{
	case S_MAIN:
	{
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		switch (selection)
		{
		case 0:
		{
			ss << "Start";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 3.f, 25.f, 31.f);

			ss.str("");
			ss << "How to Play";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 25.f, 26.f);

			ss.str("");
			ss << "Credits";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 24.3f, 21.f);


			ss.str("");
			ss << "ESC to quit";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.5f, 30.f, 10.f);

			ss.str("");
			ss << "W/S to Select, Press Enter";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.5f, 10.f, 15.f);


			break;

		case 1:
			ss << "Start";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 25.f, 31.f);

			ss.str("");
			ss << "How to Play";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 3.f, 25.f, 26.f);

			ss.str("");
			ss << "Credits";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 24.3, 21.f);

			ss.str("");
			ss << "ESC to quit";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.5f, 30.f, 10.f);

			ss.str("");
			ss << "W/S to Select, Press Enter";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.5f, 10.f, 15.f);


			break;

		case 2:
			ss << "Start";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 25.f, 31.f);

			ss.str("");
			ss << "How to Play";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 0), 3.f, 25.f, 26.f);

			ss.str("");
			ss << "Credits";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 3.f, 24.3, 21.f);

			ss.str("");
			ss << "ESC to quit";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.5f, 30.f, 10.f);

			ss.str("");
			ss << "W/S to Select, Press Enter";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.5f, 10.f, 15.f);

			break;
		}
		default:
			break;
		}
	}
	break;
	case S_INSTRUCTIONS:
	{
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		ss.str("");
		ss << "Instructions";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3.f, 26.f, 55.f);

		ss.str("");
		ss << "You are a virus";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 50.f);

		ss.str("");
		ss << "Your have to survive by gathering";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 45.f);

		ss.str("");
		ss << "biomass from killing cells";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 40.f);

		ss.str("");
		ss << "WASD to move";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 30.f);

		ss.str("");
		ss << "Mouse click to shoot";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 25.f);

		ss.str("");
		ss << "C to consume  Q to place landmine";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 20.f);

		ss.str("");
		ss << "E to split    H to pull in cells";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 2.3f, 2.f, 15.f);



		ss.str("");
		ss << "ENTER to return";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3.f, 5.f, 0.f);


		
	}
	break;
	case S_GAME:
	{
		modelStack.PushMatrix();
		modelStack.Translate(m_virus->health, 98, 0);
		modelStack.Scale(40, 5, 0);
		RenderMesh(meshList[GEO_HEALTHBAR], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Translate(20, 98, 0);
		modelStack.Scale(40, 5, 0);
		RenderMesh(meshList[GEO_HEALTHRED], false);
		modelStack.PopMatrix();

		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

	
		RenderGO(m_virus);

		ss.str("");
		ss << "Health: " << m_virus->health;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 1), 3, 0, 0);

		ss.str("");
		ss << "objects: " << m_objectCount;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 1), 3, 0, 3);


		ss.str("");
		ss << "Biomass: " << biomass << "/" << "100";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 30, 57);

		//ss.str("");
		//ss << "Lives: " << m_lives;
		//RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 1), 3, 0, 7);

		ss.str("");
		ss.precision(5);
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 1, 1), 3, 0, 16);

		
	}
	break;
	case S_WIN:
	{
		//Loading in background texture
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		ss.str("");
		ss << "VICTORY!";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 6.f, 20.f, 25.f);

		ss.str("");
		ss << "ESC to quit.";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 3.f, 23.f, 18.f);
	}
	break;
	case S_LOSE:
	{
		//Loading in background texture
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		ss.str("");
		ss << "Game Over!";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 6.f, 15.f, 25.f);

		ss.str("");
		ss << "ESC to quit.";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 1), 3.f, 20.f, 20.f);

		gameStart = false;

		break;
	}
	case S_CREDITS:
	{
		//Loading in background texture
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		ss.str("");
		ss << "Jonathan: AI, physics";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2.3f, 5.f, 35.f);
		ss.str("");
		ss << "raycasting, code merger";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2.3f, 5.f, 33.f);

		ss.str("");
		ss << "Ernst: Ability, AbilityManager";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2.3f, 5.f, 25.f);
		ss.str("");
		ss << "Physics, Game Logic";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2.3f, 5.f, 22.f);

		ss.str("");
		ss << "Jerome: Ability & Sound";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2.3f, 5.f, 15.f);

		ss.str("");
		ss << "Darren: SceneManager & UI";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 0, 0), 2.3f, 5.f, 10.f);

		gameStart = false;

		break;
	}
	default:
		break;

	}
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
