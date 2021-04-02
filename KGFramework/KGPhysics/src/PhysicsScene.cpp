#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"
#include "PhysicsComponent.h"
#include "PhysicsSystem.h"
#include "ComponentProvider.h"

using namespace physx;
using namespace KG::Physics;

struct KG::Physics::PhysicsScene::PhysicsSystems
{
	KG::System::DynamicRigidSystem dynamicRigidSystem;
	KG::System::StaticRigidSystem staticRigidSystem;

	void OnPreRender()
	{
		this->dynamicRigidSystem.OnPreRender();
		this->staticRigidSystem.OnPreRender();
	}

	void OnUpdate(float elapsedTime)
	{
		this->dynamicRigidSystem.OnUpdate(elapsedTime);
		this->staticRigidSystem.OnUpdate(elapsedTime);
	}
	void OnPostUpdate(float elapsedTime)
	{
		this->dynamicRigidSystem.OnPostUpdate(elapsedTime);
		this->staticRigidSystem.OnPostUpdate(elapsedTime);
	}

	void PostComponentProvider(KG::Component::ComponentProvider& provider)
	{
		this->dynamicRigidSystem.OnPostProvider(provider);
		this->staticRigidSystem.OnPostProvider(provider);
	}

	void Clear()
	{
		this->dynamicRigidSystem.Clear();
		this->staticRigidSystem.Clear();
	}
};

KG::Physics::PhysicsScene::PhysicsScene()
{

}

void KG::Physics::PhysicsScene::Initialize() {

	PhysicsScene::instance = this;

	// physicsSystems = std::make_unique<PhysicsSystems>();
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
	// DirectX::XMFLOAT3 pos = rigid->
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);		// Basic Setting : ���߿� �ʿ��ϸ� �߰� ( ���� ���� ���, ���� ���� ���, ��ź ���)
	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), PxBoxGeometry(cb.scale.x, cb.scale.y, cb.scale.z), *pMaterial, 1);

#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD�� ���������� üũ
#endif
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, false);		// DynamicRigidComponent�� �÷��� �߰�
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, false);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, false);

	scene->addActor(*actor);
	rigid->SetActor(actor);
}

void KG::Physics::PhysicsScene::AddStaticActor(KG::Component::StaticRigidComponent* rigid)
{
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();
	PxRigidStatic* actor = PxCreateStatic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), PxBoxGeometry(cb.scale.x, cb.scale.y, cb.scale.z), *pMaterial);
#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD�� ���������� üũ
#endif
	scene->addActor(*actor);
	rigid->SetActor(actor);
}

void KG::Physics::PhysicsScene::AddFloor(float height)
{
	PxMaterial* planeMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);
	PxRigidStatic* plane = PxCreatePlane(*physics, PxPlane(PxVec3(0, 1, 0), -height), *planeMaterial);
	scene->addActor(*plane);
}

KG::Component::DynamicRigidComponent* KG::Physics::PhysicsScene::GetNewDynamicRigidComponent()
{
	auto* comp = physicsSystems->dynamicRigidSystem.GetNewComponent();
	return comp;
}

KG::Component::StaticRigidComponent* KG::Physics::PhysicsScene::GetNewStaticRigidComponent()
{
	auto* comp = physicsSystems->staticRigidSystem.GetNewComponent();
	return comp;
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
