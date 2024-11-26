// includes all the basic structures and most standard library containers used throughout Skateboard
#include "sktbdpch.h"

#include "TutorialScene.h"
#include "Skateboard/Camera/Camera.h"
#include "Skateboard/Time/TimeManager.h"
#include "Skateboard/Assets/AssetManager.h"
#include "Skateboard/Input.h"

//N
#include "Skateboard/Assets/AssetManager.h"
#include "CMP203/Renderer203.h"
#include "AdditionalComponents.h"
#include "ECS Systems/EntityRendererSystem.h"
#include "ECS Systems/EntityPhysicsSystem.h"
#include "ECS Systems/EntitySpawningSystem.h"

#include <chrono>

TutorialScene::TutorialScene(const std::string& name): 
	Scene(name), m_FPSClock(0.0f), m_FPS(0.0f), m_FrameTime(0.0f)
{
	size_t NewDynamicDataBufferSize = 1024 * 1024;
	size_t NewInstanceDataBufferSize = 64 * 1024;
	m_Renderer.Init(NewDynamicDataBufferSize, NewInstanceDataBufferSize);

	AssetManager::LoadTexture(L"assets/fsjal", "fsjal");
	AssetManager::LoadTexture(L"CMP203/crate", "crate");
	AssetManager::LoadTexture(L"CMP203/stone", "stone");

	SpawningSystem = new EntitySpawningSystem();
	RendererSystem = new EntityRendererSystem();
	PhysicsSystem = new EntityPhysicsSystem();

	AlignmentDistance[0] = PhysicsSystem->MinimumAlignmentDistance;
	AlignmentDistance[1] = PhysicsSystem->MaximumAlignmentDistance;
	SeparationDistance[0] = PhysicsSystem->MinimumSeparationDistance;
	SeparationDistance[1] = PhysicsSystem->MaximumSeparationDistance;
}

TutorialScene::~TutorialScene()
{
	// Avoid Memory Leak
	delete RendererSystem;
	RendererSystem = nullptr;

	delete PhysicsSystem;
	PhysicsSystem = nullptr;

	delete SpawningSystem;
	SpawningSystem = nullptr;
}

void TutorialScene::OnHandleInput(TimeManager* time)
{
	Scene::OnHandleInput(time);
}

void TutorialScene::OnUpdate(TimeManager* time)
{
	Scene::OnUpdate(time);

	UpdateFPS(time->DeltaTime());
	UpdateBoundingBox();
	UpdateCamera();
	UpdateBoidsRules();

	if (bShouldRestartSimulation)
	{
		RestartSimulation();
		bShouldRestartSimulation = false;
	}

	auto start = std::chrono::high_resolution_clock::now();

	if (!bShouldStopSimulation)
	{
		PhysicsSystem->UpdateEntities(time->DeltaTime());
	}
	else
	{
		StopSimulation();
	}

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = duration_cast<std::chrono::microseconds>(stop - start);
	OverallCalculationTime = duration.count() / 1000;
}

void TutorialScene::OnRender()
{
	Scene::OnRender();

	m_Renderer.Begin();

	if (!bShouldStopSimulation)
	{
		RendererSystem->RenderEntities(m_Renderer);
	}
	
	m_Renderer.End();
}

void TutorialScene::OnImGuiRender()
{
	ImGui::Begin("ImGui");//creates new window

	ImGui::Text("FPS: %f", m_FPS);
	ImGui::Text("FrameTime: %f", m_FrameTime);
	ImGui::Text("Current Boids: %i", SpawningSystem->AllEntities.size());
	ImGui::Text("Full Calculation Time(ms): %f", OverallCalculationTime);

	ImGui::Separator();

	if(ImGui::RadioButton("Wireframe Toggle", m_Renderer.GetPipelineFlags() & CMP203::PipelineFlags::WIREFRAME))
	{
		if (m_Renderer.GetPipelineFlags() & CMP203::PipelineFlags::WIREFRAME)
			m_Renderer.UnsetPipelineFlags(CMP203::PipelineFlags::WIREFRAME);
		else
			m_Renderer.SetPipelineFlags(CMP203::PipelineFlags::WIREFRAME);
	}

	ImGui::SameLine();

	if (ImGui::RadioButton("Depth Test Toggle", m_Renderer.GetPipelineFlags() & CMP203::PipelineFlags::DEPTH_TEST))
	{
		if (m_Renderer.GetPipelineFlags() & CMP203::PipelineFlags::DEPTH_TEST)
			m_Renderer.UnsetPipelineFlags(CMP203::PipelineFlags::DEPTH_TEST);
		else
			m_Renderer.SetPipelineFlags(CMP203::PipelineFlags::DEPTH_TEST);
	}
	
	ImGui::SeparatorText("Simulation Settings");

	if(ImGui::Button("Start Simulation"))
	{
		if (NumberOfBoids > 0)
		{
			bShouldStopSimulation = false;
			bShouldRestartSimulation = true;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop Simulation"))
	{
		bShouldStopSimulation = true;
	}

	ImGui::InputInt("Boids", (int*)&NumberOfBoids, 1, 100);

	ImGui::SeparatorText("Boids Settings");

	ImGui::Text("Separation");
	ImGui::SliderFloat("Weight S", (float*)&PhysicsSystem->SeparationDistanceWeight, 0, 25);
	ImGui::SliderFloat2("Min/Max Dist S", (float*)&SeparationDistance, 0, 25);
	ImGui::Text("Alignment");
	ImGui::SliderFloat("Weight A", (float*)&PhysicsSystem->AlignmentDistanceWeight, 0, 25);
	ImGui::SliderFloat2("Min/Max Dist A", (float*)&AlignmentDistance, 0, 25);
	ImGui::Text("Cohesion");
	ImGui::SliderFloat("Weight C", (float*)&PhysicsSystem->CohesionDistanceWeight, 0, 25);
	ImGui::SliderFloat("Max Distance C", (float*)&PhysicsSystem->MaximumCohesionDistance, 0, 25);

	if (ImGui::Button("Reset"))
	{
		PhysicsSystem->SeparationDistanceWeight = 4.0f;
		PhysicsSystem->AlignmentDistanceWeight = 5.0f;
		PhysicsSystem->CohesionDistanceWeight = 1.0f;
		PhysicsSystem->MaximumCohesionDistance = 10.0f;
	}
	
	ImGui::SeparatorText("Bounding Box Limitations");

	ImGui::Text("X Coordinate Limits");
	ImGui::SliderFloat2("X Min/Max", (float*)&XBoundingBoxBounds, -100, 100);

	ImGui::Text("Y Coordinate Limits");
	ImGui::SliderFloat2("Y Min/Max", (float*)&YBoundingBoxBounds, -100, 100);

	ImGui::Text("Z Coordinate Limits");
	ImGui::SliderFloat2("Z Min/Max", (float*)&ZBoundingBoxBounds, -100, 100);

	ImGui::SeparatorText("Camera Controls");
	ImGui::SliderFloat("FOV", (float*)&ModifiableCameraDegrees, 0.01f, 180.0f);
	ImGui::SliderFloat3("Translate", (float*)&CameraTranslate, -100.0f, 100.0f);
	ImGui::SliderFloat3("Rotation", (float*)&CameraRotation, -359.9f, 359.9f);

	ImGui::SeparatorText("Saved Perspectives");
	if (ImGui::Button("Preset1"))
	{
		CameraTranslate[0] = 0;
		CameraTranslate[1] = -43.750;
		CameraTranslate[2] = 33.529;

		CameraRotation[0] = -45;
		CameraRotation[1] = 0;
		CameraRotation[2] = 0;

		XBoundingBoxBounds[0] = -15;
		XBoundingBoxBounds[1] = 15;

		YBoundingBoxBounds[0] = -15;
		YBoundingBoxBounds[1] = 15;

		ZBoundingBoxBounds[0] = -15;
		ZBoundingBoxBounds[1] = 15;
	}

	ImGui::SameLine();
	if (ImGui::Button("Rules1"))
	{
		PhysicsSystem->SeparationDistanceWeight = 25;
		SeparationDistance[1] = 3;
	}

	if (ImGui::Button("Original"))
	{
		CameraTranslate[0] = 0;
		CameraTranslate[1] = 0;
		CameraTranslate[2] = 10;

		CameraRotation[0] = 0;
		CameraRotation[1] = 0;
		CameraRotation[2] = 0;

		XBoundingBoxBounds[0] = -5;
		XBoundingBoxBounds[1] = 5;

		YBoundingBoxBounds[0] = -5;
		YBoundingBoxBounds[1] = 5;

		ZBoundingBoxBounds[0] = -5;
		ZBoundingBoxBounds[1] = 5;
	}

	ImGui::End();//ends that window
}

void TutorialScene::UpdateFPS(float DeltaTime)
{
	m_FPSClock += DeltaTime;
	if (m_FPSClock > 1.f)
	{
		m_FPSClock = 0.f;
		m_FPS = 1.f / DeltaTime;
		m_FrameTime = DeltaTime * 1000.f;
	}
}

void TutorialScene::UpdateBoundingBox()
{
	float2 BoxBoundsX, BoxBoundsY, BoxBoundsZ;
	PhysicsSystem->GetBoundingBoxContainerSize(BoxBoundsX, BoxBoundsY, BoxBoundsZ);

	float2 NewX{ XBoundingBoxBounds[0],
				 XBoundingBoxBounds[1] };

	float2 NewY{ YBoundingBoxBounds[0],
				 YBoundingBoxBounds[1] };

	float2 NewZ{ ZBoundingBoxBounds[0],
				 ZBoundingBoxBounds[1] };

	if (BoxBoundsX != NewX || BoxBoundsY != NewY || BoxBoundsZ != NewZ);
	{
		PhysicsSystem->SetBoundingBoxContainerSize(NewX, NewY, NewZ);
	}
}

void TutorialScene::UpdateCamera()
{
	float aspect = GraphicsContext::Context->GetClientAspectRatio();
	auto ProjectionMatrix = glm::perspectiveLH(glm::radians(ModifiableCameraDegrees), aspect, 0.01f, 100.f) * glm::eulerAngleXYZ(glm::radians(CameraRotation[0]), glm::radians(CameraRotation[1]), glm::radians(CameraRotation[2]));
	auto ViewMatrix = glm::lookAtLH(glm::vec3(0, 0, -10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)) + glm::translate(glm::vec3(CameraTranslate[0], CameraTranslate[1], CameraTranslate[2]));

	CMP203::FrameData NewData{ ViewMatrix, ProjectionMatrix };

	m_Renderer.SetFrameData(NewData);
}

void TutorialScene::UpdateBoidsRules()
{
	if (AlignmentDistance[0] != PhysicsSystem->MinimumAlignmentDistance || AlignmentDistance[1] != PhysicsSystem->MaximumAlignmentDistance)
	{
		PhysicsSystem->MinimumAlignmentDistance = AlignmentDistance[0];
		PhysicsSystem->MaximumAlignmentDistance = AlignmentDistance[1];
	}

	if (SeparationDistance[0] != PhysicsSystem->MinimumSeparationDistance || SeparationDistance[1] != PhysicsSystem->MaximumSeparationDistance)
	{
		PhysicsSystem->MinimumSeparationDistance = SeparationDistance[0];
		PhysicsSystem->MaximumSeparationDistance = SeparationDistance[1];
	}
}

void TutorialScene::RestartSimulation()
{
	if (SpawningSystem->AllEntities.size() > 0)
	{
		SpawningSystem->RemoveAllEntities();
	}

	SpawningSystem->AddEntities(NumberOfBoids);

	PhysicsSystem->RegisterEntities();
	PhysicsSystem->RandomizeEntityDirections();

	RendererSystem->RegisterEntities();
}

void TutorialScene::StopSimulation()
{
	if (SpawningSystem->AllEntities.size() > 0)
	{
		SpawningSystem->RemoveAllEntities();
	}
}
