#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"

using namespace physx;

bool KG::Physics::PhysicsScene::Initialize() {
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

bool KG::Physics::PhysicsScene::CreateScene() {
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

bool KG::Physics::PhysicsScene::Advance(float timeElapsed) {
	accumulator += timeElapsed;
	while (accumulator >= stepSize) {
		accumulator -= stepSize;
		scene->simulate(stepSize);
	}
	scene->fetchResults();
	return true;
}

void KG::Physics::PhysicsScene::AddActor(DirectX::XMFLOAT3 position, float width, float height, float depth)
{
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(position.x, position.y, position.z), PxBoxGeometry(width, height, depth), *pMaterial, 1);
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, false);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, false);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, false);
	
	// actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	actor->setAngularDamping(PX_MAX_F32);
	scene->addActor(*actor);
	rigid.push_back(actor);
}

void KG::Physics::PhysicsScene::AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth)
{
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);
	PxRigidStatic* actor = PxCreateStatic(*physics, PxTransform(position.x, position.y, position.z), PxBoxGeometry(width, height, depth), *pMaterial);
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
	scene->addActor(*actor);
}

void KG::Physics::PhysicsScene::AddFloor(float height)
{
	PxMaterial* planeMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* plane = PxCreatePlane(*physics, PxPlane(PxVec3(0, 1, 0), -height), *planeMaterial);
	scene->addActor(*plane);
}

void KG::Physics::PhysicsScene::Move(DirectX::XMFLOAT3 vector)
{
	rigid[0]->setLinearVelocity(PxVec3(vector.x, vector.y, vector.z)*1.5);
	rigid[0]->setLinearDamping(1);
	// rigid[0]->addForce(PxVec3(vector.x, vector.y, vector.z) * rigid[0]->getMass() * 5);
}

DirectX::XMFLOAT3 KG::Physics::PhysicsScene::GetPosition()
{
	PxVec3 pos = rigid[0]->getGlobalPose().p;
	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
	// TODO: 여기에 return 문을 삽입합니다.
}

void KG::Physics::PhysicsScene::SetRotation(DirectX::XMFLOAT4 quat)
{
	rigid[0]->getGlobalPose().q = PxQuat(quat.x, quat.y, quat.z, quat.w);
}
