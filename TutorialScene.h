#pragma once
#include "Skateboard/Scene/Scene.h"
#include "Skateboard/ComponentSystems/Systems/BaseSystem.h"

#include "CMP203/Renderer203.h"

class EntityRendererSystem;
class EntityPhysicsSystem;
class EntitySpawningSystem;

class TutorialScene : public Skateboard::Scene {
public:
	explicit TutorialScene(const std::string& name);

	TutorialScene() = delete;

	~TutorialScene();

	virtual void OnHandleInput(Skateboard::TimeManager* time) override;
	virtual void OnUpdate(Skateboard::TimeManager* time) override;
	virtual void OnRender() override;

	virtual void OnImGuiRender() override;

private:
	void UpdateFPS(float DeltaTime);
	void UpdateBoundingBox();
	void UpdateCamera();
	void UpdateBoidsRules();

	void RestartSimulation();
	void StopSimulation();

	CMP203::Renderer203 m_Renderer;

	EntityRendererSystem* RendererSystem;
	EntityPhysicsSystem* PhysicsSystem;
	EntitySpawningSystem* SpawningSystem;

	float m_FPSClock;
	float m_FPS;
	float m_FrameTime;

	float OverallCalculationTime{0};

	int NumberOfBoids{50};

	float XBoundingBoxBounds[2]{ -5, 5 };
	float YBoundingBoxBounds[2]{ -5, 5 };
	float ZBoundingBoxBounds[2]{ -5, 5 };

	// ImGui bools, these exist to avoid race conditions
	bool bShouldRestartSimulation = false;
	bool bShouldStopSimulation = true;

	float ModifiableCameraDegrees {90.0f};

	float CameraTranslate[3]{ 0, 0, 10 };
	float CameraRotation[3]{ 0, 0, 0 };

	float AlignmentDistance[2];
	float SeparationDistance[2];
};