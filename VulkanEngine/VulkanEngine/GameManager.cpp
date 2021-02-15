#include "pch.h"
#include "GameManager.h"

#include "DebugManager.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "Camera.h"

#define MshMngr MeshManager::GetInstance()

#pragma region Singleton

GameManager* GameManager::instance = nullptr;

GameManager* GameManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new GameManager();
    }

    return instance;
}

#pragma endregion

#pragma region Accessors

std::vector<std::shared_ptr<Light>> GameManager::GetLights()
{
    return lights;
}

std::shared_ptr<GameObject> GameManager::GetObjectByName(std::string name)
{
    for (int i = 0; i < gameObjects.size(); i++) {
        if (gameObjects[i]->GetName().compare(name) == 0) {
            return gameObjects[i];
        }
    }

    std::cout << "Could not find object with name: " << name << std::endl;
    return nullptr;
}

#pragma endregion

#pragma region Game Loop

void GameManager::Init()
{
    //Setup Lights
    lights.push_back(std::make_shared<Light>(glm::vec3(1.5f, 1.1f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 5.0f));
    lights.push_back(std::make_shared<Light>(glm::vec3(0.0f, 2.0f, -1.5f), glm::vec3(1.0f, 0.988f, 0.769f), 3.0f, 4.0f));

    //Game Objects are created here. First we must create an object and add it
	//to the gameObjects vector. A shared pointer to a gameObject is added, and 
	//the shared pointer has an instance and a mesh that we specify. the mesh types 
	//are: Plane, Cube, Sphere, Model, SkyBox, WireCube, WireSphere and Line 

	//We will add a cube and a skybox for now
    gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Cube]));
    gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Skybox]));
	//gameObjects.push_back(std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::WireCube]));
    
    //Setup Cube
	//Next we will set up the data for our cube. First we set a transform for
	//the cube, then we will use that newly created transform to set the position,
	//orientation and scale. We we also set a static physics object for the cube
    gameObjects[0]->SetTransform(std::make_shared<Transform>(glm::vec3()));
    gameObjects[0]->GetTransform()->SetPosition(glm::vec3(0, 0, 0));
    gameObjects[0]->GetTransform()->SetOrientation(glm::vec3(0.0f, 0.0f, 0.0f));
	gameObjects[0]->GetTransform()->SetScale(glm::vec3(0.5f, 0.5f, 0.1f));
    gameObjects[0]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[0]->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));
    gameObjects[0]->SetName("Cube");

	// setup skybox to enhnace the scene
	gameObjects[1]->SetTransform(std::make_shared<Transform>(glm::vec3(0)));
	gameObjects[1]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[1]->GetTransform(), PhysicsLayers::Trigger, 1.0f, false, false));
	gameObjects[1]->SetName("Skybox");

	//NOT WORKINGING 
	//TODO: Fix vertex buffers for debug meshes
	//gameObjects[2]->SetTransform(std::make_shared<Transform>(glm::vec3()));
	//gameObjects[2]->GetTransform()->SetPosition(glm::vec3(0, 0, 0));
	//gameObjects[2]->GetTransform()->SetOrientation(glm::vec3(0.0f, 0.0f, 0.0f));
	//gameObjects[2]->GetTransform()->SetScale(glm::vec3(0.5f, 0.5f, 0.1f));
	//gameObjects[2]->SetPhysicsObject(std::make_shared<PhysicsObject>(gameObjects[2]->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));
	//gameObjects[2]->SetName("WireCube");

    for (size_t i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->Spawn();
    }
}

void GameManager::Update()
{
    // MshMngr->ClearRenderList();
    // MeshManager::GetInstance()->DrawWireCube(glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    //Rotate Camera
    //  Hold to unlock camera 
	
	
	//We will use these strings to debug
	//the copy strings will be set at the beginnging of the function
	//to check against any changes in position
	objectLocationXCopy = objectLocationX;
	objectLocationYCopy = objectLocationY;

	
	//if the cube's x position is greater than 0.0f, then the cube is on the
	//right side of the screen.
	if (gameObjects[0]->GetTransform()->GetPosition().x > 0.0f) {
		objectLocationX = "Right";
	}
	//if the cube's x position is at 0.0f, the the cube is perfectly centered.
	else if (gameObjects[0]->GetTransform()->GetPosition().x == 0.0f) {
		objectLocationX = "Centered";
	}
	//if the cube is not right or center, assume it is on the left side
	//of the screen
	else {
		objectLocationX = "Left";
	}

	//same idea as x axis checks, if y position is greater than 0.0f,
	//the cube is up on the screen
	if (gameObjects[0]->GetTransform()->GetPosition().y > 0.0f) {
		objectLocationY = "Up";
	}
	//centered
	else if(gameObjects[0]->GetTransform()->GetPosition().y == 0.0f){
		objectLocationY = "Centered";
	}
	//else, the cube is down on the screen
	else {
		objectLocationY = "Down";
	}

	//if the location of the block has changed from the start of the fucntion, log out the new psoition it moved tp
	if (objectLocationX != objectLocationXCopy) {
		std::cout << "Cube is: " << objectLocationX << " on the x axis" << std::endl;
	}
	if (objectLocationY != objectLocationYCopy) {
		std::cout << "Cube is: " << objectLocationY << " on the y axis" << std::endl;
	}

	//these following 4 statements keep the cube locked to not be 
	//able to go off of the screen for now
	if (gameObjects[0]->GetTransform()->GetPosition().y > 2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(gameObjects[0]->GetTransform()->GetPosition().x, 2.0f, 0.0f));
	}
	if (gameObjects[0]->GetTransform()->GetPosition().y < -2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(gameObjects[0]->GetTransform()->GetPosition().x, -2.0f, 0.0f));
	}
	
	if (gameObjects[0]->GetTransform()->GetPosition().x > 2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(2.0f, gameObjects[0]->GetTransform()->GetPosition().y, 0.0f));
	}
	if (gameObjects[0]->GetTransform()->GetPosition().x < -2.0f) {
		gameObjects[0]->GetTransform()->SetPosition(glm::vec3(-2.0f, gameObjects[0]->GetTransform()->GetPosition().y, 0.0f));
	}

	
	
	//hold and drag the screen to move the camera
    if (InputManager::GetInstance()->GetKey(Controls::RightClick)) {
        lockCamera = !lockCamera; 
	}
	else {
		lockCamera = true;
	}
    if (InputManager::GetInstance()->GetKey(Controls::LeftClick)) {
        lockCamera = !lockCamera;
	}
	else {
		lockCamera = true;
	}

    //  Rotate camera if not locked
    if (!lockCamera) {
        glm::vec2 deltaMouse = InputManager::GetInstance()->GetDeltaMouse();
        if (deltaMouse.x != 0 || deltaMouse.y != 0) {
            deltaMouse = glm::normalize(deltaMouse) / 4.0f;
        }

        glm::quat orientation = Camera::GetMainCamera()->GetTransform()->GetOrientation();
        glm::vec3 rotation = orientation * glm::vec3(deltaMouse.y, 0.0f, 0.0f) + glm::vec3(0.0f, -deltaMouse.x, 0.0f);

        Camera::GetMainCamera()->GetTransform()->Rotate(rotation);
    }

    //Move Cube
    glm::vec3 moveDirection = glm::vec3(0.0f, 0.0f, 0.0f);

	//switch to up and down for now
	//Using wasd will move the cube in the corresponding directions
	//w-up, s-down, a-left, d-right
    if (InputManager::GetInstance()->GetKey(Controls::Forward)) {
        moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Back)) {
        moveDirection += glm::vec3(0.0f, -1.0f, 0.0f);
    }
	//Q and E inputs, disabled for now
    if (InputManager::GetInstance()->GetKey(Controls::Up)) {
        //moveDirection += glm::vec3(0.0f, 1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Down)) {
        //moveDirection += glm::vec3(0.0f, -1.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Left)) {
        moveDirection += glm::vec3(-1.0f, 0.0f, 0.0f);
    }
    if (InputManager::GetInstance()->GetKey(Controls::Right)) {
        moveDirection += glm::vec3(1.0f, 0.0f, 0.0f);
    }

    if (moveDirection.x != 0 || moveDirection.y != 0 || moveDirection.z != 0) {
        moveDirection = glm::normalize(moveDirection);
    }

    //Camera::GetMainCamera()->GetTransform()->Translate(moveDirection * cameraSpeed * Time::GetDeltaTime(), true);

	//Move cube
	gameObjects[0]->GetTransform()->Translate(moveDirection * cameraSpeed * Time::GetDeltaTime(), true);

    //Update Lights
    float scaledTime = Time::GetTotalTime() / 2.5f;
    lights[0]->position = glm::vec3(0.0f, 1.1f, 0.0f) + glm::vec3(cos(scaledTime), 0.0f, sin(scaledTime)) * 1.5f;
    
    //Update Game Objects
    for (size_t i = 0; i < gameObjects.size(); i++) {
        gameObjects[i]->Update();
    }

    if (InputManager::GetInstance()->GetKeyPressed(Controls::Jump)) {
        gameObjects[2]->GetPhysicsObject()->ApplyForce(glm::vec3(0.0f, 5000.0f, 0.0f));

        //Spawn Object Sample Code:
        /*
        std::shared_ptr<GameObject> newObject = std::make_shared<GameObject>(EntityManager::GetInstance()->GetMeshes()[MeshTypes::Sphere]);
        gameObjects.push_back(newObject);

        newObject->SetTransform(std::make_shared<Transform>(glm::vec3(0.0f, 2.5f, 0.0f)));
        newObject->SetPhysicsObject(std::make_shared<PhysicsObject>(newObject->GetTransform(), PhysicsLayers::Static, 1.0f, false, true));

        newObject->Spawn();
        */
    }
}

#pragma endregion