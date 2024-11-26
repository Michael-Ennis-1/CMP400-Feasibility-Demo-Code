#include "EntityRendererSystem.h"
#include "Skateboard/Assets/AssetManager.h"

EntityRendererSystem::EntityRendererSystem()
{
	RegisterEntities();
	
	EntityInstanceData.TextureIndex = GraphicsContext::Context->GetViewHeapIndex(AssetManager::GetTexture("crate"));
	EntityInstanceData.ColourScale = float4(1, 0.3, 0.3, 1);
	EntityInstanceData.SpecularColor = float4(1, 1, 1, 1);
	EntityInstanceData.SpecularPower = 200;
	EntityInstanceData.SpecularWeight = 0.5f;
}

void EntityRendererSystem::InitializeShapeComponent(Skateboard::Entity EntityID)
{
	// Generate vertices and indices for individual shapes
	if (EntityID.HasComponent<PlaneComponent>())
	{
		PlaneComponent& PlaneComponentReference = EntityID.GetComponent<PlaneComponent>();
		GeneratePlane(PlaneComponentReference);
	}
	else if (EntityID.HasComponent<PyramidComponent>())
	{
		PyramidComponent& PyramidComponentReference = EntityID.GetComponent<PyramidComponent>();
		GeneratePyramid(PyramidComponentReference);
	}
}

void EntityRendererSystem::GeneratePlane(PlaneComponent& PlaneComponentRefernece)
{
	CMP203::Vertex Quad[4] =
	{
		{float3(-1,1,0),float3(0,0,0),float2(0,0),	float3(0,0,-1)	},
		{float3(-1,-1,0),float3(0,0,0),float2(0,1),	float3(0,0,-1)	},
		{float3(1,-1,0),float3(0,0,0),float2(1,1),	float3(0,0,-1)	},
		{float3(1,1,0),float3(0,0,0),float2(1,0),	float3(0,0,-1)	}
	};

	uint32_t quadindices[6] =
	{
		0,1,2,
		0,2,3
	};

	int VertexArraySize = sizeof(Quad) / sizeof(Quad[0]);
	for (int i = 0; i < VertexArraySize; i++)
	{
		PlaneComponentRefernece.Vertices.push_back(Quad[i]);
	}

	int IndicesArraySize = sizeof(quadindices) / sizeof(quadindices[0]);
	for (int i = 0; i < IndicesArraySize; i++)
	{
		PlaneComponentRefernece.Indicies.push_back(quadindices[i]);
	}
}

void EntityRendererSystem::GeneratePyramid(PyramidComponent& PyramidComponentReference)
{
	CMP203::Vertex PyramidVertices[11] =
	{
		{float3(-1,0,1),float3(0,0,0),float2(0,1),	float3(0,0,-1)	}, // Back Left       0
		{float3(1,0,1),float3(0,0,0),float2(0,1),	float3(0,0,-1)	}, // Back Right      1
		{float3(1,0, -1),float3(0,0,0),float2(0,1),	float3(0,0,-1)	}, // Front Right     2
		{float3(-1,0, -1),float3(0,0,0),float2(0,1),	float3(0,0,-1)	}, // Front Left  3

		{float3(-1,0,1),float3(0,0,0),float2(1,1),	float3(0,0,-1)	}, // Back Left       4  (0)
		{float3(1,0,1),float3(0,0,0),float2(1,1),	float3(0,0,-1)	}, // Back Right      5  (1)
		{float3(1,0, -1),float3(0,0,0),float2(1,1),	float3(0,0,-1)	}, // Front Right     6  (2)
		{float3(-1,0, -1),float3(0,0,0),float2(1,1),	float3(0,0,-1)	}, // Front Left  7  (3)

		{float3(0,2,0),float3(0,0,0),float2(0.5,0),	float3(0,0,-1)	}, // Top             8

		{float3(1,0, -1),float3(0,0,0),float2(1,0),	float3(0,0,-1)	}, // Front Right     9  (2) 
		{float3(-1,0, -1),float3(0,0,0),float2(0,0),	float3(0,0,-1)	}, // Front Left  10 (3)
	};

	uint32_t PyramidIndices[18] =
	{
		8,5,0,
		8,6,1,
		8,7,2,
		8,4,3,
		5,9,10,
		0,5,10
	};

	int VertexArraySize = sizeof(PyramidVertices) / sizeof(PyramidVertices[0]);
	for (int i = 0; i < VertexArraySize; i++)
	{
		PyramidComponentReference.Vertices.push_back(PyramidVertices[i]);
	}

	int IndicesArraySize = sizeof(PyramidIndices) / sizeof(PyramidIndices[0]);
	for (int i = 0; i < IndicesArraySize; i++)
	{
		PyramidComponentReference.Indicies.push_back(PyramidIndices[i]);
	}
}

void EntityRendererSystem::RenderEntities(CMP203::Renderer203& m_renderer)
{
	//std::vector<CMP203::Vertex> AllVertices;
	//std::vector<uint32_t> AllIndices;
	for (int i = 0; i < ShapeComponentReferenceWrappers.size(); i++)
	{
		ShapeComponent& ShapeComponentReference = ShapeComponentReferenceWrappers[i];
		if (ShapeComponentReference.Vertices.size() > 0 && ShapeComponentReference.Indicies.size() > 0)
		{
			TransformComponent& TransformComponentReference = TransformComponentReferenceWrappers[i];
			EntityInstanceData.World = TransformComponentReference.GetTransform();

			m_renderer.DrawVertices(ShapeComponentReference.Vertices.data(), ShapeComponentReference.Vertices.size(), ShapeComponentReference.Indicies.data(), ShapeComponentReference.Indicies.size(), &EntityInstanceData);
		}
	}
}

void EntityRendererSystem::RegisterEntities()
{
	ShapeComponentReferenceWrappers.clear();
	TransformComponentReferenceWrappers.clear();

	// Lamda used to grab each shape component attached to all entities and stores them in vector
	ECS::GetRegistry().each([&](auto entity_id) {

		Skateboard::Entity entity{ entity_id };
	if (entity.HasComponent<TransformComponent>())
	{
		if (entity.HasComponent<ShapeComponent>())
		{
			ShapeComponentReferenceWrappers.push_back(entity.GetComponent<ShapeComponent>());
			TransformComponentReferenceWrappers.push_back(entity.GetComponent<TransformComponent>());
			InitializeShapeComponent(entity);
		}
		if (entity.HasComponent<PlaneComponent>())
		{
			ShapeComponentReferenceWrappers.push_back(entity.GetComponent<PlaneComponent>());
			TransformComponentReferenceWrappers.push_back(entity.GetComponent<TransformComponent>());
			InitializeShapeComponent(entity);
		}
		if (entity.HasComponent<PyramidComponent>())
		{
			ShapeComponentReferenceWrappers.push_back(entity.GetComponent<PyramidComponent>());
			TransformComponentReferenceWrappers.push_back(entity.GetComponent<TransformComponent>());
			InitializeShapeComponent(entity);
		}
	}

	});
}

