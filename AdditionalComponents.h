#pragma once
#include "CMP203\Renderer203.h"
#include "Skateboard\Scene\Components.h"
#include "sktbdpch.h"

//-----Shape Components-----\\

struct ShapeComponent : public Skateboard::BaseComponent
{
	//vertex buffer
	std::vector<CMP203::Vertex> Vertices;

	//index buffer
	std::vector<uint32_t> Indicies;
};

struct PlaneComponent : public ShapeComponent {};
struct PyramidComponent : public ShapeComponent {};

//-----PhysicsComponents-----\\

struct MovementComponent : public Skateboard::BaseComponent
{
	float CurrentSpeed = 3.0f;
	float MaxSpeed = 20.0f;

	float CurrentAcceleration = 0.0f;
	float MaxAcceleration = 0.0f;

	glm::vec3 CurrentDirection{ 0, 0, 1 };
};

struct BoidsComponent : public MovementComponent
{
	float SeparationDistance = 100.0f;
	float CohesionDistance = 100.0f;
	float AlignmentDistance = 100.0f;

	float SeparationWeight = 0.3f;
	float CohesionWeight = 0.3f;
	float AlignmentWeight = 0.3f;

	glm::vec3 SeparationVector;
	glm::vec3 CohesionVector;
	glm::vec3 AlignmentVector;
};