#include "IPhysicsScene.h"
#include "PhysicsScene.h"

using namespace KG::Physics;

void KG::Physics::IPhysicsScene::SetDsec(const PhysicsDesc& desc)
{
	this->desc = desc;
}

// EXTERNC DLL KG::Physics::IPhysicsScene* GetPhysicsScene()
// // DLL KG::Physics::PhysicsScene* KG::Physics::GetPhysicsScene()
// {
// 	// return nullptr;
// 	return new KG::Physics::PhysicsScene();
// }

DLL KG::Physics::IPhysicsScene* KG::Physics::GetPhysicsScene()
{
	return new KG::Physics::PhysicsScene();
}
