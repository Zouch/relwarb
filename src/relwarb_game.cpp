
#include "relwarb_game.h"
#include "relwarb_entity.h"
#include "relwarb.h"

void CreateDashSkill(Skill* skill, Entity* entity)
{
	skill->isActive = false;
	skill->triggerHandle = &DashTrigger;
	skill->applyHandle = &DashApply;
	skill->collideHandle = nullptr;

	skill->manaCost = 1;
	// NOTE(Thomas): Magic numbers to tailor
	skill->duration = 0.25f;
	skill->horizDistance = entity->shape->size.y * 4.f;
}

void DashTrigger(Skill* skill, Entity* entity)
{
	if (!skill->isActive && entity->mana >= skill->manaCost)
	{
		if (entity->controller->moveLeft || entity->controller->moveRight)
		{
			entity->mana -= skill->manaCost;
			skill->isActive = true;
			skill->elapsed = 0.f;

			if (entity->controller->moveLeft)
			{
				skill->direction = -1.f;
			}
			else // entity->controller->moveRight
			{
				skill->direction = 1.f;
			}
		}
	}
}

void DashApply(Skill* skill, Entity* executive, real32 dt)
{
	if (skill->isActive)
	{
		skill->elapsed += dt;
		if (skill->elapsed >= skill->duration)
		{
			skill->isActive = false;

			dt -= skill->elapsed - skill->duration;
		}

		real32 ratio = dt / skill->duration;
		executive->p.x += skill->direction * ratio * skill->horizDistance;
		executive->dp.y = 0.f;
	}
}

void UpdateGameLogic(GameState* gameState, real32 dt)
{

}