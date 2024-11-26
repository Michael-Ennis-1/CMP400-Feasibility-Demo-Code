#pragma once

#include "CMP203\Renderer203.h"
#include "AdditionalComponents.h"
#include <functional>

class Skateboard::Entity;

class EntityRendererSystem
{
public:
	EntityRendererSystem();

	// Adds verticies and indices of required shape type for rendering
	void InitializeShapeComponent(Skateboard::Entity EntityID);

	// Generate Shapes
	void GeneratePlane(PlaneComponent& PlaneComponentRefernece);
	void GeneratePyramid(PyramidComponent& PyramidComponentReference);

	// Renders all entities found to contain a "ShapeComponent"
	void RenderEntities(CMP203::Renderer203 &m_renderer);

	// Registers references in vector that will be rendered when "RenderEntities" is called
	void RegisterEntities();

private:
	std::vector<std::reference_wrapper<ShapeComponent>> ShapeComponentReferenceWrappers;
	std::vector<std::reference_wrapper<TransformComponent>> TransformComponentReferenceWrappers;

	CMP203::InstanceData EntityInstanceData;
};
