#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"
#include "PhysicsComponent.h"
#include "PhysicsSystem.h"
#include "ComponentProvider.h"
#include "Transform.h"
#include <unordered_map>

using namespace physx;
using namespace KG::Physics;

constexpr const int MAX_COMPONENT = 10000;

struct CallbackParam {
	std::function<void(KG::Component::IRigidComponent*, KG::Component::IRigidComponent*)> callback;
	KG::Component::IRigidComponent* my;
	KG::Component::IRigidComponent* other;

	void DoCallback() {
		callback(my, other);
	}
};

std::unordered_map<physx::PxActor*, CallbackParam> CollisionCallback;
std::unordered_map<unsigned int, KG::Component::IRigidComponent*> compIndex;

class KG::Physics::PhysicsEventCallback : public physx::PxSimulationEventCallback {
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
		if (CollisionCallback.count(pairHeader.actors[0]) != 0)
			CollisionCallback[pairHeader.actors[0]].DoCallback();
		if (CollisionCallback.count(pairHeader.actors[1]) != 0)
			CollisionCallback[pairHeader.actors[1]].DoCallback();
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
	// 여기서 어떤 충돌인지 정의하고 콜백 호출
	if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlag::eDEFAULT;
	}

	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	if (!(filterData0.word0 & filterData1.word1) && !(filterData0.word1 & filterData1.word0)) {
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT | physx::PxPairFlag::eCONTACT_DEFAULT;

		KG::Component::IRigidComponent* comp1 = nullptr;
		KG::Component::IRigidComponent* comp2 = nullptr;

		if (compIndex.count(filterData0.word2) != 0)
			comp1 = compIndex[filterData0.word2];

		if (compIndex.count(filterData1.word2) != 0)
			comp2 = compIndex[filterData1.word2];


		if (comp1 == nullptr)
			;
		else if (CollisionCallback.count(comp1->GetActor()) == 0) {
			if (comp1->GetCollisionCallback() != nullptr) {
				CallbackParam cp;
				cp.callback = comp1->GetCollisionCallback();
				cp.my = comp1;
				cp.other = comp2;
				CollisionCallback[comp1->GetActor()] = cp;
			}
		}

		if (comp2 == nullptr)
			;
		else if (CollisionCallback.count(comp2->GetActor()) == 0) {
			if (comp2->GetCollisionCallback() != nullptr) {
				CallbackParam cp;
				cp.callback = comp2->GetCollisionCallback();
				cp.my = comp2;
				cp.other = comp1;
				CollisionCallback[comp2->GetActor()] =cp;
			}
		}
		return physx::PxFilterFlag::eDEFAULT;
	}

	if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	return physx::PxFilterFlag::eDEFAULT;
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

class ErrorCallback : public PxErrorCallback
{
public:
	ErrorCallback() {};
	~ErrorCallback() {};

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override {
		std::cout << "되나?" << std::endl;
	}
};

void KG::Physics::PhysicsScene::Initialize() {

	PhysicsScene::instance = this;

	// physicsSystems = std::make_unique<PhysicsSystems>();
	physicsSystems = new PhysicsSystems();
	const char* strTransport = "127.0.0.1";

	allocator = new PxDefaultAllocator();
	// errorCallback = new PxDefaultErrorCallback();
	errorCallback = new ErrorCallback();
	// errorCallback->reportError()

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
	// sceneDesc.filterCallback;
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
	this->physicsSystems->OnUpdate(timeElapsed);
	accumulator += timeElapsed;
	while (accumulator >= stepSize) {
		accumulator -= stepSize;
		scene->simulate(stepSize);
		scene->collide(stepSize);
		scene->fetchCollision();
		scene->fetchResults();
	}
	this->physicsSystems->OnPostUpdate(timeElapsed);
	return true;
}

void KG::Physics::PhysicsScene::AddDynamicActor(KG::Component::DynamicRigidComponent* rigid)
{
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.0f);		// Basic Setting : 나중에 필요하면 추가 ( 정적 마찰 계수, 동적 마찰 계수, 반탄 계수)

	DirectX::XMFLOAT4X4 worldMat = rigid->GetGameObject()->GetTransform()->GetGlobalWorldMatrix();
	// trans 41 42 43
	cb.scale = Math::Vector3::Multiply(cb.scale, DirectX::XMFLOAT3(worldMat._11, worldMat._22, worldMat._33));

	PxRigidDynamic* actor = PxCreateDynamic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), 
		PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2), *pMaterial, 1);
	
	cb.center = Math::Vector3::Add(cb.center, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));
	PxTransform t = actor->getGlobalPose();
	t.p = { cb.center.x, cb.center.y, cb.center.z };
	actor->setGlobalPose(t);
	

#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD에 보여지는지 체크
#endif
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);		// DynamicRigidComponent에 플래그 추가
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
	actor->setRigidDynamicLockFlag(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, true); 
	// actor->

	scene->addActor(*actor);
	rigid->SetActor(actor);

	// 나중에 아이디 생성 추가
	for (int i = UINT_MAX; i > UINT_MAX - MAX_COMPONENT; --i) {
		if (compIndex.count(i) == 0) {
			compIndex[i] = rigid;
			rigid->SetId(i);
			break;
		}
	}

}

void KG::Physics::PhysicsScene::AddStaticActor(KG::Component::StaticRigidComponent* rigid)
{
	PxMaterial* pMaterial = physics->createMaterial(0.5f, 0.5f, 0.5f);
	KG::Component::CollisionBox cb = rigid->GetCollisionBox();

	DirectX::XMFLOAT4X4 worldMat = rigid->GetGameObject()->GetTransform()->GetGlobalWorldMatrix();
	// trans 41 42 43
	cb.scale = Math::Vector3::Multiply(cb.scale, DirectX::XMFLOAT3(worldMat._11, worldMat._22, worldMat._33));

	PxRigidStatic* actor = PxCreateStatic(*physics, PxTransform(cb.center.x, cb.center.y, cb.center.z), 
		PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2), *pMaterial);
#ifdef _DEBUG
	actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD에 보여지는지 체크
#endif
	scene->addActor(*actor);
	rigid->SetActor(actor);
	for (int i = UINT_MAX; i > UINT_MAX - MAX_COMPONENT; --i) {
		if (compIndex.count(i) == 0) {
			compIndex[i] = rigid;
			rigid->SetId(i);
			break;
		}
	}
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

KG::Component::IRigidComponent* KG::Physics::PhysicsScene::QueryRaycast(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance)
{
	PxVec3 org{ origin.x, origin.y, origin.z };
	PxVec3 dir{ direction.x, direction.y, direction.z };
	PxReal dst = maxDistance;
	PxRaycastBuffer hit;
	PxQueryFilterData filter;
	if (scene->raycast(org, dir, dst, hit, PxHitFlag::eDEFAULT, filter)) {
		for (auto& com : compIndex) {
			if (com.second->GetActor() == hit.block.actor) {
				return com.second;
			}
		}

		if (compIndex.count(filter.data.word2) == 0)
			return nullptr;
		return compIndex[filter.data.word2];
	}
	return nullptr;
}