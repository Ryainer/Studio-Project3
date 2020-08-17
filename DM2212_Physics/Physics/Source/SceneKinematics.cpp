#include "SceneKinematics.h"
#include "GL\glew.h"
#include "Application.h"
#include <sstream>

SceneKinematics::SceneKinematics()
{
}

SceneKinematics::~SceneKinematics()
{
}

void SceneKinematics::Init()
{
	SceneBase::Init();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Physics code here
	m_speed = 1.f;
	
	m_gravity.Set(0, -9.8f, 0); //init gravity as 9.8ms-2 downwards
	Math::InitRNG();

	m_ghost = new GameObject(GameObject::GO_BALL);
	m_timeGO = new GameObject(GameObject::GO_BALL);

	//Exercise 1: construct 10 GameObject with type GO_BALL and add into m_goList
	for (int i = 0; i < 30; ++i)
	{
		m_goList.push_back(new GameObject(GameObject::GO_BALL));
		m_goCList.push_back(new GameObject(GameObject::GO_CUBE));
	}
   
	
	
}

void SceneKinematics::Update(double dt)
{
	SceneBase::Update(dt);

	double x, y;

	Application::GetCursorPos(&x, &y);
	int w = Application::GetWindowWidth();
	int h = Application::GetWindowHeight();

	//Calculating aspect ratio
	m_worldHeight = 100.f;
	m_worldWidth = m_worldHeight * (float)Application::GetWindowWidth() / Application::GetWindowHeight();

	//Keyboard Section
	if(Application::IsKeyPressed('1'))
		glEnable(GL_CULL_FACE);
	if(Application::IsKeyPressed('2'))
		glDisable(GL_CULL_FACE);
	if(Application::IsKeyPressed('3'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(Application::IsKeyPressed('4'))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	if(Application::IsKeyPressed(VK_OEM_PLUS))
	{
		//Exercise 6: adjust simulation speed
		m_speed += dt * 10;
	}
	if(Application::IsKeyPressed(VK_OEM_MINUS))
	{
		//Exercise 6: adjust simulation speed
		m_speed -= dt * 10;
		m_speed = max(m_speed, 0);
	}
	if(Application::IsKeyPressed('C'))
	{
		//Exercise 9: clear screen
		for (std::vector<GameObject*>::iterator teh = m_goList.begin(); teh != m_goList.end(); ++teh)
		{
			GameObject* ball = (GameObject*)*teh;

			for (std::vector<GameObject*>::iterator et = m_goCList.begin(); et != m_goCList.end(); ++et)
			{
				GameObject* cube = (GameObject*)*et;

				
					ball->active = false;
					cube->active = false;
				
			}
		}
	}
	if(Application::IsKeyPressed(VK_SPACE))
	{
		float spawnX = rand() % 100 + 1;
		float spawnY = rand() % 100 + 1;
		//Exercise 9: spawn balls
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active) continue;
			go->active = true;
			go->type = GameObject::GAMEOBJECT_TYPE(1);
			go->pos.Set(spawnX, spawnY, 0);

			go->vel = Vector3(20, 20, 0);
			break;
		}
	}
	if(Application::IsKeyPressed('V'))
	{
		//Exercise 9: spawn obstacles
		float spawnX = rand() % 100 + 1;
		float spawnY = rand() % 100 + 1;

		for (std::vector<GameObject*>::iterator it = m_goCList.begin(); it != m_goCList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active) continue;
			go->active = true;
			go->type = GameObject::GAMEOBJECT_TYPE(2);
			go->pos.Set(spawnX, spawnY, 0);


			break;
		}
	}

	//Mouse Section
	static bool bLButtonState = false;
	//Exercise 10: ghost code here
	if(!bLButtonState && Application::IsMousePressed(0))
	{
		bLButtonState = true;
		std::cout << "LBUTTON DOWN" << std::endl;
		
		//Exercise 10: spawn ghost ball
		
			m_ghost->active = true;
			m_ghost->type = GameObject::GAMEOBJECT_TYPE(1);
			m_ghost->pos.Set((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);
	}
	else if(bLButtonState && !Application::IsMousePressed(0))
	{
		bLButtonState = false;
		std::cout << "LBUTTON UP" << std::endl;

		m_timeTaken1 = 0;
		m_heightMax = 0;
		m_timeTaken2 = 0;
	
		//Exercise 4: spawn ball
		for (std::vector<GameObject*>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active) continue;
			go->active = true;
			go->type = GameObject::GAMEOBJECT_TYPE(1);
			go->pos.Set((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);
			go->vel = Vector3((m_ghost->pos.x - (x / w) * m_worldWidth), (m_ghost->pos.y - ((h - y) / h) * m_worldHeight), 0);
			

			m_timeEstimated1 = (0 - go->vel.y) / m_gravity.y;
            
           

			if ((((-1 * (float)go->vel.y) + (float)sqrt(((go->vel.y * go->vel.y) - (4.f * m_gravity.y * go->pos.y)))) / (2.f * m_gravity.y)) > Math::EPSILON)
			{
				m_timeEstimated2 = (((-1.f * (float)go->vel.y) + (float)sqrt(((go->vel.y * go->vel.y) - (4.f * m_gravity.y * go->pos.y)))) / (2.f * m_gravity.y));
			}
			else
			{
				m_timeEstimated2 = (((-1.f * (float)go->vel.y) - (float)sqrt(((go->vel.y * go->vel.y) - (4.f * m_gravity.y * go->pos.y)))) / (2.f * m_gravity.y));
			}


			m_heightEstimated = 0.5f * go->vel.y * (m_timeEstimated1 / (m_speed / 12.f));
			m_heightEstimated += go->pos.y;
	
			
			m_timeGO->pos.x = go->pos.x;
			m_timeGO->pos.y = go->pos.y;
			m_timeGO->pos.z = go->pos.z;

			m_timeGO->vel = go->vel;
			

			break;
		}
		
		m_ghost->active = false;

		//Exercise 10: replace Exercise 4 code and use ghost to determine ball velocity
	

		//Exercise 11: kinematics equation
		
		//v = u + a * t
		
		//t = (v - u ) / a

		//v * v = u * u + 2 * a * s
		//s = - (u * u) / (2 * a)
						
		//s = u * t + 0.5 * a * t * t
		//(0.5 * a) * t * t + (u) * t + (-s) = 0

		
	}
	
	static bool bRButtonState = false;
	if(!bRButtonState && Application::IsMousePressed(1))
	{
		bRButtonState = true;
		std::cout << "RBUTTON DOWN" << std::endl;

		
		
		

		//Exercise 7: spawn obstacles using GO_CUBE
		for (std::vector<GameObject*>::iterator it = m_goCList.begin() ; it != m_goCList.end(); ++it)
		{
			GameObject* go = (GameObject*)*it;
			if (go->active) continue;
			go->active = true;
			go->type = GameObject::GAMEOBJECT_TYPE(2);
			go->pos.Set((x / w) * m_worldWidth, ((h - y) / h) * m_worldHeight, 0);

			
			break;
		}

		
		
	}
	else if(bRButtonState && !Application::IsMousePressed(1))
	{
		bRButtonState = false;
		std::cout << "RBUTTON UP" << std::endl;
	}

	//Physics Simulation Section
	fps = (float)(1.f / dt);

	//Exercise 11: update kinematics information
	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			if(go->type == GameObject::GO_BALL)
			{
				//Exercise 2: implement equation 1 & 2
				go->vel += m_gravity * dt * m_speed;
				go->pos += go->vel * dt * m_speed;
				
				if (go->vel.y > Math::EPSILON)
				{
					m_timeTaken1 += dt;
					m_heightMax = go->pos.y;
				}
				else
				{
					m_timeTaken2 += dt;
				}

				

				//Exercise 12: replace Exercise 2 code and use average speed instead
			}

			//Exercise 8: check collision with GO_CUBE
			for (std::vector<GameObject*>::iterator teh = m_goList.begin(); teh != m_goList.end(); ++teh)
			{
				GameObject* ball = (GameObject*)*teh;

				for (std::vector<GameObject*>::iterator et = m_goCList.begin(); et != m_goCList.end(); ++et)
				{
					GameObject* cube = (GameObject*)*et;
					
					float distance = sqrt(((ball->pos.x - cube->pos.x) * (ball->pos.x - cube->pos.x)) + ((ball->pos.y - cube->pos.y) * (ball->pos.y - cube->pos.y)));

					if (distance < 2)
					{
						ball->active = false;
						cube->active = false;
					}
				}
			}

			

			//Exercise 5: unspawn ball when outside window
			if (go->pos.x > m_worldWidth || go->pos.x < 0 || go->pos.y < 0 || go->pos.y > m_worldHeight) {
				go->active = false;
			}
		}
	}
}

void SceneKinematics::RenderGO(GameObject *go)
{
	switch(go->type)
	{
	case GameObject::GO_BALL:
		//Exercise 3: render a sphere with radius 1
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(go->scale.x, go->scale.y, go->scale.z);
		RenderMesh(meshList[GEO_BALL], false);
		modelStack.PopMatrix();
		break;
	case GameObject::GO_CUBE:
		//Exercise 7: render a cube with length 2
		modelStack.PushMatrix();
		modelStack.Translate(go->pos.x, go->pos.y, go->pos.z);
		modelStack.Scale(2, 2, 2);
		RenderMesh(meshList[GEO_CUBE], false);
		modelStack.PopMatrix();
		break;
	}
}

void SceneKinematics::Render()
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

	for(std::vector<GameObject *>::iterator it = m_goList.begin(); it != m_goList.end(); ++it)
	{
		GameObject *go = (GameObject *)*it;
		if(go->active)
		{
			RenderGO(go);
		}
	}
	if(m_ghost->active)
	{
		RenderGO(m_ghost);
	}
	
	for (std::vector<GameObject*>::iterator et = m_goCList.begin(); et != m_goCList.end(); ++et)
	{
		GameObject* go = (GameObject*)*et;
		if (go->active)
		{
			RenderGO(go);
		}
	}




	//On screen text
	std::ostringstream ss;
	ss.precision(5);
	ss << "FPS: " << fps;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 3, 0, 3);
	
	//Exercise 6: print simulation speed

	ss.str("");
	ss.clear();
	ss << "Speed: " << m_speed;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2.5, 50, 1);

	
	//Exercise 10: print m_ghost position and velocity informatio
	ss.str("");
	ss.clear();
	ss << "Position: " << m_ghost->pos;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 0, 10);
	ss.str("");
	ss.clear();
	ss << "Velocity: " << m_ghost->vel;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 0, 12);

	//Exercise 11: print kinematics information
	ss.str("");
	ss.clear();
	ss << "timeestimated1 "<< m_timeEstimated1;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 1, 57);
	ss.str("");
	ss.clear();
	ss << "timeestimated2 " << m_timeEstimated2;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 1, 55);
	ss.str("");
	ss.clear();
	ss << "heightestimated " << m_heightEstimated;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 1, 53);
	ss.str("");
	ss.clear();
	ss << "timeetaken1 " << m_timeTaken1;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 1, 51);
	ss.str("");
	ss.clear();
	ss << "timeetaken2 " << m_timeTaken2;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 1, 49);
	ss.str("");
	ss.clear();
	ss << "heightMax " << m_heightMax;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 1, 46);
	RenderTextOnScreen(meshList[GEO_TEXT], "Kinematics", Color(0, 1, 0), 3, 0, 58);

	//velocity and position of last ball
	ss.str("");
	ss.clear();
	ss << "Last Velocity: " << m_timeGO->vel;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 0, 19);
	ss.str("");
	ss.clear();
	ss << "Last Position: " << m_timeGO->pos;
	RenderTextOnScreen(meshList[GEO_TEXT], ss.str(), Color(0, 1, 0), 2, 0, 15);
}

void SceneKinematics::Exit()
{
	// Cleanup VBO
	for(int i = 0; i < NUM_GEOMETRY; ++i)
	{
		if(meshList[i])
			delete meshList[i];
	}
	glDeleteProgram(m_programID);
	glDeleteVertexArrays(1, &m_vertexArrayID);
	
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
