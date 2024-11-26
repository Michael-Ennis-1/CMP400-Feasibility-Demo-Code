#pragma once

#include "Skateboard\Scene\Entity.h"

class Skateboard::Entity;

class EntitySpawningSystem
{
public:
	EntitySpawningSystem();

	void AddEntities(int EntityAmount);
	void RemoveAllEntities();

	std::vector<Skateboard::Entity> AllEntities;
};
