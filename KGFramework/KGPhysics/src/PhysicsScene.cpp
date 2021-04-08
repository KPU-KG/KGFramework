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
	// 콜백 함수에 들어갈 것
	// 충돌 대상(이름 혹은 ID) - simulation에서 가져오는거라 아마도 이름
	// 물리 컴포넌트 - 이거 특정할 수 있을까?
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
	// 위에서 필터링되지 않은 모든 연락처 생성
	// pairFlags = physx::PxPairFlag::eNOTIFY_CONTACT_POINTS | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;

	// 쌍 (A, B)에 대한 연락처 콜백을 트리거합니다. 여기서 
	// A의 필터 마스크는 B의 ID를 포함하고 그 반대의 경우도 마찬가지입니다. 
	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	// return physx::PxFilterFlag::eCALLBACK;
	return physx::PxFilterFlag::eDEFAULT;

	// physx::PxDefaultSimulationFilterShader

	//   // 모든 항목에 대한 모든 초기 및 지속 보고서 (포인트 별 데이터 포함) 
	//   pairFlags = physx :: PxPairFlag :: eSOLVE_CONTACT | physx :: PxPairFlag :: eDETECT_DISCRETE_CONTACT 
	//         | physx :: PxPairFlag :: eNOTIFY_TOUCH_FOUND 
	//         | physx :: PxPairFlag :: eNOTIFY_TOUCH_PERSISTS 
	//         | physx :: PxPairFlag :: eNOTIFY_CONTACT_POINTS; 
	//   physx :: PxFilterFlag :: eNOTIFY를 반환합니다. // : eCALLBACK; // physx :: PxFilterFlag :: eDEFAULT;
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
	// 실제로 물리적으로 작용이 일어나는 것은 박스로 처리
	// 그 외에 충돌 판정은 해야 하나 물리적 작용은 안해도 되는 것 (총알에 맞는 판정 등)은 KINETIC 플래그 설정
	// 그러면 콜리전 박스를 2개로 나눠서 관리 / kinetic, dynamic
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();
	// DirectX::XMFLOAT3 pos = rigid->
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);		// Basic Setting : 나중에 필요하면 추가 ( 정적 마찰 계수, 동적 마찰 계수, 반탄 계수)
	// auto box = PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2);
	// box.
	// PxFilterFlag::
	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), 
		PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2), *pMaterial, 1);
	
	// PxShape p =;
	// PxShape
	

#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD에 보여지는지 체크
#endif
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);		// DynamicRigidComponent에 플래그 추가
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
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD에 보여지는지 체크
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
