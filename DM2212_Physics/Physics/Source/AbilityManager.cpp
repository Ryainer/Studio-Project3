#include "AbilityManager.h"

AbilityManager::AbilityManager()
{
}

AbilityManager::~AbilityManager()
{
}
//actor, actee, player
void AbilityManager::DoAbility(GameObject* go1, GameObject* go2, GameObject* player)
{
	switch (go2->type)//change this to take into account level
	{
	case GameObject::GO_PROJECTILE:
	{
		go1->health--;
		go2->active = false;
		if (go1->health < 1)
		{
			go1->active = false;
		}
		break;
	}
	case GameObject::GO_BOOMERANG:
	{
		if (go2->range > 0)
		{
			go1->health--;
			//std::cout << go1->health << std::endl;
		}
		if (go2->range <= 0)
		{
			go1->health -= 2;
			player->health++;
			//std::cout << go1->health << std::endl;
			//std::cout << player->health << std::endl;
		}

		if (go1->health < 1)
		{
			go1->active = false;
		}

		break;
	}
	}

}
