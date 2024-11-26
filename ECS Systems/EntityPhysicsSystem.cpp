#include "EntityPhysicsSystem.h"
#include "AdditionalComponents.h"
#include "Skateboard\Scene\Entity.h"

#include <random>
#include <cmath>

EntityPhysicsSystem::EntityPhysicsSystem()
{
	RegisterEntities();
	RandomizeEntityDirections();
}

void EntityPhysicsSystem::RegisterEntities()
{
	TransformComponentReferenceWrappers.clear();
	MovementComponentReferenceWrappers.clear();

	// Lamda used to grab each movement and transform component attached to all entities and stores them in vector
	ECS::GetRegistry().each([&](auto entity_id) {

		Skateboard::Entity entity{ entity_id };
		if (entity.HasComponent<MovementComponent>() && entity.HasComponent<TransformComponent>())
		{
			TransformComponentReferenceWrappers.push_back(entity.GetComponent<TransformComponent>());
			MovementComponentReferenceWrappers.push_back(entity.GetComponent<MovementComponent>());
		}

	});
}

void EntityPhysicsSystem::RandomizeEntityDirections()
{
	std::random_device RandomDevice;
	std::mt19937 MTEngine(RandomDevice());

	std::uniform_real_distribution<> RandomDistribution(-1.0, 1.0);

	for (int i = 0; i < MovementComponentReferenceWrappers.size(); i++)
	{
		MovementComponent& MovementComponentReference = MovementComponentReferenceWrappers[i];

		glm::vec3 RandomEntityDirection{ RandomDistribution(MTEngine), RandomDistribution(MTEngine), RandomDistribution(MTEngine) };
		RandomEntityDirection = glm::normalize(RandomEntityDirection);

		MovementComponentReference.CurrentDirection = RandomEntityDirection;
	}
}

void EntityPhysicsSystem::UpdateEntities(float DeltaTime)
{
	for (int i = 0; i < MovementComponentReferenceWrappers.size(); i++)
	{
		MovementComponent& MovementComponentReference = MovementComponentReferenceWrappers[i];
		TransformComponent& TransformComponentReference = TransformComponentReferenceWrappers[i];

		CalculateSeparation(TransformComponentReference, MovementComponentReference, DeltaTime);
		CalculateAlignment(TransformComponentReference, MovementComponentReference, DeltaTime);
		CalculateCohesion(TransformComponentReference, MovementComponentReference, DeltaTime);

		glm::vec3 Velocity = MovementComponentReference.CurrentDirection * MovementComponentReference.CurrentSpeed;
		TransformComponentReference.Trans.Translation += (Velocity * DeltaTime);

		RotateTowardsCurrentDirection(TransformComponentReference, MovementComponentReference, DeltaTime);
		CheckIfOutsideBounds(TransformComponentReference, MovementComponentReference);
	}
}

void EntityPhysicsSystem::CalculateSeparation(TransformComponent& ThisTransformComponent, MovementComponent& ThisMovementComponent, float DeltaTime)
{
	int BoidsAmountCalculated = 0;

	glm::vec3 FinalSeparationVector{0, 0, 0};
	glm::vec3 ThisPosition = ThisTransformComponent.Trans.Translation;

	for (int i = 0; i < TransformComponentReferenceWrappers.size(); i++)
	{
		TransformComponent& OtherTransformComponent = TransformComponentReferenceWrappers[i];
		if (OtherTransformComponent.m_entityId == ThisTransformComponent.m_entityId)
		{
			continue;
		}

		glm::vec3 OtherPosition = OtherTransformComponent.Trans.Translation;
		if (ThisPosition == OtherPosition)
		{
			return;
		}

		float DistanceFromOther = glm::distance(OtherPosition, ThisPosition);
		if (DistanceFromOther < MaximumSeparationDistance)
		{
			float DistanceWeight = 1;

			glm::vec3 DirectionVector = ThisPosition - OtherPosition;
			DirectionVector = glm::normalize(DirectionVector);

			DistanceFromOther -= MinimumSeparationDistance;
			float DistanceWeightDivisibleFactor = MaximumSeparationDistance - MinimumSeparationDistance;

			// If distance too close, leave distance weight as max (1)
			if (DistanceFromOther > 0)
			{
				DistanceWeight = 1 - (DistanceFromOther / DistanceWeightDivisibleFactor);
			}

			// If distance too far, don't calculate to avoid potential floating-point errors in mathematical calculation
			if (DistanceWeight > 0.01)
			{
				FinalSeparationVector += (DirectionVector * DistanceWeight);
				BoidsAmountCalculated++;
			}
		}
	}
	
	if (BoidsAmountCalculated > 0)
	{
		FinalSeparationVector /= BoidsAmountCalculated;

		ThisMovementComponent.CurrentDirection += (FinalSeparationVector * SeparationDistanceWeight * DeltaTime);
		ThisMovementComponent.CurrentDirection = glm::normalize(ThisMovementComponent.CurrentDirection);
	}
}

void EntityPhysicsSystem::CalculateCohesion(TransformComponent& ThisTransformComponent, MovementComponent& ThisMovementComponent, float DeltaTime)
{
	int BoidsAmountCalculated = 0;

	glm::vec3 FinalCohesionVector{ 0, 0, 0 };
	glm::vec3 AveragePositionOfNearbyBoids{ 0, 0, 0 };
	glm::vec3 ThisPosition = ThisTransformComponent.Trans.Translation;

	for (int i = 0; i < TransformComponentReferenceWrappers.size(); i++)
	{
		TransformComponent& OtherTransformComponent = TransformComponentReferenceWrappers[i];
		if (OtherTransformComponent.m_entityId == ThisTransformComponent.m_entityId)
		{
			continue;
		}

		glm::vec3 OtherPosition = OtherTransformComponent.Trans.Translation;
		if (ThisPosition == OtherPosition)
		{
			return;
		}

		float DistanceFromOther = glm::distance(OtherPosition, ThisPosition);
		if (DistanceFromOther < MaximumCohesionDistance)
		{
			AveragePositionOfNearbyBoids += OtherPosition;

			BoidsAmountCalculated++;
		}
	}

	if (BoidsAmountCalculated > 0)
	{
		AveragePositionOfNearbyBoids /= BoidsAmountCalculated;

		glm::vec3 CohesionDirectionVector = AveragePositionOfNearbyBoids - ThisPosition;
		CohesionDirectionVector = glm::normalize(CohesionDirectionVector);

		ThisMovementComponent.CurrentDirection += (CohesionDirectionVector * CohesionDistanceWeight * DeltaTime);
		ThisMovementComponent.CurrentDirection = glm::normalize(ThisMovementComponent.CurrentDirection);
	}
}

void EntityPhysicsSystem::CalculateAlignment(TransformComponent& ThisTransformComponent, MovementComponent& ThisMovementComponent, float DeltaTime)
{
	int BoidsAmountCalculated = 0;

	glm::vec3 FinalAlignmentVector{ 0, 0, 0 };
	glm::vec3 ThisPosition = ThisTransformComponent.Trans.Translation;

	for (int i = 0; i < TransformComponentReferenceWrappers.size(); i++)
	{
		TransformComponent& OtherTransformComponent = TransformComponentReferenceWrappers[i];
		if (OtherTransformComponent.m_entityId == ThisTransformComponent.m_entityId)
		{
			continue;
		}

		glm::vec3 OtherPosition = OtherTransformComponent.Trans.Translation;
		if (ThisPosition == OtherPosition)
		{
			return;
		}

		float DistanceFromOther = glm::distance(OtherPosition, ThisPosition);
		if (DistanceFromOther < MaximumAlignmentDistance)
		{
			float DistanceWeight = 1;

			MovementComponent& OtherMovementComponent = MovementComponentReferenceWrappers[i];

			glm::vec3 OtherDirectionVector = OtherMovementComponent.CurrentDirection;

			DistanceFromOther -= MinimumAlignmentDistance;
			float DistanceWeightDivisibleFactor = MaximumAlignmentDistance - MinimumAlignmentDistance;

			// If distance too close, leave distance weight as max (1)
			if (DistanceFromOther > 0)
			{
				DistanceWeight = 1 - (DistanceFromOther / DistanceWeightDivisibleFactor);
			}

			// If distance too far, don't calculate to avoid potential floating-point errors in mathematical calculation
			if (DistanceWeight > 0.01)
			{
				FinalAlignmentVector += (OtherDirectionVector * DistanceWeight);

				BoidsAmountCalculated++;
			}
		}
	}

	if (BoidsAmountCalculated > 0)
	{
		FinalAlignmentVector /= BoidsAmountCalculated;

		ThisMovementComponent.CurrentDirection += (FinalAlignmentVector * AlignmentDistanceWeight * DeltaTime);
		ThisMovementComponent.CurrentDirection = glm::normalize(ThisMovementComponent.CurrentDirection);
	}
}

void EntityPhysicsSystem::CheckIfOutsideBounds(TransformComponent& TransformComponentReference, MovementComponent& MovementComponentReference)
{
	glm::vec3 BoidTranslation = TransformComponentReference.Trans.Translation;

	// Reflection on YZ Plane
	if (BoidTranslation.x > XBoxBounds.y || BoidTranslation.x < XBoxBounds.x)
	{
		MovementComponentReference.CurrentDirection.x *= -1;
		TransformComponentReference.Trans.Translation.x = AlignWithinBounds(BoidTranslation.x, XBoxBounds.x, XBoxBounds.y);
	}

	// Reflection on XZ Plane
	if (BoidTranslation.y > YBoxBounds.y || BoidTranslation.y < YBoxBounds.x)
	{
		MovementComponentReference.CurrentDirection.y *= -1;
		TransformComponentReference.Trans.Translation.y = AlignWithinBounds(BoidTranslation.y, YBoxBounds.x, YBoxBounds.y);
	}
	
	// Reflection on XY Plane
	if (BoidTranslation.z > ZBoxBounds.y || BoidTranslation.z < ZBoxBounds.x)
	{
		MovementComponentReference.CurrentDirection.z *= -1;
		TransformComponentReference.Trans.Translation.z = AlignWithinBounds(BoidTranslation.z, ZBoxBounds.x, ZBoxBounds.y);
	}
}

float EntityPhysicsSystem::AlignWithinBounds(float BoidPositionCoordinate, float MinimumBounds, float MaximumBounds)
{
	float NewCoordinate = 0;

	float DistanceFromMaxBounds = (BoidPositionCoordinate - MaximumBounds) * (BoidPositionCoordinate - MaximumBounds);
	float DistanceFromMinBounds = (BoidPositionCoordinate - MinimumBounds) * (BoidPositionCoordinate - MinimumBounds);

	if (DistanceFromMaxBounds <= DistanceFromMinBounds)
	{
		NewCoordinate = MaximumBounds - DistanceFromEdgeOfBox;
	}
	else
	{
		NewCoordinate = MinimumBounds + DistanceFromEdgeOfBox;
	}

	return NewCoordinate;
}

void EntityPhysicsSystem::SetBoundingBoxContainerSize(float2 XCoordinateBounds, float2 YCoordinateBounds, float2 ZCoordinateBounds)
{
	XBoxBounds = XCoordinateBounds;
	YBoxBounds = YCoordinateBounds;
	ZBoxBounds = ZCoordinateBounds;
}

void EntityPhysicsSystem::GetBoundingBoxContainerSize(float2& XCoordinateBounds, float2& YCoordinateBounds, float2& ZCoordinateBounds)
{
	XCoordinateBounds = XBoxBounds;
	YCoordinateBounds = YBoxBounds;
	ZCoordinateBounds = ZBoxBounds;
}

void EntityPhysicsSystem::RotateTowardsCurrentDirection(TransformComponent& TransformComponentReference, MovementComponent& MovementComponentReference, float DeltaTime)
{
	glm::vec3 Up = TransformComponentReference.Trans.GetUpVector();

	glm::vec3 TargetDirectionNormalized = glm::normalize(MovementComponentReference.CurrentDirection);
	TransformComponentReference.Trans.LookAt((TargetDirectionNormalized * 10) + TransformComponentReference.Trans.Translation);
	TransformComponentReference.Trans.Rotate(glm::vec3(glm::radians(90.0f), 0, glm::radians(90.0f)));
}