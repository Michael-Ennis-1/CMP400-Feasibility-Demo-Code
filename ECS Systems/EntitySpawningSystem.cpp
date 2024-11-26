#include "EntitySpawningSystem.h"
#include "AdditionalComponents.h"

EntitySpawningSystem::EntitySpawningSystem()
{
}

void EntitySpawningSystem::AddEntities(int EntityAmount)
{
	for (int i = 0; i < EntityAmount; i++)
	{
		Skateboard::Entity Boid = Skateboard::ECS::CreateEntity("");
		Boid.AddComponent<PyramidComponent>();
		Boid.AddComponent<MovementComponent>();

		AllEntities.push_back(Boid);
	}
}

void EntitySpawningSystem::RemoveAllEntities()
{
	for (int i = 0; i < AllEntities.size(); i++)
	{
		Skateboard::Entity Boid = AllEntities[i];
		entt::registry& Reg = Skateboard::ECS::GetRegistry();

		Reg.remove<TransformComponent>(Boid);
		Reg.remove<TagComponent>(Boid);
		Reg.remove<MovementComponent>(Boid);
		Reg.remove<PyramidComponent>(Boid);

		Reg.destroy(Boid);
	}

	AllEntities.clear();
}
