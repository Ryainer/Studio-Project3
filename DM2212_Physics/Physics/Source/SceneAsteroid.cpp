#include "SceneAsteroid.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>
#include <MyMath.h>

const float SceneAsteroid::ROTATION_POWER = 3.f;
static float i_frames = 0.f;

SceneAsteroid::SceneAsteroid()
{
	
}

SceneAsteroid::~SceneAsteroid()
{
}

void SceneAsteroid::Init()
{
	SceneBase::Init();

	PI = 3.14159265;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;

	elapsedtime = prevElapsed = 0;
	
	Math::InitRNG();

	//Exercise 2a: Construct 100 GameObject with type GO_ASTEROID and add into m_goList
	for (int i = 0; i < 100; ++i)
	{
		m_goList.push_back(new GameObject(GameObject::GO_ASTEROID));
	}
    
	Level = 1;

	//Exercise 2b: Initialize m_lives and m_score
	m_lives = 3;
	m_score = 0;

	minionCounter = 0;
	cooldown = 10.f;
	enemyHealth = 3;

	
	powerupchck = 0;

	//Exercise 2c: Construct m_ship, set active, type, scale and pos
	m_ship = new GameObject(GameObject::GO_SHIP);
	m_ship->active = true;
	m_ship->type = GameObject::GO_SHIP;
	m_ship->scale.Set(3.5f, 3.5f, 3.5f);
	m_ship->pos.Set(m_worldWidth/2, m_worldHeight/2, 0.f);
	m_ship->vel.Set(0.f, 0.f, 0.f);

	m_ship->momentofinertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	m_ship->angularVelocity = 0.f;

	m_ship->health = 20;

	
	asteroidCounter = 0;
	enemyCounter = 0;
	counter = 0;
	miniasteroidCounter = 0;
	bosscounter = 0;
	
	enemycheck = true;
	asteroidcheck = true;
	lvlchck = true;
	missilechck = true;

	activatespray = false;

	scenechanger = false;

	timeLimit = 0;

	if (GetLevel() == 1)
	{
		enemy_remaining = 1;
		asteroid_remaining = 55;
	}
	

	missiles_remaining = 5;

	boss_remaining = 1;

	m_torque.Set(0, 0, 0);

	angle = 0;
}

GameObject* SceneAsteroid::FetchGO()
{
	//Exercise 3a: Fetch a game object from m_goList and return it
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
			m_goList.push_back(new GameObject(GameObject::GO_ASTEROID));
			
		}
		m_goList.at(previousMaxSize)->active = true;
		
	return m_goList.at(previousMaxSize);
}

void SceneAsteroid::UpdateMinion(double dt)
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



int SceneAsteroid::GetLevel()
{
	return Level;
}

int SceneAsteroid::getAsteroidRemainder()
{
	return asteroid_remaining;
}

int SceneAsteroid::getEnemiesRemainder()
{
	return enemy_remaining;
}

int SceneAsteroid::getMissilesRemainder()
{
	return missiles_remaining;
}

int SceneAsteroid::getBossRemainder()
{
	return boss_remaining;
}

//only the closest enemies or asteroids
GameObject* SceneAsteroid::GetClosestGo(GameObject* current) const
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

GameObject* SceneAsteroid::Enemyclosest(GameObject* ship) const //for boss ship missile to track player
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

float SceneAsteroid::ForceBtwObjects(GameObject* one, GameObject* two)
{

	float r = (one->pos - two->pos).LengthSquared();

	return  (10.f * ((one->mass * two->mass) / (r)));
}


static Vector3 RotateVector(const Vector3& vec, float radian)
{
	return Vector3(vec.x * cos(radian) + vec.y * -sin(radian), vec.x * sin(radian) + vec.y * cos(radian), 0.f);
	
}

void SceneAsteroid::Update(double dt)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	SceneBase::Update(dt);

	elapsedtime += dt;

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();
	
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
	//Exercise 6: set m_force values based on WASD
	if(Application::IsKeyPressed('W'))
	{
		m_force += m_ship->dir * 100.f;
		

		//m_force.y += 5;
	}
	if(Application::IsKeyPressed('A'))
	{
		m_force += m_ship->dir * 5;
		m_torque += Vector3(-m_ship->scale.x, -m_ship->scale.y, 0).Cross(Vector3(5 ,0, 0));
	}
	if(Application::IsKeyPressed('S'))
	{
		m_force -= m_ship->dir * 100;
		
		//m_force.y -= 5;
	}
	if(Application::IsKeyPressed('D'))
	{
		m_force += m_ship->dir * 5;
		m_torque += Vector3(-m_ship->scale.x, m_ship->scale.y, 0).Cross(Vector3(5, 0, 0));
	}
	//Exercise 8: use 2 keys to increase and decrease mass of ship
	if (Application::IsKeyPressed('M'))
	{
		m_ship->mass += 2 * dt;
		m_ship->momentofinertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	}
	if (Application::IsKeyPressed('N'))
	{
		m_ship->mass -= 2 * dt;
		if (m_ship->mass <= 0.1)
		{
			m_ship->mass = 0.1;
		}
		m_ship->momentofinertia = m_ship->mass * m_ship->scale.x * m_ship->scale.x;
	}

	
	//Exercise 11: use a key to spawn some asteroids
	if (Level <= 3 && getAsteroidRemainder() > 0) //chks if less than or equal to 3 for levels and that there is still asteroids
	{
		if (asteroidCounter <= getAsteroidRemainder()) //renders out asteroids until counter is less than or equal
		{
			GameObject* asteroids = FetchGO();
			//asteroids->active = true;
			asteroids->type = GameObject::GO_ASTEROID;
			asteroids->scale.Set(4.5f, 4.5f, 4.5f);
			//asteroids->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), Math::RandFloatMinMax(0, m_worldHeight), 0);
			asteroids->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), (rand() % 5) * (int)m_worldHeight, 0);
			asteroids->vel = Vector3(Math::RandFloatMinMax(-5, 5), Math::RandFloatMinMax(-5, 5), 0);
			++asteroidCounter;
			
		} 

		if (GetLevel() == 2 && asteroidcheck == true) //these if and else if chcks the level and ensures that counter would reset to 0
		{
			asteroidCounter = 0;
			asteroidcheck = false;
		}
		else if (GetLevel() == 3 && asteroidcheck == false)
		{
			asteroidCounter = 0;
			asteroidcheck = true;
		}

	}
	//Exercise 14: use a key to spawn a bullet
	//Exercise 15: limit the spawn rate of bullets
	if (Application::IsKeyPressed(VK_SPACE))
	{
		float timedifference = elapsedtime - prevElapsed;
		
		if (activatespray == false)
		{
			if (timedifference > 0.2f)
			{
				GameObject* Bullets = FetchGO();
				//Bullets->active = true;
				Bullets->type = GameObject::GO_BULLET;
				Bullets->scale.Set(0.2f, 0.2f, 0.2f);
				Bullets->pos.Set(m_ship->pos.x, m_ship->pos.y, 0);
				Bullets->vel = m_ship->dir.Normalized() * BULLET_SPEED;
				prevElapsed = elapsedtime;

			}
		}
			
			
			if (activatespray == true && timeLimit <= 20)
			{
				for (int bullet = 0; bullet < 10; ++bullet)
				{
					if (timedifference > 1.f)
					{
						GameObject* Bullets = FetchGO();
						//Bullets->active = true;
						Bullets->type = GameObject::GO_BULLET;
						Bullets->scale.Set(0.2f, 0.2f, 0.2f);
						Bullets->pos.Set(m_ship->pos.x, m_ship->pos.y, 0);
						Bullets->vel = RotateVector(m_ship->dir.Normalized() * BULLET_SPEED,
							-Math::PI / 24 * 4);
						Bullets->vel = RotateVector(Bullets->vel, Math::PI / 12 * (bullet));
						
						timeLimit += 1;
						prevElapsed = elapsedtime;
						
					}
				}
				if (timeLimit >= 20)
				{
					activatespray = false;
					timeLimit = 0;
				}
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
			go->type = GameObject::GO_ENEMY;
			go->scale.Set(3.5f, 3.5f, 0);
			go->health = 3;
			enemyHealth = go->health;
			go->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), (rand() % 2) * (int)m_worldHeight, 0);
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

	if (GetLevel() == 3 && getBossRemainder() > 0) //spawns boss
	{
		if (bosscounter < getBossRemainder())
		{
			GameObject* Boss = FetchGO();
			Boss->type = GameObject::GO_BOSS;
			Boss->scale.Set(10.f, 10.f, 10.f);
			Boss->health = 20;
			Boss->pos.Set(Math::RandFloatMinMax(0, m_worldWidth), (rand() % 2) * (int)m_worldHeight, 0);
			bosscounter++;
		}
		

	}

	if (Application::IsKeyPressed('X'))
	{
		float timedifference = elapsedtime - prevElapsed;
		if (timedifference > 0.2f && counter <= 5) //makes sure that player has limited amt of missiles
		{
			GameObject* Missile = FetchGO();
			//Missile->active = true;
			Missile->type = GameObject::GO_MISSILE;
			Missile->scale.Set(2.5f, 2.5f, 2.5f);
			Missile->pos.Set(m_ship->pos.x, m_ship->pos.y, 0);
			m_ship->vel = m_ship->dir.Normalized() * MISSILE_SPEED;
			prevElapsed = elapsedtime;
			++counter;
			--missiles_remaining;
		}
		if (missiles_remaining < 5) //chcks if less than 5
		{
			if (GetLevel() == 2 && missilechck == true)
			{
				counter = 5 - missiles_remaining;//counter is resetted to the remainder
				missilechck = false;
			}
			else if (GetLevel() == 3 && missilechck == false) //same here
			{
				counter = 5 - missiles_remaining;
				missilechck = true;
			}
		}
		
	}

	if (Application::IsKeyPressed('R') && m_lives == 0)
	{
		m_lives = 3;
		GameObject* reset = FetchGO();
		reset->active = true;
		counter = 0;
		asteroidCounter = 0;
		enemyCounter = 0;
		bosscounter = 0;
	}

	if (Application::IsKeyPressed(VK_RETURN))
	{
		scenechanger = true;
	}


	static float minion_bounceTime = 0;
	if (minion_bounceTime <= elapsedtime && minionCounter < 2)
	{
		GameObject* Minion = FetchGO();
		Minion->active = true;
		Minion->type = GameObject::GO_SMALLSHIP;
		Minion->scale.Set(2.f, 2.f, 2.f);
		Minion->pos.Set(m_worldWidth / 2.f - m_ship->dir.x, m_worldHeight / 2.f - m_ship->dir.y, 0.f);
		Minion->vel.Set(0.f, 0.f, 0.f);
		Minion->momentofinertia = (1.f / 12.f) * m_ship->mass * (1.f + 1.f);
		Minion->angularVelocity = 0.f;
		Minion->angle = 0.f;
		++minionCounter;
		minion_bounceTime = elapsedtime + 0.5f;
	}
	UpdateMinion(dt);
	double x, y;

	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();

	//Mouse Section
	static bool bLButtonState = false;
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;

		GameObject* hole = FetchGO();
		hole->active = true;
		hole->type = GameObject::GO_BLACKHOLE;
		hole->scale.Set(5, 5, 1);
		hole->vel.SetZero();
		hole->mass = 1000;
		hole->pos.Set((x / w)* m_worldWidth, (h - y) / h * m_worldHeight, 0);
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;
	}
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;

		GameObject* repel = FetchGO();
		repel->active = true;
		repel->type = GameObject::GO_REPELLER;
		repel->scale.Set(5, 5, 1);
		repel->mass = 550;
		repel->pos.Set((x / w)* m_worldWidth, (h - y) / h * m_worldHeight, 0);
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}

	//Physics Simulation Section
	//m_ship->vel = m_ship->dir.Normalized() * m_force.y;
	//Exercise 7: Update ship's velocity based on m_force
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
	
	//Exercise 9: wrap ship position if it leaves screen

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
	else if (GetLevel() == 2 && getAsteroidRemainder() <= 3 && getEnemiesRemainder() <= -1)
	{
		Level++;
	}
	else if (GetLevel() == 3 && getAsteroidRemainder() <= -1 && getEnemiesRemainder() <= -1 && getBossRemainder() <= 0)
	{
		Level = 4;
	}

	//sets the enemies for the next level 
	if (GetLevel() == 2 && lvlchck == true)
	{
		enemy_remaining = 2;
		asteroid_remaining = 60;
		missiles_remaining += 2;
		if (missiles_remaining >= 5)
			missiles_remaining = 5;
		lvlchck = false;
	}

	if (GetLevel() == 3 && lvlchck == false)
	{
		enemy_remaining = 2;
		asteroid_remaining = 25;
		missiles_remaining += 2;
		if (missiles_remaining >= 5)
			missiles_remaining = 5;
		lvlchck = true;
	}

	//disables ship
	if (GetLevel() == 4)
	{
		m_ship->vel.SetZero();
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			go->active = false;
		}
	}

	
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			go->pos += go->vel * dt * m_speed;
			//Exercise 12: handle collision between GO_SHIP and GO_ASTEROID using simple distance-based check

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
							m_score += 2;
							break;
						}
					}
					else if (Other->active == true && Other->type == GameObject::GO_ENEMY)
					{
						Vector3 tempDist = go->pos - Other->pos;

						if (tempDist.Length() < go->scale.x + Other->scale.x)
						{
							m_score += 2;
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
							m_score += 2;
							go->active = false;
							//enemy_remaining -= 1;
							Other->health -= 2;
							break;
						}
					}
				}
			}
			
			else if (go->type == GameObject::GO_BOSS_MISSILE)
			{
			 if (go->pos.x > m_worldWidth || go->pos.x < 0 || go->pos.y < 0 || go->pos.y > m_worldHeight)
			 {
				go->active = false;

				continue;
			 }

			 GameObject* closestShip = Enemyclosest(go);
			 if (closestShip != nullptr)
			 {
				 Vector3 tempDist = closestShip->pos - go->pos;
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

			 Vector3 tempDist = go->pos - m_ship->pos;

			 if (tempDist.Length() < go->scale.x + m_ship->scale.x)
			 {
				 m_score += 2;
				 go->active = false;
				 //enemy_remaining -= 1;
				 --m_ship->health;
				 break;
			 }

            }

			else if (go->type == GameObject::GO_BLACKHOLE ) // chck displacement, slap in acceleration to ensure direction
			{

				for (int i = 0; i < m_goList.size(); ++i)
				{
					if (m_goList[i]->active == true && m_goList[i]->type != GameObject::GO_BLACKHOLE )
					{
						if ((m_goList[i]->pos - go->pos).LengthSquared() < 2500.f)
						{
							if ((m_goList[i]->pos - go->pos).LengthSquared() < 4.f)
							{
								go->mass += m_goList[i]->mass;
								m_goList[i]->active = false;
								
							}
							else
							{
						
								Vector3 displacedvector =  (go->pos - m_goList[i]->pos).Normalized();
								float force = ForceBtwObjects(m_goList[i], go);

								m_goList[i]->vel += 1.f / m_goList[i]->mass * displacedvector * force * dt * m_speed;
							}
						}
					}
					
				}
				
					if ((m_ship->pos - go->pos).LengthSquared() < 2500.f)
					{
						if ((m_ship->pos - go->pos).LengthSquared() < 4.f)
						{
							go->mass += m_ship->mass;
							m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0.f);
							--m_lives;
						}
						else
						{
							Vector3 displacedvector = (go->pos - m_ship->pos).Normalized();
							float force = ForceBtwObjects(m_ship, go);

							

							//go->vel += BHAcceleration * dt * m_speed;

							m_ship->vel += 1.f / m_ship->mass * displacedvector * force * dt * m_speed;
						}
					}
				
				

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

			else if (go->type == GameObject::GO_REPELLER) // chck displacement, slap in acceleration to ensure direction
			{

			 for (int i = 0; i < m_goList.size(); ++i)
			 {
				if (m_goList[i]->active == true &&  m_goList[i]->type != GameObject::GO_REPELLER)
				{
					if ((m_goList[i]->pos - go->pos).LengthSquared() < 2500.f)
					{
						
							Vector3 displacedvector =  (go->pos - m_goList[i]->pos).Normalized();
							float force = ForceBtwObjects(m_goList[i], go);

							m_goList[i]->vel -= 1.f / m_goList[i]->mass * displacedvector * force * dt * m_speed;
						
					}
				}

			 }

			 if ((m_ship->pos - go->pos).LengthSquared() < 2500.f)
			 {
				
			
					Vector3 displacedvector = (go->pos - m_ship->pos).Normalized();
					float force = ForceBtwObjects(m_ship, go);


					//go->vel += BHAcceleration * dt * m_speed;

					m_ship->vel -= 1.f / m_ship->mass * displacedvector * force * dt * m_speed;
				
			 }



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
			
			else if (go->type == GameObject::GO_SMALLSHIP)
			{
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
				  go2->vel = m_ship->dir.Normalized() * BULLET_SPEED;;
				 
			  }
            }

			else if (go->type == GameObject::GO_ENEMY)
			{
				if (go->health <= 0)
				{
					go->active = false;
					enemy_remaining -= 1;
					continue;
				}

				// Shoot
				if (go->bounceTime > 0.f) go->bounceTime -= dt;
				else
				{
					go->bounceTime = dt * 50 * (rand() % 2 + 1);

					GameObject* go2 = FetchGO();
					go2->active = true;
					go2->type = GameObject::GO_ENEMY_BULLET;
					go2->scale.Set(.5f, .5f, 0);
					go2->pos = go->pos;
					go2->vel.Set(go->dir.x * BULLET_SPEED, go->dir.y * BULLET_SPEED, 0);
				}

				// Follow
				Vector3 tempDist = m_ship->pos - go->pos;
				go->dir = tempDist.Normalized();

				if (tempDist.Length() > (m_ship->scale.x + go->scale.x) * (rand() % 10 + 1) * 10)
				{
					go->vel += 1.f / go->mass * go->dir * 100 * dt * m_speed;
					if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
						go->vel.Normalize() *= MAX_SPEED;
				}
				else 
				{
					go->vel.Set(2.5f, 2.5f, 0);
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

			else if (go->type == GameObject::GO_BOSS)
			{
			  if (go->health <= 0)
			  {
				  boss_remaining -= 1;
				  m_score += 100;
				go->active = false;
				
				continue;
			  }

			  if (go->bounceTime > -1.f)
			  {
				  go->bounceTime -= dt;
			  }
			  else
			  {
				  go->bounceTime = dt * 50 * (rand() % 2 + 1);

				  GameObject* Enemy_Missile = FetchGO();
				  //Missile->active = true;
				  Enemy_Missile->type = GameObject::GO_BOSS_MISSILE;
				  Enemy_Missile->scale.Set(1.5f, 1.5f, 1.5f);
				  Enemy_Missile->pos.Set(go->pos.x, go->pos.y, 0);
				  go->vel = go->dir.Normalized() * MISSILE_SPEED;
			  }


			  // Follow
			  Vector3 tempDist = m_ship->pos - go->pos;
			  go->dir = tempDist.Normalized();

			  if (tempDist.Length() > (m_ship->scale.x + go->scale.x) * (rand() % 10 + 1) * 10)
			  {
				go->vel += 1.f / go->mass * go->dir * 100 * dt * m_speed;
				if (go->vel.LengthSquared() > MAX_SPEED * MAX_SPEED)
					go->vel.Normalize() *= MAX_SPEED;
			  }
			  else
			  {
				go->vel.Set(1.5f, 1.5f, 0);
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
							m_score += 2;
                            //powerupchck = Math::RandIntMinMax(1, 5);
							//
							//
							//
							//if (powerupchck == 1)
							//{
							//	GameObject* powerup = FetchGO();
							//	powerup->active = true;
							//	powerup->type = GameObject::GO_HEALTHPOWERUP;
							//	powerup->scale.Set(5.f, 5.f, 0);
							//	powerup->vel.SetZero();
							//	powerup->pos = Other->pos;
							//	
							//}
							//else if (powerupchck == 5)
							//{
							//	GameObject* powerup = FetchGO();
							//	powerup->active = true;
							//	powerup->type = GameObject::GO_BULLETPOWERUP;
							//	powerup->scale.Set(5.f, 5.f, 0);
							//	powerup->vel.SetZero();
							//	powerup->pos = Other->pos;
							//}

							//for (int miniasteroid = 0; miniasteroid < 1; ++miniasteroid)
							//{
							//	GameObject* mini_asteroid = FetchGO();
							//	//Missile->active = true;
							//	mini_asteroid->type = GameObject::GO_MINIASTEROID;
							//	mini_asteroid->scale.Set(2.f, 2.f, 2.f);
							//	mini_asteroid->pos.Set(Other->pos.x - (Math::RandIntMinMax(1, 3)),
							//		Other->pos.y - (Math::RandIntMinMax(1, 3)), 0);
							//	mini_asteroid->vel.Set(Math::RandIntMinMax(-1,2), Math::RandIntMinMax(-1,2), 0);
							//	

							//}
								
								
							
							
							break;
						}

						


					}
					else if (Other->active == true && Other->type == GameObject::GO_ENEMY)
					{
						Vector3 tempDist = go->pos - Other->pos;

						if (tempDist.Length() < go->scale.x + Other->scale.x)
						{
							m_score += 2;
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
							m_score += 1;

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

			else if (go->type == GameObject::GO_MINION_BULLET)
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
						m_score += 2;
						powerupchck = Math::RandIntMinMax(1, 5);



						if (powerupchck == 1)
						{
							GameObject* powerup = FetchGO();
							powerup->active = true;
							powerup->type = GameObject::GO_HEALTHPOWERUP;
							powerup->scale.Set(5.f, 5.f, 0);
							powerup->vel.SetZero();
							powerup->pos = Other->pos;

						}
						else if (powerupchck == 5)
						{
							GameObject* powerup = FetchGO();
							powerup->active = true;
							powerup->type = GameObject::GO_BULLETPOWERUP;
							powerup->scale.Set(5.f, 5.f, 0);
							powerup->vel.SetZero();
							powerup->pos = Other->pos;
						}

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
						m_score += 2;
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
						m_score += 1;

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
			
	        else if (go->type == GameObject::GO_ENEMY_BULLET)
	          {
		        if (go->pos.y < 0 || go->pos.y >= m_worldHeight || go->pos.x < 0 || go->pos.x >= m_worldWidth)
		        {
			      go->active = false;
		        }
		        else if (i_frames <= 0.f)
		        {

			      Vector3 tempDist = go->pos - m_ship->pos;

			        if (tempDist.Length() < m_ship->scale.x + go->scale.x)
			        {
				       --m_ship->health;
				      i_frames = 1;
				      glClearColor(0.4f, 0.0f, 0.0f, 0.0f);
				      go->active = false;
					  
			        }
		        }
	          }

			else if (go->type == GameObject::GO_HEALTHPOWERUP)
			  {
			   float distance = sqrt(((go->pos.x - m_ship->pos.x) * (go->pos.x - m_ship->pos.x)) 
				  + ((go->pos.y - m_ship->pos.y) * (go->pos.y - m_ship->pos.y)));

			   if (distance < 4.5f)
			   {
				   go->active = false;
				   if (m_ship->health < 20)
				   {
					   m_ship->health += 2;
				   }
				   else if (m_ship->health == 20)
				   {
					   m_score += 1;
				   }
			   }
              }

		/*	else if (go->type == GameObject::GO_BULLETPOWERUP)
			{
			 float distance = sqrt(((go->pos.x - m_ship->pos.x) * (go->pos.x - m_ship->pos.x))
				+ ((go->pos.y - m_ship->pos.y) * (go->pos.y - m_ship->pos.y)));

			 if (distance < 4.5f)
			 {
				go->active = false;
				
				if (activatespray == false)
				{
					activatespray = true;
				}
			 }
            }*/
			 
			else if (go->type == GameObject::GO_MINIASTEROID) 
			  {
			   float distance = sqrt(((go->pos.x - m_ship->pos.x) * (go->pos.x - m_ship->pos.x)) 
				   + ((go->pos.y - m_ship->pos.y) * (go->pos.y - m_ship->pos.y)));

			   if (distance < 0.5f)
			   {
				  go->active = false;
				  m_ship->pos.Set(m_worldWidth / 2, m_worldHeight / 2, 0.f);
				  m_lives--;
				  asteroid_remaining -= 1;
			   }
              }
		}
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
}


void SceneAsteroid::RenderGO(GameObject *go, float z)
{

	switch(go->type)
	{
	case GameObject::GO_SHIP:
		//Exercise 4a: render a sphere with radius 1
		//Exercise 17a: render a ship texture or 3D ship model
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, z);
		modelStack.Rotate(Math::RadianToDegree(atan2(go->dir.y, go->dir.x)), 0.f, 0.f, 1.f);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_SHIP], false);
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
		break;
		//Exercise 17a: render a ship texture or 3D ship model
		//Exercise 17b:	re-orientate the ship with velocity
		
	}
}




void SceneAsteroid::Render()
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


	if (scenechanger == false) //the if and else if chcks what scene it is
	{
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_BG], false);
		modelStack.PopMatrix();

		std::ostringstream ss;
		ss.precision(5);
		ss << "Bootleg Asteroids";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 15, 30);

		ss.str("");
		ss.precision(5);
		ss << "Press Enter to Start!";
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 10, 25);
	}
	else if (scenechanger == true)
	{
		modelStack.PushMatrix();
		modelStack.Scale(500, 500, 0);
		RenderMesh(meshList[GEO_GAMEBG], false);
		modelStack.PopMatrix();

		float Zval = 0.001f;
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active)
			{
				RenderGO(go, Zval);
				Zval += 0.001f;
			}
		}

		//On screen text
		RenderGO(m_ship, Zval);
		//Exercise 5a: Render m_lives, m_score
		//Exercise 5b: Render position, velocity & mass of ship

		std::ostringstream ss;

		ss.precision(3);
		ss << "Lives: " << m_lives << std::endl;
		ss << "Score: " << m_score;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 55);

		ss.str("");
		ss.precision(3);
		ss << "Speed: " << m_speed;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 6);

		ss.str("");
		ss.precision(5);
		ss << "FPS: " << fps;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 3);

		ss.str("");
		ss.precision(5);
		ss << "Position: " << m_ship->pos;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2.5f, 0, 12);

		ss.str("");
		ss.precision(5);
		ss << "Velocity: " << m_ship->vel;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2.5f, 0, 15);

		ss.str("");
		ss.precision(5);
		ss << "Mass: " << m_ship->mass;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2.5f, 0, 18);

		for (int i = 0; i < m_goList.size(); i++)
		{
			if (m_goList[i]->type == GameObject::GO_ENEMY || m_goList[i]->type == GameObject::GO_BOSS)
			{
				ss.str("");
				ss.precision(5);
				ss << "Enemy Health: " << enemyHealth;
				RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2, 0, 20);
			}

		}

		ss.str("");
		ss.precision(5);
		if (GetLevel() == 1)
		{
			ss << "Asteroids Remaining: " << getAsteroidRemainder() + 1;
		}
		else if (GetLevel() == 2)
		{
			ss << "Asteroids Remaining: " << getAsteroidRemainder();
		}
		else if (GetLevel() == 3)
		{
			ss << "Asteroids Remaining: " << getAsteroidRemainder() + 3;
		}
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2, 0, 51.5f);

		ss.str("");
		ss.precision(5);
		if (GetLevel() == 1)
		{
			ss << "Enemies Remaining: " << getEnemiesRemainder() + 1; 
		}
		else if (GetLevel() == 2)
		{
			ss << "Enemies Remaining: " << getEnemiesRemainder() + 2; 
		}
		else if (GetLevel() == 3)
		{
			ss << "Enemies Remaining: " << getEnemiesRemainder() + 1; 
		}
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2, 0, 50);

		if (GetLevel() == 3)
		{
			ss.str("");
			ss.precision(5);
			ss << "Boss Remaining: " << getBossRemainder();
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2, 0, 58);
		}

		

		if (m_lives <= 0)
		{
			ss.str("");
			ss.precision(5);
			ss << "YOU LOSE! CONTINUE?";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 25);
		}

		ss.str("");
		ss.precision(5);
		ss << "Missiles Remaining: " << getMissilesRemainder() + 1;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2, 0, 53);

		ss.str("");
		ss.precision(5);
		ss << "Player Health: " << m_ship->health;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 48);

		ss.str("");
		ss.precision(5);
		ss << "Level " << Level;
		RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 2.5f, 57, 57);

		RenderTextOnScreen(meshList[GEO_TEXT], "Asteroid", Color(1, 0, 0), 3, 0, 0);

		if (GetLevel() == 4)
		{
			ss.str("");
			ss.precision(5);
			ss << "CONGRATS YOU BEAT THE GAME!!";
			RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(1, 0, 0), 3, 0, 25);
		}
	}
	
}

void SceneAsteroid::Exit()
{
	SceneBase::Exit();
	//Cleanup GameObjects
	while(m_goList.size() > 0)
	{
		GameObject *go = m_goList.back();
		delete go;
		m_goList.pop_back();
	}
	if(m_ship)
	{
		delete m_ship;
		m_ship = NULL;
	}
}
