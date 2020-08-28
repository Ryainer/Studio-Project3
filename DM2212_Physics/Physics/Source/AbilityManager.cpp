#include "AbilityManager.h"

AbilityManager::AbilityManager()
{
}

AbilityManager::~AbilityManager()
{
}
//actor, actee, player
void AbilityManager::DoAbility(GameObject* go1, GameObject* go2, GameObject* player)//go1 is the projectile, go2 is the target
{
	switch (go1->type)//change this to take into account level
	{
	case GameObject::GO_PROJECTILE:
	{
		int randHealth = Math::RandIntMinMax(1, 100);
		go2->health--;
		go1->active = false;
		if (go2->health < 1)
		{
			go2->active = false;
			std::cout << "yeet" << std::endl;
		}
		if (randHealth < 25)
		{
			player->health++;
		}
		break;
	}
	case GameObject::GO_BOOMERANG:
	{
		if (go1->range > 0)
		{
			go2->health--;
			//std::cout << go1->health << std::endl;
		}
		if (go1->range <= 0)
		{
			go2->health -= 2;
			player->health++;
			//std::cout << go1->health << std::endl;
			//std::cout << player->health << std::endl;
		}

		if (go2->health < 1)
		{
			go2->active = false;
		}
		std::cout << "yeetus" << std::endl;
		break;
	}
	}

}
