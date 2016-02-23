//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/Components/Physics/playerControllerComponent.h"

#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "T3D/gameBase/gameConnection.h"
#include "collision/collision.h"

#include "T3D/physics/physicsPlayer.h"
#include "T3D/physics/physicsPlugin.h"

#include "T3D/Components/Collision/collisionInterfaces.h"

#include "T3D/trigger.h"
#include "T3D/examples/triggerExample.h"
#include "T3D/components/collision/collisionTrigger.h"
//#include "T3D/Components/Game/mountedCameraComponent.h"

// Movement constants
static F32 sVerticalStepDot = 0.173f;   // 80
static F32 sMinFaceDistance = 0.01f;
static F32 sTractionDistance = 0.04f;
static F32 sNormalElasticity = 0.01f;
static U32 sMoveRetryCount = 5;
static F32 sMaxImpulseVelocity = 200.0f;

//////////////////////////////////////////////////////////////////////////
// Callbacks
IMPLEMENT_CALLBACK(PlayerControllerComponent, updateMove, void, (PlayerControllerComponent* obj), (obj),
   "Called when the player updates it's movement, only called if object is set to callback in script(doUpdateMove).\n"
   "@param obj the Player object\n");

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////
PlayerControllerComponent::PlayerControllerComponent() : Component()
{
   addComponentField("isStatic", "If enabled, object will not simulate physics", "bool", "0", "");
   addComponentField("gravity", "The direction of gravity affecting this object, as a vector", "vector", "0 0 -9", "");
   addComponentField("drag", "The drag coefficient that constantly affects the object", "float", "0.7", "");
   addComponentField("mass", "The mass of the object", "float", "1", "");

   mBuoyancy = 0.f;
   mFriction = 0.3f;
   mElasticity = 0.4f;
   mMaxVelocity = 3000.f;
   mSticky = false;
   
   mFalling = false;
   mSwimming = false;
   mInWater = false;

   mDelta.pos = mDelta.posVec = Point3F::Zero;
   mDelta.warpTicks = mDelta.warpCount = 0;
   mDelta.rot[0].identity(); 
   mDelta.rot[1].identity();
   mDelta.dt = 1;

   mUseDirectMoveInput = false;

   mFriendlyName = "Player Controller";
   mComponentType = "Physics";

   mDescription = getDescriptionText("A general-purpose physics player controller.");

   mNetFlags.set(Ghostable | ScopeAlways);

   mMass = 9.0f;         // from ShapeBase
   //maxEnergy = 60.0f;   // from ShapeBase
   mDrag = 1.0f;         // from ShapeBase

   maxStepHeight = 1.0f;
   moveSurfaceAngle = 60.0f;
   contactSurfaceAngle = 85.0f;

   fallingSpeedThreshold = -10.0f;

   horizMaxSpeed = 80.0f;
   horizMaxAccel = 100.0f;
   horizResistSpeed = 38.0f;
   horizResistFactor = 1.0f;

   upMaxSpeed = 80.0f;
   upMaxAccel = 100.0f;
   upResistSpeed = 38.0f;
   upResistFactor = 1.0f;

   //minImpactSpeed = 25.0f;
   //minLateralImpactSpeed = 25.0f;

   // Air control
   airControl = 0.0f;

   //Grav mod
   mGravityMod = 1;

   mInputVelocity = Point3F(0, 0, 0);

   mPhysicsRep = NULL;
   mPhysicsWorld = NULL;
}

PlayerControllerComponent::~PlayerControllerComponent()
{
   for (S32 i = 0; i < mFields.size(); ++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(PlayerControllerComponent);

//////////////////////////////////////////////////////////////////////////

bool PlayerControllerComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   //prepPhysicsBody();

   return true;
}

void PlayerControllerComponent::onRemove()
{
   Parent::onRemove();

   SAFE_DELETE(mPhysicsRep);
}

void PlayerControllerComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   updatePhysics();
}

void PlayerControllerComponent::componentAddedToOwner(Component *comp)
{
   if (comp->getId() == getId())
      return;

   //test if this is a shape component!
   CollisionInterface *collisionInterface = dynamic_cast<CollisionInterface*>(comp);
   if (collisionInterface)
   {
      collisionInterface->onCollisionChanged.notify(this, &PlayerControllerComponent::updatePhysics);
      mOwnerCollisionInterface = collisionInterface;
      updatePhysics();
   }

   /*RenderComponentInterface *renderInterface = dynamic_cast<RenderComponentInterface*>(comp);
   if (renderInterface)
   {
      renderInterface->onShapeInstanceChanged.notify(this, &PlayerControllerComponent::targetShapeChanged);
      prepPhysicsBody();
   }*/
}

void PlayerControllerComponent::componentRemovedFromOwner(Component *comp)
{
   if (comp->getId() == getId()) //?????????
      return;

   //test if this is a shape component!
   CollisionInterface *collisionInterface = dynamic_cast<CollisionInterface*>(comp);
   if (collisionInterface)
   {
      collisionInterface->onCollisionChanged.remove(this, &PlayerControllerComponent::updatePhysics);
      mOwnerCollisionInterface = NULL;
      updatePhysics();
   }
}

void PlayerControllerComponent::updatePhysics(PhysicsCollision *collision)
{
   if (!PHYSICSMGR)
      return;

   mPhysicsWorld = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

   //first, clear the old physRep
   SAFE_DELETE(mPhysicsRep);

   mPhysicsRep = PHYSICSMGR->createPlayer();

   F32 runSurfaceCos = mCos(mDegToRad(moveSurfaceAngle));

   Point3F ownerBounds = mOwner->getObjBox().getExtents() * mOwner->getScale();

   mPhysicsRep->init("", ownerBounds, runSurfaceCos, maxStepHeight, mOwner, mPhysicsWorld);

   mPhysicsRep->setTransform(mOwner->getTransform());
}

void PlayerControllerComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("inputVelocity", TypePoint3F, Offset(mInputVelocity, PlayerControllerComponent), "");
   addField("useDirectMoveInput", TypePoint3F, Offset(mUseDirectMoveInput, PlayerControllerComponent), "");
}

U32 PlayerControllerComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   /*if (stream->writeFlag(mask & PositionMask))
   {
      stream->writeFlag(mask & ForceMoveMask);

      stream->writeCompressedPoint(mRigid.linPosition);
      mathWrite(*stream, mRigid.angPosition);
      mathWrite(*stream, mRigid.linMomentum);
      mathWrite(*stream, mRigid.angMomentum);
      stream->writeFlag(mRigid.atRest);
   }*/

   return retMask;
}

void PlayerControllerComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   /*if (stream->readFlag())
   {
      // Check if we need to jump to the given transform
      // rather than interpolate to it.
      bool forceUpdate = stream->readFlag();

      mPredictionCount = sMaxPredictionTicks;
      F32 speed = mRigid.linVelocity.len();
      mDelta.warpRot[0] = mRigid.angPosition;

      // Read in new position and momentum values
      stream->readCompressedPoint(&mRigid.linPosition);
      mathRead(*stream, &mRigid.angPosition);
      mathRead(*stream, &mRigid.linMomentum);
      mathRead(*stream, &mRigid.angMomentum);
      mRigid.atRest = stream->readFlag();
      mRigid.updateVelocity();

      if (!forceUpdate && isProperlyAdded())
      {
         // Determine number of ticks to warp based on the average
         // of the client and server velocities.
         Point3F cp = mDelta.pos + mDelta.posVec * mDelta.dt;
         mDelta.warpOffset = mRigid.linPosition - cp;

         // Calc the distance covered in one tick as the average of
         // the old speed and the new speed from the server.
         F32 dt, as = (speed + mRigid.linVelocity.len()) * 0.5 * TickSec;

         // Cal how many ticks it will take to cover the warp offset.
         // If it's less than what's left in the current tick, we'll just
         // warp in the remaining time.
         if (!as || (dt = mDelta.warpOffset.len() / as) > sMaxWarpTicks)
            dt = mDelta.dt + sMaxWarpTicks;
         else
            dt = (dt <= mDelta.dt) ? mDelta.dt : mCeil(dt - mDelta.dt) + mDelta.dt;

         // Adjust current frame interpolation
         if (mDelta.dt)
         {
            mDelta.pos = cp + (mDelta.warpOffset * (mDelta.dt / dt));
            mDelta.posVec = (cp - mDelta.pos) / mDelta.dt;
            QuatF cr;
            cr.interpolate(mDelta.rot[1], mDelta.rot[0], mDelta.dt);
            mDelta.rot[1].interpolate(cr, mRigid.angPosition, mDelta.dt / dt);
            mDelta.rot[0].extrapolate(mDelta.rot[1], cr, mDelta.dt);
         }

         // Calculated multi-tick warp
         mDelta.warpCount = 0;
         mDelta.warpTicks = (S32)(mFloor(dt));
         if (mDelta.warpTicks)
         {
            mDelta.warpOffset = mRigid.linPosition - mDelta.pos;
            mDelta.warpOffset /= mDelta.warpTicks;
            mDelta.warpRot[0] = mDelta.rot[1];
            mDelta.warpRot[1] = mRigid.angPosition;
         }
      }
      else
      {
         // Set the shape to the server position
         mDelta.dt = 0;
         mDelta.pos = mRigid.linPosition;
         mDelta.posVec.set(0, 0, 0);
         mDelta.rot[1] = mDelta.rot[0] = mRigid.angPosition;
         mDelta.warpCount = mDelta.warpTicks = 0;
         setPosition(mRigid.linPosition, mRigid.angPosition);
      }
   }*/
}

//
void PlayerControllerComponent::processTick()
{
   Parent::processTick();

   if (!isServerObject() || !isActive())
      return;

   //
   //if (mCollisionObject && !--mCollisionTimeout)
   //    mCollisionObject = 0;

   // Warp to catch up to server
   if (mDelta.warpCount < mDelta.warpTicks)
   {
      mDelta.warpCount++;

      // Set new pos.
      mDelta.pos = mOwner->getPosition();
      mDelta.pos += mDelta.warpOffset;
      mDelta.rot[0] = mDelta.rot[1];
      mDelta.rot[1].interpolate(mDelta.warpRot[0], mDelta.warpRot[1], F32(mDelta.warpCount) / mDelta.warpTicks);
      
      MatrixF trans;
      mDelta.rot[1].setMatrix(&trans);
      trans.setPosition(mDelta.pos);

      mOwner->setTransform(trans);

      // Pos backstepping
      mDelta.posVec.x = -mDelta.warpOffset.x;
      mDelta.posVec.y = -mDelta.warpOffset.y;
      mDelta.posVec.z = -mDelta.warpOffset.z;
   }
   else
   {
      // Save current rigid state interpolation
      mDelta.posVec = mOwner->getPosition();
      mDelta.rot[0] = mOwner->getTransform();

      updateMove();
      updatePos(TickSec);

      // Wrap up interpolation info
      mDelta.pos = mOwner->getPosition();
      mDelta.posVec -= mOwner->getPosition();
      mDelta.rot[1]  = mOwner->getTransform();

      // Update container database
      setTransform(mOwner->getTransform());
      
      setMaskBits(VelocityMask);
      setMaskBits(PositionMask);
      //updateContainer();
   }
}

void PlayerControllerComponent::interpolateTick(F32 dt)
{
   // Client side interpolation
   /*Point3F pos = mDelta.pos + mDelta.posVec * dt;
   MatrixF mat = mOwner->getRenderTransform();
   mat.setColumn(3, pos);
   mOwner->setRenderTransform(mat);
   mDelta.dt = dt;*/
}

void PlayerControllerComponent::ownerTransformSet(MatrixF *mat)
{
   if (mPhysicsRep)
      mPhysicsRep->setTransform(mOwner->getTransform());
}

void PlayerControllerComponent::setTransform(const MatrixF& mat)
{
   mOwner->setTransform(mat);

   //if (getPhysicsRep())
   //   getPhysicsRep()->setTransform(mOwner->getTransform());

   setMaskBits(UpdateMask);
}

//
void PlayerControllerComponent::updateMove()
{
   if (!PHYSICSMGR)
      return;

   Move *move = &mOwner->lastMove;

   //If we're not set to use mUseDirectMoveInput, then we allow for an override in the form of mInputVelocity
   if (!mUseDirectMoveInput)
   {
      move->x = mInputVelocity.x;
      move->y = mInputVelocity.y;
      move->z = mInputVelocity.z;
   }

   // Is waterCoverage high enough to be 'swimming'?
   {
      bool swimming = mOwner->getContainerInfo().waterCoverage > 0.65f/* && canSwim()*/;

      if (swimming != mSwimming)
      {
         if (!isGhost())
         {
            /*if (swimming)
               onStartSwim_callback(this);
            else
               onStopSwim_callback(this);*/
         }

         mSwimming = swimming;
      }
   }

   // Update current orientation
   bool doStandardMove = true;
   GameConnection* con = mOwner->getControllingClient();

#ifdef TORQUE_EXTENDED_MOVE
   // Work with an absolute rotation from the ExtendedMove class?
   if (con && con->getControlSchemeAbsoluteRotation())
   {
      doStandardMove = false;
      const ExtendedMove* emove = dynamic_cast<const ExtendedMove*>(move);
      U32 emoveIndex = smExtendedMoveHeadPosRotIndex;
      if (emoveIndex >= ExtendedMove::MaxPositionsRotations)
         emoveIndex = 0;

      if (emove->EulerBasedRotation[emoveIndex])
      {
         // Head pitch
         mHead.x += (emove->rotX[emoveIndex] - mLastAbsolutePitch);

         // Do we also include the relative yaw value?
         if (con->getControlSchemeAddPitchToAbsRot())
         {
            F32 x = move->pitch;
            if (x > M_PI_F)
               x -= M_2PI_F;

            mHead.x += x;
         }

         // Constrain the range of mHead.x
         while (mHead.x < -M_PI_F)
            mHead.x += M_2PI_F;
         while (mHead.x > M_PI_F)
            mHead.x -= M_2PI_F;

         // Rotate (heading) head or body?
         if (move->freeLook && ((isMounted() && getMountNode() == 0) || (con && !con->isFirstPerson())))
         {
            // Rotate head
            mHead.z += (emove->rotZ[emoveIndex] - mLastAbsoluteYaw);

            // Do we also include the relative yaw value?
            if (con->getControlSchemeAddYawToAbsRot())
            {
               F32 z = move->yaw;
               if (z > M_PI_F)
                  z -= M_2PI_F;

               mHead.z += z;
            }

            // Constrain the range of mHead.z
            while (mHead.z < 0.0f)
               mHead.z += M_2PI_F;
            while (mHead.z > M_2PI_F)
               mHead.z -= M_2PI_F;
         }
         else
         {
            // Rotate body
            mRot.z += (emove->rotZ[emoveIndex] - mLastAbsoluteYaw);

            // Do we also include the relative yaw value?
            if (con->getControlSchemeAddYawToAbsRot())
            {
               F32 z = move->yaw;
               if (z > M_PI_F)
                  z -= M_2PI_F;

               mRot.z += z;
            }

            // Constrain the range of mRot.z
            while (mRot.z < 0.0f)
               mRot.z += M_2PI_F;
            while (mRot.z > M_2PI_F)
               mRot.z -= M_2PI_F;
         }
         mLastAbsoluteYaw = emove->rotZ[emoveIndex];
         mLastAbsolutePitch = emove->rotX[emoveIndex];

         // Head bank
         mHead.y = emove->rotY[emoveIndex];

         // Constrain the range of mHead.y
         while (mHead.y > M_PI_F)
            mHead.y -= M_2PI_F;
      }
   }
#endif

   /*if (doStandardMove)
   {
      F32 p = move->pitch;
      if (p > M_PI_F)
         p -= M_2PI_F;
      mHead.x = mClampF(mHead.x + p, minLookAngle, maxLookAngle);

      F32 y = move->yaw;
      if (y > M_PI_F)
         y -= M_2PI_F;


      mRot.z += y;
      // Rotate the head back to the front, center horizontal
      // as well if we're controlling another object.
      mHead.z *= 0.5f;
      if (mControlObject)
         mHead.x *= 0.5f;

      // constrain the range of mRot.z
      while (mRot.z < 0.0f)
         mRot.z += M_2PI_F;
      while (mRot.z > M_2PI_F)
         mRot.z -= M_2PI_F;
   }

   delta.rot = mRot;
   delta.rotVec.x = delta.rotVec.y = 0.0f;
   delta.rotVec.z = prevZRot - mRot.z;
   if (delta.rotVec.z > M_PI_F)
      delta.rotVec.z -= M_2PI_F;
   else if (delta.rotVec.z < -M_PI_F)
      delta.rotVec.z += M_2PI_F;

   delta.head = mHead;
   delta.headVec -= mHead;
   for (U32 i = 0; i<3; ++i)
   {
      if (delta.headVec[i] > M_PI_F)
         delta.headVec[i] -= M_2PI_F;
      else if (delta.headVec[i] < -M_PI_F)
         delta.headVec[i] += M_2PI_F;
   }*/

   MatrixF zRot;
   zRot.set(EulerF(0.0f, 0.0f, mOwner->getRotation().asEulerF().z));

   // Desired move direction & speed
   VectorF moveVec;
   F32 moveSpeed = mInputVelocity.len();

   zRot.getColumn(0, &moveVec);
   moveVec *= move->x;
   VectorF tv;
   zRot.getColumn(1, &tv);
   moveVec += tv * move->y;

   // Acceleration due to gravity
   VectorF acc(mPhysicsWorld->getGravity() * mGravityMod * TickSec);

   // Determine ground contact normal. Only look for contacts if
   // we can move and aren't mounted.
   mContactInfo.contactNormal = VectorF::Zero;
   mContactInfo.jump = false;
   mContactInfo.run = false;

   bool jumpSurface = false, runSurface = false;
   if (!mOwner->isMounted())
      findContact(&mContactInfo.run, &mContactInfo.jump, &mContactInfo.contactNormal);
   if (mContactInfo.jump)
      mJumpSurfaceNormal = mContactInfo.contactNormal;

   // If we don't have a runSurface but we do have a contactNormal,
   // then we are standing on something that is too steep.
   // Deflect the force of gravity by the normal so we slide.
   // We could also try aligning it to the runSurface instead,
   // but this seems to work well.
   if (!mContactInfo.run && !mContactInfo.contactNormal.isZero())
      acc = (acc - 2 * mContactInfo.contactNormal * mDot(acc, mContactInfo.contactNormal));

   // Acceleration on run surface
   if (mContactInfo.run && !mSwimming)
   {
      mContactTimer = 0;

      VectorF pv = moveVec;

      // Adjust the player's requested dir. to be parallel
      // to the contact surface.
      F32 pvl = pv.len();

      // Convert to acceleration
      if (pvl)
         pv *= moveSpeed / pvl;
      VectorF runAcc = pv - (mVelocity + acc);
      F32 runSpeed = runAcc.len();

      // Clamp acceleration, player also accelerates faster when
      // in his hard landing recover state.
      F32 maxAcc;

      maxAcc = (horizMaxAccel / mMass) * TickSec;

      if (runSpeed > maxAcc)
         runAcc *= maxAcc / runSpeed;

      acc += runAcc;
   }
   else if (!mSwimming && airControl > 0.0f)
   {
      VectorF pv;
      pv = moveVec;
      F32 pvl = pv.len();

      if (pvl)
         pv *= moveSpeed / pvl;

      VectorF runAcc = pv - (mVelocity + acc);
      runAcc.z = 0;
      runAcc.x = runAcc.x * airControl;
      runAcc.y = runAcc.y * airControl;
      F32 runSpeed = runAcc.len();

      // We don't test for sprinting when performing air control
      F32 maxAcc = (horizMaxAccel / mMass) * TickSec * 0.3f;

      if (runSpeed > maxAcc)
         runAcc *= maxAcc / runSpeed;

      acc += runAcc;

      // There are no special air control animations 
      // so... increment this unless you really want to 
      // play the run anims in the air.
      mContactTimer++;
   }
   else if (mSwimming)
   {
      // Remove acc into contact surface (should only be gravity)
      // Clear out floating point acc errors, this will allow
      // the player to "rest" on the ground.
      F32 vd = -mDot(acc, mContactInfo.contactNormal);
      if (vd > 0.0f) 
      {
         VectorF dv = mContactInfo.contactNormal * (vd + 0.002f);
         acc += dv;
         if (acc.len() < 0.0001f)
            acc.set(0.0f, 0.0f, 0.0f);
      }

      // get the head pitch and add it to the moveVec
      // This more accurate swim vector calc comes from Matt Fairfax
      MatrixF xRot, zRot;
      xRot.set(EulerF(mOwner->getRotation().asEulerF().x, 0, 0));
      zRot.set(EulerF(0, 0, mOwner->getRotation().asEulerF().z));
      MatrixF rot;
      rot.mul(zRot, xRot);
      rot.getColumn(0, &moveVec);

      moveVec *= move->x;
      VectorF tv;
      rot.getColumn(1, &tv);
      moveVec += tv * move->y;
      rot.getColumn(2, &tv);
      moveVec += tv * move->z;

      // Force a 0 move if there is no energy, and only drain
      // move energy if we're moving.
      VectorF swimVec = moveVec;

      // If we are swimming but close enough to the shore/ground
      // we can still have a surface-normal. In this case align the
      // velocity to the normal to make getting out of water easier.

      moveVec.normalize();
      F32 isSwimUp = mDot(moveVec, mContactInfo.contactNormal);

      if (!mContactInfo.contactNormal.isZero() && isSwimUp < 0.1f)
      {
         F32 pvl = swimVec.len();

         if (pvl)
         {
            VectorF nn;
            mCross(swimVec, VectorF(0.0f, 0.0f, 1.0f), &nn);
            nn *= 1.0f / pvl;
            VectorF cv = mContactInfo.contactNormal;
            cv -= nn * mDot(nn, cv);
            swimVec -= cv * mDot(swimVec, cv);
         }
      }

      F32 swimVecLen = swimVec.len();

      // Convert to acceleration.
      if (swimVecLen)
         swimVec *= moveSpeed / swimVecLen;
      VectorF swimAcc = swimVec - (mVelocity + acc);
      F32 swimSpeed = swimAcc.len();

      // Clamp acceleration.
      F32 maxAcc = (horizMaxAccel / mMass) * TickSec;
      if (swimSpeed > maxAcc)
         swimAcc *= maxAcc / swimSpeed;

      acc += swimAcc;

      mContactTimer++;
   }
   else
      mContactTimer++;

   // Acceleration from Jumping
   /*if (move->trigger[sJumpTrigger] && canJump())// !isMounted() && 
   {
      // Scale the jump impulse base on maxJumpSpeed
      F32 zSpeedScale = mVelocity.z;
      if (zSpeedScale <= mDataBlock->maxJumpSpeed)
      {
         zSpeedScale = (zSpeedScale <= mDataBlock->minJumpSpeed) ? 1 :
            1 - (zSpeedScale - mDataBlock->minJumpSpeed) /
            (mDataBlock->maxJumpSpeed - mDataBlock->minJumpSpeed);

         // Desired jump direction
         VectorF pv = moveVec;
         F32 len = pv.len();
         if (len > 0)
            pv *= 1 / len;

         // We want to scale the jump size by the player size, somewhat
         // in reduced ratio so a smaller player can jump higher in
         // proportion to his size, than a larger player.
         F32 scaleZ = (getScale().z * 0.25) + 0.75;

         // Calculate our jump impulse
         F32 impulse = mDataBlock->jumpForce / getMass();

         if (mDataBlock->jumpTowardsNormal)
         {
            // If we are facing into the surface jump up, otherwise
            // jump away from surface.
            F32 dot = mDot(pv, mJumpSurfaceNormal);
            if (dot <= 0)
               acc.z += mJumpSurfaceNormal.z * scaleZ * impulse * zSpeedScale;
            else
            {
               acc.x += pv.x * impulse * dot;
               acc.y += pv.y * impulse * dot;
               acc.z += mJumpSurfaceNormal.z * scaleZ * impulse * zSpeedScale;
            }
         }
         else
            acc.z += scaleZ * impulse * zSpeedScale;

         mJumpDelay = mDataBlock->jumpDelay;
         mEnergy -= mDataBlock->jumpEnergyDrain;

         // If we don't have a StandJumpAnim, just play the JumpAnim...
         S32 seq = (mVelocity.len() < 0.5) ? PlayerData::StandJumpAnim : PlayerData::JumpAnim;
         if (mDataBlock->actionList[seq].sequence == -1)
            seq = PlayerData::JumpAnim;
         setActionThread(seq, true, false, true);

         mJumpSurfaceLastContact = JumpSkipContactsMax;
      }
   }
   else
   {
      if (jumpSurface)
      {
         if (mJumpDelay > 0)
            mJumpDelay--;
         mJumpSurfaceLastContact = 0;
      }
      else
         mJumpSurfaceLastContact++;
   }*/

   // Add in force from physical zones...
   acc += (mOwner->getContainerInfo().appliedForce / mMass) * TickSec;

   // Adjust velocity with all the move & gravity acceleration
   // TG: I forgot why doesn't the TickSec multiply happen here...
   mVelocity += acc;

   // apply horizontal air resistance

   F32 hvel = mSqrt(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y);

   if (hvel > horizResistSpeed)
   {
      F32 speedCap = hvel;
      if (speedCap > horizMaxSpeed)
         speedCap = horizMaxSpeed;
      speedCap -= horizResistFactor * TickSec * (speedCap - horizResistSpeed);
      F32 scale = speedCap / hvel;
      mVelocity.x *= scale;
      mVelocity.y *= scale;
   }
   if (mVelocity.z > upResistSpeed)
   {
      if (mVelocity.z > upMaxSpeed)
         mVelocity.z = upMaxSpeed;
      mVelocity.z -= upResistFactor * TickSec * (mVelocity.z - upResistSpeed);
   }

   // Apply drag
   //if (mSwimming)
   //   mVelocity -= mVelocity * mDrag * TickSec * (mVelocity.len() / maxUnderwaterForwardSpeed);
   //else
      mVelocity -= mVelocity * mDrag * TickSec;

   // Clamp very small velocity to zero
   if (mVelocity.isZero())
      mVelocity = Point3F::Zero;

   // If we are not touching anything and have sufficient -z vel,
   // we are falling.
   if (mContactInfo.run)
   {
      mFalling = false;
   }
   else
   {
      VectorF vel;
      mOwner->getWorldToObj().mulV(mVelocity, &vel);
      mFalling = vel.z < fallingSpeedThreshold;
   }

   // Enter/Leave Liquid
   if (!mInWater && mOwner->getContainerInfo().waterCoverage > 0.0f)
   {
      mInWater = true;

      //if (!isGhost())
     //    onEnterLiquid_callback(this, mWaterCoverage, mLiquidType.c_str());
   }
   else if (mInWater && mOwner->getContainerInfo().waterCoverage <= 0.0f)
   {
      mInWater = false;

      /*if (!isGhost())
      {
         onLeaveLiquid_callback(this, mLiquidType.c_str());
      }*/
   }
}

void PlayerControllerComponent::updatePos(const F32 travelTime)
{
   if (!PHYSICSMGR)
      return;

   PROFILE_SCOPE(PlayerControllerComponent_UpdatePos);
   //mOwner->getTransform().getColumn(3, &delta.posVec);

   // When mounted to another object, only Z rotation used.
   /*if (mOwner->isMounted()) {
      mVelocity = mMount.object->getVelocity();
      setPosition(Point3F(0.0f, 0.0f, 0.0f), mRot);
      setMaskBits(MoveMask);
      return true;
   }*/

   Point3F newPos;

   Collision col;
   dMemset(&col, 0, sizeof(col));

   // DEBUG:
   //Point3F savedVelocity = mVelocity;

   static CollisionList collisionList;
   collisionList.clear();

   newPos = mPhysicsRep->move(mVelocity * travelTime, collisionList);

   bool haveCollisions = false;
   bool wasFalling = mFalling;
   if (collisionList.getCount() > 0)
   {
      mFalling = false;
      haveCollisions = true;

      //TODO: clean this up so the phys component doesn't have to tell the col interface to do this
      CollisionInterface* colInterface = mOwner->getComponent<CollisionInterface>();
      if (colInterface)
      {
         colInterface->handleCollisionList(collisionList, mVelocity);
      }
   }

   if (haveCollisions)
   {
      // Pick the collision that most closely matches our direction
      VectorF velNormal = mVelocity;
      velNormal.normalizeSafe();
      const Collision *collision = &collisionList[0];
      F32 collisionDot = mDot(velNormal, collision->normal);
      const Collision *cp = collision + 1;
      const Collision *ep = collision + collisionList.getCount();
      for (; cp != ep; cp++)
      {
         F32 dp = mDot(velNormal, cp->normal);
         if (dp < collisionDot)
         {
            collisionDot = dp;
            collision = cp;
         }
      }

      // Modify our velocity based on collisions
      for (U32 i = 0; i<collisionList.getCount(); ++i)
      {
         F32 bd = -mDot(mVelocity, collisionList[i].normal);
         VectorF dv = collisionList[i].normal * (bd + sNormalElasticity);
         mVelocity += dv;
      }

      // Store the last collision for use later on.  The handle collision
      // code only expects a single collision object.
      if (collisionList.getCount() > 0)
         col = collisionList[collisionList.getCount() - 1];

      // We'll handle any player-to-player collision, and the last collision
      // with other obejct types.
      for (U32 i = 0; i<collisionList.getCount(); ++i)
      {
         Collision& colCheck = collisionList[i];
         if (colCheck.object)
         {
            /*SceneObject* obj = static_cast<SceneObject*>(col.object);
            if (obj->getTypeMask() & PlayerObjectType)
            {
               _handleCollision(colCheck);
            }
            else
            {*/
               col = colCheck;
            //}
         }
      }

      //_handleCollision(col);
   }

   // Set new position
   // If on the client, calc delta for backstepping
   /*if (isClientObject())
   {
      delta.pos = newPos;
      delta.posVec = delta.posVec - delta.pos;
      delta.dt = 1.0f;
   }*/

   MatrixF newMat;
   newMat.setPosition(newPos);
   mPhysicsRep->setTransform(newMat);

   mOwner->setPosition(newPos);

   //setPosition(newPos, mRot);
   //setMaskBits(MoveMask);
   //updateContainer();

   /*if (!isGhost())
   {
      // Collisions are only queued on the server and can be
      // generated by either updateMove or updatePos
      notifyCollision();

      // Do mission area callbacks on the server as well
      checkMissionArea();
   }*/

   // Check the total distance moved.  If it is more than 1000th of the velocity, then
   //  we moved a fair amount...
   //if (totalMotion >= (0.001f * initialSpeed))
   //return true;
   //else
   //return false;
}
//
void PlayerControllerComponent::setVelocity(const VectorF& vel)
{
   mVelocity = vel;

   // Clamp against the maximum velocity.
   if (mMaxVelocity > 0)
   {
      F32 len = mVelocity.magnitudeSafe();
      if (len > mMaxVelocity)
      {
         Point3F excess = mVelocity * (1.0f - (mMaxVelocity / len));
         mVelocity -= excess;
      }
   }

   setMaskBits(VelocityMask);
}

void PlayerControllerComponent::findContact(bool *run, bool *jump, VectorF *contactNormal)
{
   SceneObject *contactObject = NULL;

   Vector<SceneObject*> overlapObjects;

   mPhysicsRep->findContact(&contactObject, contactNormal, &overlapObjects);

   F32 vd = (*contactNormal).z;
   *run = vd > mCos(mDegToRad(moveSurfaceAngle));
   *jump = vd > mCos(mDegToRad(contactSurfaceAngle));

   // Check for triggers
   for (U32 i = 0; i < overlapObjects.size(); i++)
   {
      SceneObject *obj = overlapObjects[i];
      U32 objectMask = obj->getTypeMask();

      // Check: triggers, corpses and items...
      //
      if (objectMask & TriggerObjectType)
      {
         if (Trigger* pTrigger = dynamic_cast<Trigger*>(obj))
         {
            pTrigger->potentialEnterObject(mOwner);
         }
         else if (TriggerExample* pTriggerEx = dynamic_cast<TriggerExample*>(obj))
         {
            if (pTriggerEx)
               pTriggerEx->potentialEnterObject(mOwner);
         }
         else if (CollisionTrigger* pTriggerEx = dynamic_cast<CollisionTrigger*>(obj))
         {
            if (pTriggerEx)
               pTriggerEx->potentialEnterObject(mOwner);
         }
      }
   }

   /*vd = (*contactNormal).z;
   *run = vd > mCos(mDegToRad(runSurfaceAngle));
   *jump = true;

   mContactInfo.clear();*/

   mContactInfo.contacted = contactObject != NULL;
   mContactInfo.contactObject = contactObject;

   if (mContactInfo.contacted)
      mContactInfo.contactNormal = *contactNormal;

   //mContactInfo.run = *run;
   //mContactInfo.jump = *jump;

   /*mContactInfo.clear();

   mContactInfo.contacted = contactObject != NULL;
   mContactInfo.contactObject = contactObject;

   if (mContactInfo.contacted)
      mContactInfo.contactNormal = *contactNormal;

   mContactInfo.run = *run;
   mContactInfo.jump = *jump;*/
}

void PlayerControllerComponent::applyImpulse(const Point3F &pos, const VectorF &vec)
{

   AssertFatal(!mIsNaN(vec), "Player::applyImpulse() - The vector is NaN!");

   // Players ignore angular velocity
   VectorF vel;
   vel.x = vec.x / getMass();
   vel.y = vec.y / getMass();
   vel.z = vec.z / getMass();

   // Make sure the impulse isn't too bigg
   F32 len = vel.magnitudeSafe();
   if (len > sMaxImpulseVelocity)
   {
      Point3F excess = vel * (1.0f - (sMaxImpulseVelocity / len));
      vel -= excess;
   }

   setVelocity(mVelocity + vel);
}

DefineEngineMethod(PlayerControllerComponent, applyImpulse, bool, (Point3F pos, VectorF vel), ,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   object->applyImpulse(pos, vel);
   return true;
}

DefineEngineMethod(PlayerControllerComponent, getContactNormal, Point3F, (), ,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   return object->getContactNormal();
}

DefineEngineMethod(PlayerControllerComponent, getContactObject, SceneObject*, (), ,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   return object->getContactObject();
}

DefineEngineMethod(PlayerControllerComponent, isContacted, bool, (), ,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   return object->isContacted();
}