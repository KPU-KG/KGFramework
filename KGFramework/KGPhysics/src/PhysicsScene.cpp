#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"
#include "PhysicsComponent.h"
#include "PhysicsSystem.h"
#include "ComponentProvider.h"
#include "Transform.h"

using namespace physx;
using namespace KG::Physics;

class KG::Physics::PhysicsEventCallback : public physx::PxSimulationEventCallback {
	// std::unordered_map<KG::Physics::PHYSICS_CALLBACK, void* ()> eventCallback;
	// �ݹ� �Լ��� �� ��
	// �浹 ���(�̸� Ȥ�� ID) - simulation���� �������°Ŷ� �Ƹ��� �̸�
	// ���� ������Ʈ - �̰� Ư���� �� ������?
	// 
public:
	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {
		DebugNormalMessage("Called onConstraintBreak()");
		//  eventCallback[PHYSICS_CALLBACK::CONSTRAINT]();
	}
	void onWake(physx::PxActor** actors, physx::PxU32 count) override {
		DebugNormalMessage("Called onWake()");
		//eventCallback[PHYSICS_CALLBACK::WAKE]();
	}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) override {
		DebugNormalMessage("Called onSleep()");
		//  eventCallback[PHYSICS_CALLBACK::SLEEP]();
	}
	void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override {
		DebugNormalMessage("Called onTrigger()");
		// eventCallback[PHYSICS_CALLBACK::TRIGGER]();
	}
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {
		DebugNormalMessage("Called onAdvance()");
		// eventCallback[PHYSICS_CALLBACK::ADVANCE]();
	}
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override {
		DebugNormalMessage("Called onContact()");
	}
};

physx::PxFilterFlags contactReportFilterShader(physx::PxFilterObjectAttributes attributes0,
	physx::PxFilterData filterData0,
	physx::PxFilterObjectAttributes attributes1,
	physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags,
	const  void* constantBlock,
	physx::PxU32 constantBlockSize)
{
	(void)(constantBlockSize);
	(void)(constantBlock);

	//
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}

	// pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;
	pairFlags = physx::PxPairFlag::eMODIFY_CONTACTS | physx::PxPairFlag::eCONTACT_DEFAULT;
	// ������ ���͸����� ���� ��� ����ó ����
	// pairFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;

	// �� (A, B)�� ���� ����ó �ݹ��� Ʈ�����մϴ�. ���⼭ 
	// A�� ���� ����ũ�� B�� ID�� �����ϰ� �� �ݴ��� ��쵵 ���������Դϴ�. 
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	// return physx::PxFilterFlag::eCALLBACK;
	return physx::PxFilterFlag::eDEFAULT;

	// physx::PxDefaultSimulationFilterShader

	//   // ��� �׸� ���� ��� �ʱ� �� ���� ���� (����Ʈ �� ������ ����) 
	//   pairFlags = physx :: PxPairFlag :: eSOLVE_CONTACT | physx :: PxPairFlag :: eDETECT_DISCRETE_CONTACT 
	//         | physx :: PxPairFlag :: eNOTIFY_TOUCH_FOUND 
	//         | physx :: PxPairFlag :: eNOTIFY_TOUCH_PERSISTS 
	//         | physx :: PxPairFlag :: eNOTIFY_CONTACT_POINTS; 
	//   physx :: PxFilterFlag :: eNOTIFY�� ��ȯ�մϴ�. // : eCALLBACK; // physx :: PxFilterFlag :: eDEFAULT;
}


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
	physicsEventCallback = new PhysicsEventCallback();
	sceneDesc.simulationEventCallback = physicsEventCallback;
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.filterCallback;
	// sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	// sceneDesc.dynamicStructure = PxPruningStructureType::eLAST
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
		scene->collide(stepSize);
		scene->fetchCollision();
		// scene->advance();
		scene->fetchResults();
	}
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
	// auto box = PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2);
	// box.
	// PxFilterFlag::
	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), 
		PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2), *pMaterial, 1);
	
	// PxShape p =;
	// PxShape
	

#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD�� ���������� üũ
#endif
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);		// DynamicRigidComponent�� �÷��� �߰�
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, true); 
	// actor->

	scene->addActor(*actor);
	rigid->SetActor(actor);
}

void KG::Physics::PhysicsScene::AddStaticActor(KG::Component::StaticRigidComponent* rigid)
{
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();
	PxRigidStatic* actor = PxCreateStatic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), 
		PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2), *pMaterial);
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
