#include "PhysicsScene.h"
#include "PxPhysicsAPI.h"
#include "PhysicsComponent.h"
#include "PhysicsSystem.h"
#include "ComponentProvider.h"
#include "Transform.h"
#include "MathHelper.h"
#include "PhysicsComponent.h"
#include <unordered_map>

using namespace physx;
using namespace KG::Physics;

constexpr int MAX_COMPONENT = 10000;


static PxMat44 DxMatToPxMat(const DirectX::XMFLOAT4X4& rMatrix)
{
    return PxMat44(
        PxVec4(rMatrix._11, rMatrix._12, rMatrix._13, rMatrix._14),
        PxVec4(rMatrix._21, rMatrix._22, rMatrix._23, rMatrix._24),
        PxVec4(rMatrix._31, rMatrix._32, rMatrix._33, rMatrix._34),
        PxVec4(rMatrix._41, rMatrix._42, rMatrix._43, rMatrix._44)
    );
};


struct CallbackParam
{
    std::function<void(KG::Component::IRigidComponent*, KG::Component::IRigidComponent*)> collisionCallback;
    KG::Component::IRigidComponent* my;
    KG::Component::IRigidComponent* other;

    void DoCallback()
    {
        collisionCallback(my, other);
    }
};

std::unordered_map<physx::PxActor*, CallbackParam> CollisionCallback;
std::unordered_map<unsigned int, KG::Component::IRigidComponent*> compIndex;

class KG::Physics::PhysicsEventCallback : public physx::PxSimulationEventCallback
{
public:
    virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override
    {
        DebugNormalMessage("Called onConstraintBreak()");
        //  eventCallback[PHYSICS_CALLBACK::CONSTRAINT]();
    }
    void onWake(physx::PxActor** actors, physx::PxU32 count) override
    {
        DebugNormalMessage("Called onWake()");
        //eventCallback[PHYSICS_CALLBACK::WAKE]();
    }
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override
    {
        DebugNormalMessage("Called onSleep()");
        //  eventCallback[PHYSICS_CALLBACK::SLEEP]();
    }
    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
    {
        DebugNormalMessage("Called onTrigger()");
        // eventCallback[PHYSICS_CALLBACK::TRIGGER]();
    }
    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override
    {
        DebugNormalMessage("Called onAdvance()");
        // eventCallback[PHYSICS_CALLBACK::ADVANCE]();
    }
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override
    {
        //DebugNormalMessage("Called onContact()");
        if (CollisionCallback.count(pairHeader.actors[0]) != 0) {
            CollisionCallback[pairHeader.actors[0]].DoCallback();
            CollisionCallback.erase(pairHeader.actors[0]);
        }
        if (CollisionCallback.count(pairHeader.actors[1]) != 0) {
            CollisionCallback[pairHeader.actors[1]].DoCallback();
            CollisionCallback.erase(pairHeader.actors[1]);
        }
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

    if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1)) {
        pairFlags = physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
        return physx::PxFilterFlag::eDEFAULT;
    }

    pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

    if ( !(filterData0.word0 & filterData1.word1) && !(filterData0.word1 & filterData1.word0) )
    {
        pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT | physx::PxPairFlag::eCONTACT_DEFAULT;

        KG::Component::IRigidComponent* comp1 = nullptr;
        KG::Component::IRigidComponent* comp2 = nullptr;

        if ( compIndex.count(filterData0.word2) != 0 )
            comp1 = compIndex[filterData0.word2];

        if ( compIndex.count(filterData1.word2) != 0 )
            comp2 = compIndex[filterData1.word2];


        if ( comp1 == nullptr )
            ;
        else if ( CollisionCallback.count(comp1->GetActor()) == 0 )
        {
            if ( comp1->GetCollisionCallback() != nullptr )
            {
                CallbackParam cp;
                cp.collisionCallback = comp1->GetCollisionCallback();
                cp.my = comp1;
                cp.other = comp2;
                CollisionCallback[comp1->GetActor()] = cp;
            }
        }

        if ( comp2 == nullptr )
            ;
        else if ( CollisionCallback.count(comp2->GetActor()) == 0 )
        {
            if ( comp2->GetCollisionCallback() != nullptr )
            {
                CallbackParam cp;
                cp.collisionCallback = comp2->GetCollisionCallback();
                cp.my = comp2;
                cp.other = comp1;
                CollisionCallback[comp2->GetActor()] = cp;
            }
        }
        return physx::PxFilterFlag::eDEFAULT;
    }
    else {
        return physx::PxFilterFlag::eDEFAULT;
    }

    if ( (filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1) )
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

    virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override
    {
        //std::cout << "되나?" << std::endl;
    }
};

void KG::Physics::PhysicsScene::Initialize()
{

    PhysicsScene::instance = this;

    // physicsSystems = std::make_unique<PhysicsSystems>();
    physicsSystems = new PhysicsSystems();
    const char* strTransport = "127.0.0.1";

    allocator = new PxDefaultAllocator();
    errorCallback = new PxDefaultErrorCallback();
    // errorCallback = new ErrorCallback();
    // errorCallback->reportError()

    foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *allocator, *errorCallback);

    if ( !foundation )
        ; //

    // PVD
    if ( desc.connectPVD )
    {
        pvd = PxCreatePvd(*foundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(strTransport, 5425, 10);
        bool pvdConnectionResult = pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
        if ( !pvdConnectionResult )
        {

        }

    }

    physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), desc.connectPVD, pvd);

    if ( !physics )
        ; // return false;

    cpuDispatcher = PxDefaultCpuDispatcherCreate(1);

    CreateScene(desc.gravity);
}

bool KG::Physics::PhysicsScene::CreateScene(float gravity)
{
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
    if ( !scene )
        return false;

    // PVD
    PxPvdSceneClient* pvdClient = scene->getScenePvdClient();
    if ( pvdClient )
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }
    initialized = true;
    return true;
}

bool KG::Physics::PhysicsScene::Advance(float timeElapsed)
{
    this->physicsSystems->OnUpdate(timeElapsed);
    accumulator += timeElapsed;
    while ( accumulator >= stepSize )
    {
        accumulator -= stepSize;
        scene->simulate(stepSize);
        scene->collide(stepSize);
        scene->fetchCollision(true);
        scene->advance();
        scene->fetchResults(true);
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
    PxRigidDynamic* actor = nullptr;
    for ( int i = 0; i < 3; ++i )
    {
        actor = PxCreateDynamic(*physics, PxTransform(cb.position.x, cb.position.y, cb.position.z),
            PxBoxGeometry(cb.scale.x / 2, cb.scale.y / 2, cb.scale.z / 2), *pMaterial, 1);
        if ( actor != nullptr )
            break;
    }

    if ( actor == nullptr )
    {
        DebugErrorMessage("actor is null!");
        return;
    }

    cb.position = Math::Vector3::Add(cb.position, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));
    PxTransform t = actor->getGlobalPose();
    t.p = { cb.position.x, cb.position.y, cb.position.z };
    auto objectQuat = rigid->GetGameObject()->GetTransform()->GetRotation();
    DirectX::XMStoreFloat4(&objectQuat, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&objectQuat)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&cb.rotation))));
    t.q = PxQuat(objectQuat.x, objectQuat.y, objectQuat.z, objectQuat.w);
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
    for ( unsigned int i = UINT_MAX; i > UINT_MAX - MAX_COMPONENT; --i )
    {
        if ( compIndex.count(i) == 0 )
        {
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
    DirectX::XMFLOAT4X4 cbMat = cb.GetMatrix();
    auto localMat = KG::Math::Matrix4x4::Multiply(cbMat, worldMat);

    PxRigidStatic* actor = nullptr;
    {
        DirectX::XMVECTOR s{};
        DirectX::XMVECTOR r{};
        DirectX::XMVECTOR t{};

        DirectX::XMMatrixDecompose(&s, &r, &t, DirectX::XMLoadFloat4x4(&localMat));

        DirectX::XMFLOAT3 sf{};
        DirectX::XMFLOAT4 rf{};
        DirectX::XMFLOAT3 tf{};
        DirectX::XMStoreFloat3(&sf, s);
        DirectX::XMStoreFloat4(&rf, r);
        DirectX::XMStoreFloat3(&tf, t);

        // trans 41 42 43
        //cb.scale = Math::Vector3::Multiply(cb.scale, DirectX::XMFLOAT3(worldMat._11, worldMat._22, worldMat._33));


        for ( int i = 0; i < 3; ++i )
        {
            actor = PxCreateStatic(*physics, PxTransform(PxVec3(tf.x, tf.y, tf.z), PxQuat(rf.x, rf.y, rf.z, rf.w)),
                PxBoxGeometry(abs(sf.x / 2), abs(sf.y / 2), abs(sf.z / 2)), *pMaterial);
            if ( actor != nullptr )
                break;
        }
    }

    //for (int i = 0; i < 3; ++i) {
    //	actor = PxCreateStatic(*physics, PxTransform(cb.position.x, cb.position.y, cb.position.z),
    //		PxBoxGeometry(abs(cb.scale.x / 2), abs(cb.scale.y / 2), abs(cb.scale.z / 2)), *pMaterial);
    //	if (actor != nullptr)
    //		break;
    //}

    if ( actor == nullptr )
    {
        rigid->SetActor(actor);
        DebugErrorMessage("actor is null!");
        return;
    }





#ifdef _DEBUG
    actor->setActorFlag(PxActorFlag::eVISUALIZATION, true);				// PVD에 보여지는지 체크
#endif
    cb.position = Math::Vector3::Add(cb.position, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));
    PxTransform t = actor->getGlobalPose();
    t.p = { cb.position.x, cb.position.y, cb.position.z };
    auto objectQuat = rigid->GetGameObject()->GetTransform()->GetRotation();
    DirectX::XMStoreFloat4(&objectQuat, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&objectQuat)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&cb.rotation))));
    t.q = PxQuat(objectQuat.x, objectQuat.y, objectQuat.z, objectQuat.w);
    actor->setGlobalPose(t);

    scene->addActor(*actor);
    rigid->SetActor(actor);
    for ( unsigned int i = UINT_MAX; i > UINT_MAX - MAX_COMPONENT; --i )
    {
        if ( compIndex.count(i) == 0 )
        {
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

void KG::Physics::PhysicsScene::ReleaseActor(KG::Component::IRigidComponent* comp)
{
    auto* actor = comp->GetActor();
    if (CollisionCallback.count(actor) != 0)
        CollisionCallback.erase(actor);
    if (compIndex.count(comp->GetId()) != 0)
        compIndex.erase(comp->GetId());
    this->scene->removeActor(*actor);
}

KG::Component::IRigidComponent* KG::Physics::PhysicsScene::QueryRaycast(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId, uint32_t mask)
{
    PxVec3 org{ origin.x, origin.y, origin.z };
    PxVec3 dir{ direction.x, direction.y, direction.z };
    PxReal dst = maxDistance;
    dir.normalize();

    PxRaycastBuffer hit;

    PxQueryFilterData filter = PxQueryFilterData();
    filter.data.word0 = mask;
    if (mask != 0) {
        // if (scene->raycast(org, dir, dst, buf, PxHitFlag::eDEFAULT, filter))
        if (scene->raycast(org, dir, dst, hit, PxHitFlag::eDEFAULT, filter))
        {
            PxU32 hitId = hit.block.shape->getSimulationFilterData().word2;
            if (myId != hitId)
            {
                if (compIndex.count(hitId) != 0)
                    return compIndex[hitId];
            }
            // for (int i = 0; i < buf.getNbTouches(); ++i)
            // {
            //     PxU32 hitId = hit[i].shape->getSimulationFilterData().word2;
            //     if (myId != hitId)
            //     {
            //         if (compIndex.count(hitId) != 0)
            //             return compIndex[hitId];
            //     }
            // }
        }
    }
    else {
        //  if (scene->raycast(org, dir, dst, buf))
        if (scene->raycast(org, dir, dst, hit))
        {
            PxU32 hitId = hit.block.shape->getSimulationFilterData().word2;
            if (myId != hitId)
            {
                if (compIndex.count(hitId) != 0)
                    return compIndex[hitId];
            }
            // for (int i = 0; i < buf.getNbTouches(); ++i)
            // {
            //     PxU32 hitId = hit[i].shape->getSimulationFilterData().word2;
            //     if (myId != hitId)
            //     {
            //         if (compIndex.count(hitId) != 0)
            //             return compIndex[hitId];
            //     }
            // }
        }
    }
    // if (mask == 0) {
    //     filter.data.word0 = UINT32_MAX;
    // }
    // if (scene->raycast(org, dir, dst, hit, PxHitFlag::eDEFAULT, filter))
    // {
    //     PxU32 hitId = hit.block.shape->getSimulationFilterData().word2;
    //     if (myId != hitId)
    //     {
    //         if (compIndex.count(hitId) != 0)
    //             return compIndex[hitId];
    //     }
    // }
    return nullptr;
}


RaycastResult KG::Physics::PhysicsScene::QueryRaycastResult(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId)
{
    PxVec3 org{ origin.x, origin.y, origin.z };
    PxVec3 dir{ direction.x, direction.y, direction.z };
    PxReal dst = maxDistance;
    PxRaycastBuffer hit;
    RaycastResult result{};
    if (scene->raycast(org, dir, dst, hit))
    {
        PxU32 hitId = hit.block.shape->getSimulationFilterData().word2;
        if (myId != hitId)
        {
            auto& curHit = hit.block;
            if (compIndex.count(hitId) != 0)
                result.targetRigid = compIndex[hitId];
            result.hitPosition = DirectX::XMFLOAT3(curHit.position.x, curHit.position.y, curHit.position.z);
            result.normal = DirectX::XMFLOAT3(curHit.normal.x, curHit.normal.y, curHit.normal.z);
            result.uv = DirectX::XMFLOAT2(curHit.u, curHit.v);
            result.distance = curHit.distance;

            return result;
        }
    }
    return result;
}

bool KG::Physics::PhysicsScene::IsInitialized() const
{
    return initialized;
}

std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> KG::Physics::PhysicsScene::GetStaticActorExtents()
{
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> bounds;

    PxActorTypeFlags types;
    types.set(PxActorTypeFlag::eRIGID_STATIC);

    PxActor** buf = nullptr;
    PxU32 size = this->scene->getNbActors(types);
    buf = new PxActor * [size];
    scene->getActors(types, buf, static_cast<physx::PxU32>(size), 1);

    for (int i = 0; i < size - 1; ++i) {
        auto wb = buf[i]->getWorldBounds();
        if (wb.isValid()) {
            auto e = wb.getExtents();
            auto p = wb.getCenter();
            bounds.emplace_back(std::make_pair(round(e.x), round(e.z)), std::make_pair(round(p.x), round(p.z)));
        }
    }

    return bounds;
}