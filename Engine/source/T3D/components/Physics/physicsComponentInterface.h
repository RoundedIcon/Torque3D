//This is basically a helper file that has general-usage behavior interfaces for rendering
#ifndef _PHYSICS_COMPONENT_INTERFACE_H_
#define _PHYSICS_COMPONENT_INTERFACE_H_

#include "T3D/Components/coreInterfaces.h"

class PhysicsComponentInterface : public Interface<PhysicsComponentInterface>
{
protected:
   VectorF  mVelocity;
   F32      mMass;

   F32		mGravityMod;

public:
   void updateForces();

   VectorF getVelocity() { return mVelocity; }
   void setVelocity(VectorF vel) { mVelocity = vel; }

   F32 getMass() { return mMass; }

   Signal< void(VectorF normal, Vector<SceneObject*> overlappedObjects) > PhysicsComponentInterface::onPhysicsCollision;
};
#endif