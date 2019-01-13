#pragma once

#include <PxPhysics.h>

class PhysXSetup
{
	double _accumulator;
public:
	PhysXSetup() : _accumulator(0) {}
	physx::PxScene* scene;
	void initPhysics();
	void stepPhysics(float deltaTime);
	physx::PxRigidDynamic* createDynamic(const physx::PxTransform& t,
		const physx::PxGeometry& geometry, const physx::PxVec3& velocity);
	void createStack(const physx::PxTransform& t, physx::PxU32 size, physx::PxReal halfExtent);
	physx::PxRigidDynamic* createActor(const physx::PxTransform& t);
	void cleanupPhysics();
};

