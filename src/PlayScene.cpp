#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"
#include "Transition.h"
#include "Attack.h"
#include "MoveToLOS.h"
#include "MoveToPlayer.h"
#include "Patrol.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"
#include "Util.h"


PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
	if(EventManager::Instance().isIMGUIActive())
	{
		GUI_Function();	
	}

	drawDisplayList();

	SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{
	updateDisplayList();

	m_CheckShipLOS(m_pTarget);

	// Set conditions
	m_pHasLOSCondition->SetCondition(m_pShip->hasLOS());
	m_pLostLOSCondition->SetCondition(!m_pShip->hasLOS());
	m_pIsWithinDetectionRadiusCondition->SetCondition(
		Util::distance(m_pShip->getTransform()->position, m_pTarget->getTransform()->position) <= 300.0f
	);
	m_pIsNotWithinDetectionRadiusCondition->SetCondition(
		Util::distance(m_pShip->getTransform()->position, m_pTarget->getTransform()->position) > 300.0f
	);
	//m_pIsWithinCombatRangeCondition->SetCondition();
	//m_pIsNotWithinCombatRangeCondition->SetCondition();
	// Update StateMachine
	m_pStateMachine->Update();
}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(START_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance()->changeSceneState(END_SCENE);
	}

	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_F))
	{

	}
	
	if(EventManager::Instance().isKeyDown(SDL_SCANCODE_M))
	{
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_G))
	{
	}
	
}

void PlayScene::start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";

	// add the ship to the scene as a start point
	m_pShip = new Ship();
	m_pShip->getTransform()->position = glm::vec2(200.0f, 300.0f);
	addChild(m_pShip, 2);

	// add the Obstacle to the scene as a start point
	m_pObstacle1 = new Obstacle();
	m_pObstacle1->getTransform()->position = glm::vec2(400.0f, 300.0f);
	addChild(m_pObstacle1);

	// add the Obstacle to the scene as a start point
	m_pObstacle2 = new Obstacle();
	m_pObstacle2->getTransform()->position = glm::vec2(400.0f, 100.0f);
	addChild(m_pObstacle2);

	// add the Obstacle to the scene as a start point
	m_pObstacle3 = new Obstacle();
	m_pObstacle3->getTransform()->position = glm::vec2(600.0f, 500.0f);
	addChild(m_pObstacle3);
	
	// added the target to the scene a goal
	m_pTarget = new Target();
	m_pTarget->getTransform()->position = glm::vec2(600.0f, 300.0f);
	addChild(m_pTarget);
	
	// create a test StateMachine
	m_buildStateMachine();
}

void PlayScene::GUI_Function() 
{
	//TODO: We need to deal with this
	auto offset = glm::vec2(Config::TILE_SIZE * 0.5f, Config::TILE_SIZE * 0.5f);
	
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();
	
	ImGui::Begin("GAME3001 - Lab 8", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

	// allow ship rotation
	static int angle;
	if(ImGui::SliderInt("Ship Direction", &angle, -360, 360))
	{
		m_pShip->setCurrentHeading(angle);
	}
	
	ImGui::Separator();

	static int shipPosition[] = { m_pShip->getTransform()->position.x, m_pShip->getTransform()->position.y };
	if (ImGui::SliderInt2("Ship Position", shipPosition, 0, 800))
	{
		m_pShip->getTransform()->position.x = shipPosition[0];
		m_pShip->getTransform()->position.y = shipPosition[1];
	}
	
	static int targetPosition[] = { m_pTarget->getTransform()->position.x, m_pTarget->getTransform()->position.y };
	if(ImGui::SliderInt2("Target Position", targetPosition, 0, 800))
	{
		m_pTarget->getTransform()->position.x = targetPosition[0];
		m_pTarget->getTransform()->position.y = targetPosition[1];
	}
	
	ImGui::Separator();
	
	if (ImGui::Button("Start"))
	{

	}

	ImGui::SameLine();
	
	if (ImGui::Button("Reset"))
	{
		// reset everything back to initial values
		
	}

	ImGui::Separator();

	ImGui::End();

	// Don't Remove this
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}

void PlayScene::m_CheckShipLOS(DisplayObject* target_object)
{
	// Start without LOS
	m_pShip->setHasLOS(false);
	// if ship to target distance is less than or equal to LOS Distance
	auto ShipToTargetDistance = Util::distance(m_pShip->getTransform()->position, target_object->getTransform()->position);
	if (ShipToTargetDistance <= m_pShip->getLOSDistance())
	{
		std::vector<DisplayObject*> contactList;
		for (auto object : getDisplayList())
		{
			// check if object is farther than than the target
			auto ShipToObjectDistance = Util::distance(m_pShip->getTransform()->position, object->getTransform()->position);

			if (ShipToObjectDistance <= ShipToTargetDistance)
			{
				if ((object->getType() != m_pShip->getType()) && (object->getType() != target_object->getType()))
				{
					contactList.push_back(object);
				}
			}
		}
		contactList.push_back(target_object); // add the target to the end of the list
		auto hasLOS = CollisionManager::LOSCheck(m_pShip->getTransform()->position,
			m_pShip->getTransform()->position + m_pShip->getCurrentDirection() * m_pShip->getLOSDistance(), contactList, target_object);

		m_pShip->setHasLOS(hasLOS);
	}
}

void PlayScene::m_buildStateMachine()
{
	// Define States
	State* patrolState = new State();
	State* moveToPlayerState = new State();
	State* moveToLOSState = new State();
	State* attackState = new State();

	// Define Conditions
	m_pHasLOSCondition = new Condition();
	m_pLostLOSCondition = new Condition();
	m_pIsWithinDetectionRadiusCondition = new Condition();
	m_pIsNotWithinDetectionRadiusCondition = new Condition();
	m_pIsWithinCombatRangeCondition = new Condition();
	//m_pIsNotWithinCombatRangeCondition = new Condition();

	// Define Transitions
	Transition* moveToPlayerTransition = new Transition(m_pHasLOSCondition, moveToPlayerState);
	Transition* moveToLOSTransition = new Transition(m_pIsWithinDetectionRadiusCondition, moveToLOSState);
	Transition* attackTransition = new Transition(m_pIsWithinCombatRangeCondition, attackState);
	// Alex's added Transitions
	Transition* LOSToPatrolTransition = new Transition(m_pIsNotWithinDetectionRadiusCondition, patrolState);
	Transition* moveToPlayerToLOSTransition = new Transition(m_pLostLOSCondition, moveToLOSState);

	// Define Actions
	Patrol* patrolAction = new Patrol();
	MoveToLOS* moveToLOSAction = new MoveToLOS();
	MoveToPlayer* moveToPlayerAction = new MoveToPlayer();
	Attack* attackAction = new Attack();

	// Setup Patrol State
	patrolState->addTransition(moveToPlayerTransition);
	patrolState->addTransition(moveToLOSTransition);
	patrolState->setAction(patrolAction);

	// Setup MoveToPlayer State
	moveToPlayerState->addTransition(attackTransition);
	moveToPlayerState->addTransition(moveToPlayerToLOSTransition); 
	moveToPlayerState->setAction(moveToPlayerAction);

	// Setup MoveTOLOS State
	moveToLOSState->addTransition(moveToPlayerTransition);
	moveToLOSState->addTransition(LOSToPatrolTransition);
	moveToLOSState->setAction(moveToLOSAction);

	// Setup Attack State
	attackState->addTransition(moveToPlayerTransition); // Missing Condition
	attackState->addTransition(moveToLOSTransition); // Missing Condition
	attackState->setAction(attackAction);

	m_pStateMachine = new StateMachine();
	m_pStateMachine->setCurrentState(patrolState);
}
