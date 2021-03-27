#include "IPhysicsScene.h"
#include "PhysicsScene.h"

void KG::Physics::IPhysicsScene::SetDsec(const PhysicsDesc& desc)
{
	this->desc = desc;
}

// DLL KG::Physics::IPhysicsScene* KG::Physics::GetPhysiceScene()
// {
// 	return new PhysicsScene();
// }
