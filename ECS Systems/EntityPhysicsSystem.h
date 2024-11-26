#pragma once

#include <functional>
#include "AdditionalComponents.h"

class EntityPhysicsSystem
{
public:
	EntityPhysicsSystem();

	void RegisterEntities();
	void RandomizeEntityDirections();

	void UpdateEntities(float DeltaTime);

	// Sets size of bounds that boids can traverse within
	void SetBoundingBoxContainerSize(float2 XCoordinateBounds, float2 YCoordinateBounds, float2 ZCoordinateBounds);

	void GetBoundingBoxContainerSize(float2& XCoordinateBounds, float2& YCoordinateBounds, float2& ZCoordinateBounds);

private:

	// Boids specific calculations, Move to separate Boids system
	void CalculateSeparation(TransformComponent& ThisTransformComponent, MovementComponent& ThisMovementComponent, float DeltaTime);
	void CalculateCohesion(TransformComponent& ThisTransformComponent, MovementComponent& ThisMovementComponent, float DeltaTime);
	void CalculateAlignment(TransformComponent& ThisTransformComponent, MovementComponent& ThisMovementComponent, float DeltaTime);

	// Changes direction for boid if out of bounds, reflects axis if out of bounds.
	void CheckIfOutsideBounds(TransformComponent& TransformComponentReference, MovementComponent& MovementComponentReference);

	// Clamp cooridinate within minimum/maximum bounds
	float AlignWithinBounds(float BoidPositionCoordinate, float MinimumBounds, float MaximumBounds);

	// Rotates immediately towards current direction
	void RotateTowardsCurrentDirection(TransformComponent& TransformComponentReference, MovementComponent& MovementComponentReference, float DeltaTime);

	// Store components as references - removes need to "GetComponent" every frame
	std::vector<std::reference_wrapper<TransformComponent>> TransformComponentReferenceWrappers;
	std::vector<std::reference_wrapper<MovementComponent>> MovementComponentReferenceWrappers;

	float2 XBoxBounds{ -5, 5 };
	float2 YBoxBounds{ -5, 5 };
	float2 ZBoxBounds{ -5, 5 };

	float DistanceFromEdgeOfBox = 0.1f;

public:
	// Boids related variables
	float MinimumSeparationDistance = 1;
	float MaximumSeparationDistance = 8;
	float SeparationDistanceWeight = 5;

	float MinimumAlignmentDistance = 1;
	float MaximumAlignmentDistance = 5;
	float AlignmentDistanceWeight = 4;

	float MaximumCohesionDistance = 5;
	float CohesionDistanceWeight = 1;
};