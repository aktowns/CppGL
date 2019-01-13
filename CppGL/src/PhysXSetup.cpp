#include <PxPhysicsAPI.h>
#include <PxFoundation.h>
#include <foundation/PxSimpleTypes.h>

#include "PhysXSetup.hpp"

using namespace physx;

#ifndef _WINDOWS 
#define aalloc(sz) memalign(16, sz)
#define afree free
#else
#define aalloc(sz) _aligned_malloc(sz, 16)
#define afree _aligned_free
#endif

class MyAllocator : public PxAllocatorCallback
{
public:
	void* allocate(size_t size, const char*, const char*, int)
	{
		void* ptr = aalloc(size);
		PX_ASSERT((reinterpret_cast<size_t>(ptr) & 15)==0);
		return ptr;
	}

	void deallocate(void* ptr)
	{
		afree(ptr);
	}
};

class MyErrorCallback : public PxErrorCallback
{
public:
	void reportError(PxErrorCode::Enum code, const char* message, const char* file,
		int line)
	{}
};

MyAllocator		gAllocator;
MyErrorCallback	gErrorCallback;

PxFoundation*			gFoundation = NULL;
PxPhysics*				gPhysics	= NULL;

PxDefaultCpuDispatcher*	gDispatcher = NULL;

PxMaterial*				gMaterial	= NULL;

PxPvd*                  gPvd        = NULL;

PxReal stackZ = 10.0f;

PxRigidDynamic* PhysXSetup::createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity=PxVec3(0))
{
	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
	dynamic->setAngularDamping(0.5f);
	dynamic->setLinearVelocity(velocity);
	scene->addActor(*dynamic);
	return dynamic;
}

PxRigidDynamic* PhysXSetup::createActor(const PxTransform& t) 
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *gMaterial);
	PxRigidDynamic* body = gPhysics->createRigidDynamic(t);
	body->attachShape(*shape);
	scene->addActor(*body);

	return body;
}

void PhysXSetup::createStack(const PxTransform& t, PxU32 size, PxReal halfExtent)
{
	PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
	for(PxU32 i=0; i<size;i++)
	{
		for(PxU32 j=0;j<size-i;j++)
		{
			PxTransform localTm(PxVec3(PxReal(j*2) - PxReal(size-i), PxReal(i*2+1), 0) * halfExtent);
			PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
			body->attachShape(*shape);
			PxRigidBodyExt::updateMassAndInertia(*body, 0.01f);
			scene->addActor(*body);
		}
	}
	shape->release();
}

void PhysXSetup::initPhysics()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.bounceThresholdVelocity = 0.2;
	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	scene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = scene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 10), *gMaterial);
	scene->addActor(*groundPlane);

	//for (PxU32 i = 0; i < 5; i++)
	//	createStack(PxTransform(PxVec3(0, 0, stackZ -= 10.0f)), 10, 2.0f);

	//createDynamic(PxTransform(PxVec3(0, 40, 100)), PxSphereGeometry(10), PxVec3(0, -50, -100));
	scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eWORLD_AXES, 1.0);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AXES, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_COMPOUNDS, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_AABBS, 1.0f);
	scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_EDGES, 1.0f);
}

const float stepSize = 1.0f / 60.0f;
void PhysXSetup::stepPhysics(float deltaTime)
{
	_accumulator += deltaTime;
	if (_accumulator < stepSize) {
		return;
	}
	_accumulator -= stepSize;
	scene->simulate(stepSize);
	scene->fetchResults(true);
}
	
void PhysXSetup::cleanupPhysics()
{
	scene->release();
	gDispatcher->release();
	gPhysics->release();	
	PxPvdTransport* transport = gPvd->getTransport();
	gPvd->release();
	transport->release();
	
	gFoundation->release();
}
/*
void keyPress(unsigned char key, const PxTransform& camera)
{
	switch(toupper(key))
	{
	case 'B':	createStack(PxTransform(PxVec3(0,0,stackZ-=10.0f)), 10, 2.0f);						break;
	case ' ':	createDynamic(camera, PxSphereGeometry(3.0f), camera.rotate(PxVec3(0,0,-1))*200);	break;
	}
}
*/
