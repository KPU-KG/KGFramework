#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"
#include "PhysicsComponent.h"

using namespace physx;

bool KG::Physics::IPhysicsScene::Initialize() {
	const char* strTransport = "127.0.0.1";

	allocator = new PxDefaultAllocator();
	errorCallback = new PxDefaultErrorCallback();

	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *allocator, *errorCallback);

	if (!foundation)
		return false;
	// PVD
	pvd = PxCreatePvd(*foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(strTransport, 5425, 10);
	bool pvdConnectionResult = pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
	if (!pvdConnectionResult) {
	
	}

	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true, pvd);

	if (!physics)
		return false;

	cpuDispatcher = PxDefaultCpuDispatcherCreate(1);

	return true;
}

bool KG::Physics::IPhysicsScene::CreateScene() {
	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.cpuDispatcher = cpuDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	scene = physics->createScene(sceneDesc);
	if (!scene)
		return false;

	// PVD
	PxPvdSceneClient* pvdClient = scene->getScenePvdClient();
	if (pvdClient) {
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	return true;
}

bool KG::Physics::IPhysicsScene::Advance(float timeElapsed) {
	accumulator += timeElapsed;
	while (accumulator >= stepSize) {
		accumulator -= stepSize;
		scene->simulate(stepSize);
	}
	scene->fetchResults();
	return true;
}

void KG::Physics::IPhysicsScene::AddActor(KG::Component::DynamicRigidComponent* rigid)
{
	// 실제로 물리적으로 작용이 일어나는 것은 박스로 처리
	// 그 외에 충돌 판정은 해야 하나 물리적 작용은 안해도 되는 것 (총알에 맞는 판정 등)은 KINETIC 플래그 설정
	// 그러면 콜리전 박스를 2개로 나눠서 관리 / kinetic, dynamic
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);		// Basic Setting : 나중에 필요하면 추가 ( 정적 마찰 계수, 동적 마찰 계수, 반탄 계수)
	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), PxBoxGeometry(cb.width, cb.height, cb.depth), *pMaterial, 1);

#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
#endif
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, false);		// DynamicRigidComponent에 플래그 추가
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, false);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, false);

	scene->addActor(*actor);
	rigid->SetActor(actor);

	// rigid->
}

// void KG::Physics::IPhysicsScene::AddActor(DirectX::XMFLOAT3 position, float width, float height, float depth)
// {
// 	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);
// 	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(position.x, position.y, position.z), PxBoxGeometry(width, height, depth), *pMaterial, 1);
// 	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
// 
// 	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, false);
// 	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, false);
// 	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, false);
// 
// 	scene->addActor(*actor);
// 	rigid.push_back(actor);
// }

void KG::Physics::IPhysicsScene::AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth)
{
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);
	PxRigidStatic* actor = PxCreateStatic(*physics, PxTransform(position.x, position.y, position.z), PxBoxGeometry(width, height, depth), *pMaterial);
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
	scene->addActor(*actor);
}

void KG::Physics::IPhysicsScene::AddFloor(float height)
{
	PxMaterial* planeMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* plane = PxCreatePlane(*physics, PxPlane(PxVec3(0, 1, 0), -height), *planeMaterial);
	scene->addActor(*plane);
}

void KG::Physics::IPhysicsScene::Move(DirectX::XMFLOAT3 vector)
{
	rigid[0]->setLinearVelocity(PxVec3(vector.x, vector.y, vector.z)*1.5);
	rigid[0]->setLinearDamping(1);
	// rigid[0]->addForce(PxVec3(vector.x, vector.y, vector.z) * rigid[0]->getMass() * 5);
}

DirectX::XMFLOAT3 KG::Physics::IPhysicsScene::GetPosition()
{
	PxVec3 pos = rigid[0]->getGlobalPose().p;
	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
	// TODO: 여기에 return 문을 삽입합니다.
}

void KG::Physics::IPhysicsScene::SetRotation(DirectX::XMFLOAT4 quat)
{
	rigid[0]->getGlobalPose().q = PxQuat(quat.x, quat.y, quat.z, quat.w);
	// rigid[0]->
}
