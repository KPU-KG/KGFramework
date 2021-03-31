#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"
#include "PhysicsComponent.h"
#include "PhysicsSystem.h"
#include "ComponentProvider.h"

using namespace physx;
using namespace KG::Physics;

struct KG::Physics::PhysicsScene::PhysicsSystems
{
	KG::System::PhysicsSystem physicsSystem;

	void OnPreRender()
	{
		this->physicsSystem.OnPreRender();
	}

	void OnUpdate(float elapsedTime)
	{
		this->physicsSystem.OnUpdate(elapsedTime);
	}
	void OnPostUpdate(float elapsedTime)
	{
		this->physicsSystem.OnPostUpdate(elapsedTime);
	}

	void PostComponentProvider(KG::Component::ComponentProvider& provider)
	{
		this->physicsSystem.OnPostProvider(provider);
	}

	void Clear()
	{
		this->physicsSystem.Clear();
	}
};

KG::Physics::PhysicsScene::PhysicsScene()
{

}

void KG::Physics::PhysicsScene::Initialize() {

	// physicsSystems = std::make_unique<PhysicsSystems>();
	PhysicsScene::instance = this;
	physicsSystems = new PhysicsSystems();
	const char* strTransport = "127.0.0.1";

	allocator = new PxDefaultAllocator();
	errorCallback = new PxDefaultErrorCallback();

	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *allocator, *errorCallback);

	if (!foundation)
		; //

	// PVD
	if (desc.connectPVD) {
		pvd = PxCreatePvd(*foundation);
		PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(strTransport, 5425, 10);
		bool pvdConnectionResult = pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
		if (!pvdConnectionResult) {

		}

	}

	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), desc.connectPVD, pvd);

	if (!physics)
		; // return false;

	cpuDispatcher = PxDefaultCpuDispatcherCreate(1);

	CreateScene(desc.gravity);
}

bool KG::Physics::PhysicsScene::CreateScene(float gravity) {
	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -gravity, 0.0f);
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
	this->physicsSystems->OnPostUpdate(timeElapsed);
	return true;
}

void KG::Physics::PhysicsScene::AddDynamicActor(KG::Component::DynamicRigidComponent* rigid)
{
	// ������ ���������� �ۿ��� �Ͼ�� ���� �ڽ��� ó��
	// �� �ܿ� �浹 ������ �ؾ� �ϳ� ������ �ۿ��� ���ص� �Ǵ� �� (�Ѿ˿� �´� ���� ��)�� KINETIC �÷��� ����
	// �׷��� �ݸ��� �ڽ��� 2���� ������ ���� / kinetic, dynamic
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);		// Basic Setting : ���߿� �ʿ��ϸ� �߰� ( ���� ���� ���, ���� ���� ���, ��ź ���)
	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), PxBoxGeometry(cb.scale.x, cb.scale.y, cb.scale.z), *pMaterial, 1);

#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);
#endif
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, false);		// DynamicRigidComponent�� �÷��� �߰�
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, false);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, false);

	scene->addActor(*actor);
	rigid->SetActor(actor);

	// rigid->
}

// void KG::Physics::PhysicsScene::AddDynamicActor(DirectX::XMFLOAT3 position, float width, float height, float depth)
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

KG::Component::DynamicRigidComponent* KG::Physics::PhysicsScene::GetNewPhysicsComponent()
{
	auto* physicsComponent = physicsSystems->physicsSystem.GetNewComponent();
	// ddDynamicActor(physicsComponent);
	return physicsComponent;
}

void KG::Physics::PhysicsScene::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
	physicsSystems->PostComponentProvider(provider);
}

// KG::Component::DynamicRigidComponent* KG::Physics::PhysicsScene::GetNewRenderComponent()
// {
// 	// this->physicsSystems
// 	// return nullptr;
// }

// void KG::Physics::PhysicsScene::Move(DirectX::XMFLOAT3 vector)
// {
// 	rigid[0]->setLinearVelocity(PxVec3(vector.x, vector.y, vector.z)*1.5);
// 	rigid[0]->setLinearDamping(1);
// 	// rigid[0]->addForce(PxVec3(vector.x, vector.y, vector.z) * rigid[0]->getMass() / */ 5);
// }
// 
// DirectX::XMFLOAT3 KG::Physics::PhysicsScene::GetPosition()
// {
// 	PxVec3 pos = rigid[0]->getGlobalPose().p;
// 	return DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
// 	// TODO: ���⿡ return ���� �����մϴ�.
// }
// 
// void KG::Physics::PhysicsScene::SetRotation(DirectX::XMFLOAT4 quat)
// {
// 	rigid[0]->getGlobalPose().q = PxQuat(quat.x, quat.y, quat.z, quat.w);
// 	// rigid[0]->
// }
