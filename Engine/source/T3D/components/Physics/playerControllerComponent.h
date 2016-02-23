//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLAYER_CONTORLLER_COMPONENT_H_
#define _PLAYER_CONTORLLER_COMPONENT_H_

#ifndef _PHYSICSBEHAVIOR_H_
#include "T3D/Components/Physics/physicsBehavior.h"
#endif

#ifndef __RESOURCE_H__
#include "core/resource.h"
#endif
#ifndef _TSSHAPE_H_
#include "ts/tsShape.h"
#endif
#ifndef _SCENERENDERSTATE_H_
#include "scene/sceneRenderState.h"
#endif
#ifndef _MBOX_H_
#include "math/mBox.h"
#endif
#ifndef _ENTITY_H_
#include "T3D/Entity.h"
#endif
#ifndef _CONVEX_H_
#include "collision/convex.h"
#endif
#ifndef _BOXCONVEX_H_
#include "collision/boxConvex.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif
#ifndef _T3D_PHYSICS_PHYSICSWORLD_H_
#include "T3D/physics/physicsWorld.h"
#endif
#ifndef _PHYSICS_COMPONENT_INTERFACE_H_
#include "T3D/Components/physics/physicsComponentInterface.h"
#endif
#ifndef _COLLISION_INTERFACES_H_
#include "T3D/Components/collision/collisionInterfaces.h"
#endif

class SceneRenderState;
class PhysicsWorld;
class PhysicsPlayer;
class SimplePhysicsBehaviorInstance;
class CollisionInterface;

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class PlayerControllerComponent : public Component,
   public PhysicsComponentInterface
{
   typedef Component Parent;

   enum MaskBits {
      VelocityMask = Parent::NextFreeMask << 0,
      PositionMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2
   };

   struct StateDelta
   {
      Move move;                    ///< Last move from server
      F32 dt;                       ///< Last interpolation time
      // Interpolation data
      Point3F pos;
      Point3F posVec;
      QuatF rot[2];
      // Warp data
      S32 warpTicks;                ///< Number of ticks to warp
      S32 warpCount;                ///< Current pos in warp
      Point3F warpOffset;
      QuatF warpRot[2];
   };

   StateDelta mDelta;

   PhysicsPlayer *mPhysicsRep;
   PhysicsWorld  *mPhysicsWorld;

   CollisionInterface* mOwnerCollisionInterface;

   struct ContactInfo
   {
      bool contacted, jump, run;
      SceneObject *contactObject;
      VectorF  contactNormal;
      F32 contactTime;

      void clear()
      {
         contacted = jump = run = false;
         contactObject = NULL;
         contactNormal.set(1, 1, 1);
      }

      ContactInfo() { clear(); }

   } mContactInfo;

protected:
   F32 mDrag;
   F32 mBuoyancy;
   F32 mFriction;
   F32 mElasticity;
   F32 mMaxVelocity;
   bool mSticky;

   bool mFalling;
   bool mSwimming;
   bool mInWater;

   S32 mContactTimer;               ///< Ticks since last contact

   U32 mIntegrationCount;

   Point3F mJumpSurfaceNormal;      ///< Normal of the surface the player last jumped on

   F32 maxStepHeight;         ///< Maximum height the player can step up
   F32 moveSurfaceAngle;      ///< Maximum angle from vertical in degrees the player can run up
   F32 contactSurfaceAngle;   ///< Maximum angle from vertical in degrees we consider having real 'contact'

   F32 horizMaxSpeed;         ///< Max speed attainable in the horizontal
   F32 horizMaxAccel;
   F32 horizResistSpeed;      ///< Speed at which resistance will take place
   F32 horizResistFactor;     ///< Factor of resistance once horizResistSpeed has been reached

   F32 upMaxSpeed;            ///< Max vertical speed attainable
   F32 upMaxAccel;
   F32 upResistSpeed;         ///< Speed at which resistance will take place
   F32 upResistFactor;        ///< Factor of resistance once upResistSpeed has been reached

   F32 fallingSpeedThreshold; ///< Downward speed at which we consider the player falling

   // Air control
   F32 airControl;

   Point3F mInputVelocity;

   bool mUseDirectMoveInput;

public:
   PlayerControllerComponent();
   virtual ~PlayerControllerComponent();
   DECLARE_CONOBJECT(PlayerControllerComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();

   virtual void componentAddedToOwner(Component *comp);
   virtual void componentRemovedFromOwner(Component *comp);

   virtual void ownerTransformSet(MatrixF *mat);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   void updatePhysics(PhysicsCollision *collision = NULL);

   virtual void processTick();
   virtual void interpolateTick(F32 dt);
   virtual void updatePos(const F32 dt);
   void updateMove();

   virtual VectorF getVelocity() { return mVelocity; }
   virtual void setVelocity(const VectorF& vel);
   virtual void setTransform(const MatrixF& mat);

   void findContact(bool *run, bool *jump, VectorF *contactNormal);
   Point3F getContactNormal() { return mContactInfo.contactNormal; }
   SceneObject* getContactObject() { return mContactInfo.contactObject; }
   bool isContacted() { return mContactInfo.contacted; }

   //
   void applyImpulse(const Point3F &pos, const VectorF &vec);

   //This is a weird artifact of the PhysicsReps. We want the collision component to be privvy to any events that happen
   //so when the physics components do a findContact test during their update, they'll have a signal collision components
   //can be listening to to update themselves with that info
   Signal< void(SceneObject*) > PlayerControllerComponent::onContactSignal;

   //
   DECLARE_CALLBACK(void, updateMove, (PlayerControllerComponent* obj));
};

#endif // _COMPONENT_H_
