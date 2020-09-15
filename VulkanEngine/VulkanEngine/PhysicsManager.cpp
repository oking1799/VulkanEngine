#include "pch.h"
#include "PhysicsManager.h"

#pragma region Singleton

PhysicsManager* PhysicsManager::instance = nullptr;

PhysicsManager* PhysicsManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new PhysicsManager();
        instance->physicsObjects.resize(PhysicsLayers::PhysicsLayerCount);

        for (size_t i = 0; i < instance->physicsObjects.size(); i++) {
            instance->physicsObjects[i] = std::vector<std::shared_ptr<PhysicsObject>>();
        }
    }

    return instance;
}

#pragma endregion

#pragma region Accessors

float PhysicsManager::GetGravity()
{
    return gravity;
}

void PhysicsManager::SetGravity(float value)
{
    gravity = value;
}

glm::vec3 PhysicsManager::GetGravityDirection()
{
    return gravityDirection;
}

void PhysicsManager::SetGravityDirection(glm::vec3 value)
{
    gravityDirection = value;
}

void PhysicsManager::AddPhysicsObject(std::shared_ptr<PhysicsObject> object)
{
    physicsObjects[object->GetPhysicsLayer()].push_back(object);
}

#pragma endregion

#pragma region Update

void PhysicsManager::Update()
{
    //Update dynamic objects
    for (size_t i = 0; i < physicsObjects[PhysicsLayers::Dynamic].size(); i++) {
        physicsObjects[PhysicsLayers::Dynamic][i]->Update();
    }

    for (size_t i = 0; i < physicsObjects[PhysicsLayers::Static].size(); i++) {
        physicsObjects[PhysicsLayers::Static][i]->Update();
    }

    //Check for collisions
    DetectCollisions();
}

#pragma endregion

#pragma region Collision Detection

void PhysicsManager::DetectCollisions()
{
    //Check Dynamic objects against all objects
    for (size_t i = 0; i < physicsObjects[PhysicsLayers::Dynamic].size(); i++) {
        for (size_t j = i + 1; j < physicsObjects[PhysicsLayers::Dynamic].size(); j++) {
            if (physicsObjects[PhysicsLayers::Dynamic][i]->GetCollider()->CheckCollision(physicsObjects[PhysicsLayers::Dynamic][j]->GetCollider().get())) {
                ResolveCollision(physicsObjects[PhysicsLayers::Dynamic][i], physicsObjects[PhysicsLayers::Dynamic][j]);
            }
        }

        for (size_t j = 0; j < physicsObjects[PhysicsLayers::Static].size(); j++) {
            if (physicsObjects[PhysicsLayers::Dynamic][i]->GetCollider()->CheckCollision(physicsObjects[PhysicsLayers::Static][j]->GetCollider().get())) {
                ResolveCollision(physicsObjects[PhysicsLayers::Dynamic][i], physicsObjects[PhysicsLayers::Static][j]);
            }
        }

        for (size_t j = 0; j < physicsObjects[PhysicsLayers::Trigger].size(); j++) {
            if (physicsObjects[PhysicsLayers::Dynamic][i]->GetCollider()->CheckCollision(physicsObjects[PhysicsLayers::Trigger][j]->GetCollider().get())) {
                //TODO: Call the trigger's on collide function
            }
        }
    }

    //Check Static objects against triggers
    for (size_t i = 0; i < physicsObjects[PhysicsLayers::Static].size(); i++) {
        for (size_t j = 0; j < physicsObjects[PhysicsLayers::Trigger].size(); j++) {
            if (physicsObjects[PhysicsLayers::Static][i]->GetCollider()->CheckCollision(physicsObjects[PhysicsLayers::Trigger][j]->GetCollider().get())) {
                //TODO: Call the trigger's on collide function
            }
        }
    }
}

#pragma endregion

#pragma region Collision Resolution

void PhysicsManager::ResolveCollision(std::shared_ptr<PhysicsObject> physicsObject1, std::shared_ptr<PhysicsObject> physicsObject2)
{
    //TODO: Change this to use colliders rather than a radius check
    //Double Dynamic
    if (physicsObject1->GetPhysicsLayer() == PhysicsLayers::Dynamic && physicsObject2->GetPhysicsLayer() == PhysicsLayers::Dynamic) {
        glm::vec3 center = (physicsObject1->GetTransform()->GetPosition() + physicsObject2->GetTransform()->GetPosition()) / 2.0f;
        glm::vec3 collisionDirection = glm::normalize(physicsObject2->GetTransform()->GetPosition() - physicsObject1->GetTransform()->GetPosition());

        physicsObject1->GetTransform()->SetPosition(center + collisionDirection * -0.5f);
        physicsObject2->GetTransform()->SetPosition(center + collisionDirection * 0.5f);
    }
    else { //One Dynamic and One Static
        //Figure out which object is static and which is dynamic
        std::shared_ptr<PhysicsObject> staticObject;
        std::shared_ptr<PhysicsObject> dynamicObject;

        if (physicsObject1->GetPhysicsLayer() == PhysicsLayers::Dynamic) {
            dynamicObject = physicsObject1;
            staticObject = physicsObject2;
        }
        else {
            dynamicObject = physicsObject2;
            staticObject = physicsObject1;
        }

        //TODO: remove this once actual collisions are working
        if (dynamicObject->GetVelocity().y < 0) {
            dynamicObject->SetVelocity(glm::vec3(dynamicObject->GetVelocity().x, dynamicObject->GetVelocity().y * -0.8f, dynamicObject->GetVelocity().z));
        }
    }

    //TODO: Call both object's on collision methods
}

#pragma endregion