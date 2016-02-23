//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "T3D/examples/PhysicsPlayerExample.h"

#include "math/mathIO.h"
#include "scene/sceneRenderState.h"
#include "core/stream/bitStream.h"
#include "materials/sceneData.h"
#include "gfx/gfxDebugEvent.h"
#include "gfx/gfxTransformSaver.h"
#include "renderInstance/renderPassManager.h"

#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsWorld.h"
#include "T3D/physics/physicsCollision.h"

#include "T3D/containerQuery.h"
#include "platform/platform.h"

#include "console/consoleTypes.h"

#include "T3D/shapeBase.h"
#include "T3D/trigger.h"

#include "T3D/physics/physicsPlayer.h"

#include "console/engineAPI.h"

#include "T3D/examples/TriggerExample.h"
#include "T3D/examples/physicsExample.h"

#include "T3D/tagLibrary.h"

bool PhysicsPlayerExample::smNoCorrections = false;
bool PhysicsPlayerExample::smNoSmoothing = false;

static U32 sCollisionMoveMask = TerrainObjectType |
WaterObjectType |
PlayerObjectType |
StaticShapeObjectType |
VehicleObjectType |
PhysicalZoneObjectType;

static U32 sServerCollisionContactMask = sCollisionMoveMask |
ItemObjectType |
TriggerObjectType |
CorpseObjectType |
DynamicShapeObjectType;

static U32 sClientCollisionContactMask = sCollisionMoveMask |
TriggerObjectType;

Chunker<PhysicsPlayerExample::CollisionTimeout> sTimeoutChunker;
PhysicsPlayerExample::CollisionTimeout* PhysicsPlayerExample::sFreeTimeoutList = 0;

static F32 sNormalElasticity = 0.01f;
static F32 sMaxImpulseVelocity = 200.0f;

IMPLEMENT_CO_NETOBJECT_V1(PhysicsPlayerExample);

ConsoleDocClass(PhysicsPlayerExample,
   "@brief An example scene object which renders using a callback.\n\n"
   "This class implements a basic SceneObject that can exist in the world at a "
   "3D position and render itself. Note that PhysicsPlayerExample handles its own "
   "rendering by submitting itself as an ObjectRenderInst (see "
   "renderInstance\renderPassmanager.h) along with a delegate for its render() "
   "function. However, the preffered rendering method in the engine is to submit "
   "a MeshRenderInst along with a Material, vertex buffer, primitive buffer, and "
   "transform and allow the RenderMeshMgr handle the actual rendering. You can "
   "see this implemented in RenderMeshExample.\n\n"
   "See the C++ code for implementation details.\n\n"
   "@ingroup Examples\n");

//-----------------------------------------------------------------------------
// Object setup and teardown
//-----------------------------------------------------------------------------
PhysicsPlayerExample::PhysicsPlayerExample()
{
   // Flag this object so that it will always
   // be sent across the network to clients
   mNetFlags.set(Ghostable | ScopeAlways);

   // Set it as a "static" object
   //mTypeMask |= TriggerObjectType;
   mTypeMask |= DynamicShapeObjectType;

   mPhysicsRep = NULL;

   mass = 20;
   
   mDrag = 0;
   mContactTimer = 0;
   horizMaxSpeed = 80.0f;
   horizResistSpeed = 38.0f;
   horizResistFactor = 1.0f;

   upMaxSpeed = 80.0f;
   upResistSpeed = 38.0f;
   upResistFactor = 1.0f;

   mMoveSpeed = VectorF::Zero;

   isTrigger = false;

   runSurfaceAngle = 45;
   maxStepHeight = 1;

   mVelocity = VectorF::Zero;

   mTimeoutList = NULL;
}

PhysicsPlayerExample::~PhysicsPlayerExample()
{
   SAFE_DELETE(mPhysicsRep);
}

//-----------------------------------------------------------------------------
// Object Editing
//-----------------------------------------------------------------------------
void PhysicsPlayerExample::initPersistFields()
{
   // SceneObject already handles exposing the transform
   Parent::initPersistFields();

   addField("mass", TypeF32, Offset(mass, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");

   addField("drag", TypeF32, Offset(mDrag, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");

   addField("moveVector", TypePoint3F, Offset(mMoveSpeed, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");

   addField("horizMaxSpeed", TypeF32, Offset(horizMaxSpeed, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("horizResistSpeed", TypeF32, Offset(horizResistSpeed, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("horizResistFactor", TypeF32, Offset(horizResistFactor, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");

   addField("upMaxSpeed", TypeF32, Offset(upMaxSpeed, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("upResistSpeed", TypeF32, Offset(upResistSpeed, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("upResistFactor", TypeF32, Offset(upResistFactor, PhysicsPlayerExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
}

void PhysicsPlayerExample::inspectPostApply()
{
   // SceneObject already handles exposing the transform
   Parent::inspectPostApply();

   setupPhysics();
}

bool PhysicsPlayerExample::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Set up a 1x1x1 bounding box
   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
      Point3F(0.5f, 0.5f, 0.5f));

   resetWorldBox();

   // Add this object to the scene
   addToScene();

   setupPhysics();

   return true;
}

void PhysicsPlayerExample::onRemove()
{
   // Remove this object from the scene
   removeFromScene();

   Parent::onRemove();
}

void PhysicsPlayerExample::setupPhysics()
{
   SAFE_DELETE(mPhysicsRep);

   if (!PHYSICSMGR)
      return;

   mWorld = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

   Point3F boxSize = mObjBox.getExtents() * mObjScale;

   F32 runSurfaceCos = mCos(mDegToRad(runSurfaceAngle));

   mPhysicsRep = PHYSICSMGR->createPlayer();
   mPhysicsRep->init("", boxSize, runSurfaceCos, maxStepHeight, this, mWorld);
   mPhysicsRep->setTransform(getTransform());

}

void PhysicsPlayerExample::setTransform(const MatrixF & mat)
{
   // Let SceneObject handle all of the matrix manipulation
   Parent::setTransform(mat);

   mState.position = getPosition();
   mState.orientation.set(mat);
   mRenderState[0] = mRenderState[1] = mState;
   setMaskBits(StateMask);

   if (mPhysicsRep)
      mPhysicsRep->setTransform(mat);

   // Dirty our network mask so that the new transform gets
   // transmitted to the client object
   setMaskBits(TransformMask);
}

U32 PhysicsPlayerExample::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   // Write our transform information
   if (stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   /*if (stream->writeFlag(mask & StateMask))
   {
      // This will encode the position relative to the control
      // object position.  
      //
      // This will compress the position to as little as 6.25
      // bytes if the position is within about 30 meters of the
      // control object.
      //
      // Worst case its a full 12 bytes + 2 bits if the position
      // is more than 500 meters from the control object.
      //
      stream->writeCompressedPoint(mState.position);

      // Use only 3.5 bytes to send the orientation.
      stream->writeQuat(mState.orientation, 9);

      // If the server object has been set to sleep then
      // we don't need to send any velocity.
      if (!stream->writeFlag(mState.sleeping))
      {
         // This gives me ~0.015f resolution in velocity magnitude
         // while only costing me 1 bit of the velocity is zero length,
         // <5 bytes in normal cases, and <8 bytes if the velocity is
         // greater than 1000.
         AssertWarn(mState.linVelocity.len() < 1000.0f,
            "PhysicsShape::packUpdate - The linVelocity is out of range!");
         stream->writeVector(mState.linVelocity, 1000.0f, 16, 9);

         // For angular velocity we get < 0.01f resolution in magnitude
         // with the most common case being under 4 bytes.
         AssertWarn(mState.angVelocity.len() < 10.0f,
            "PhysicsShape::packUpdate - The angVelocity is out of range!");
         stream->writeVector(mState.angVelocity, 10.0f, 10, 9);
      }
   }*/

   return retMask;
}

void PhysicsPlayerExample::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())  // TransformMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);
   }

   /*if (stream->readFlag()) // StateMask
   {
      PhysicsState state;

      // Read the encoded and compressed position... commonly only 6.25 bytes.
      stream->readCompressedPoint(&state.position);

      // Read the compressed quaternion... 3.5 bytes.
      stream->readQuat(&state.orientation, 9);

      state.sleeping = stream->readFlag();
      if (!state.sleeping)
      {
         stream->readVector(&state.linVelocity, 1000.0f, 16, 9);
         stream->readVector(&state.angVelocity, 10.0f, 10, 9);
      }

      if (!smNoCorrections && mPhysicsRep && mPhysicsRep->isDynamic())
      {
         // Set the new state on the physics object immediately.
         mPhysicsRep->applyCorrection(state.getTransform());

         mPhysicsRep->setSleeping(state.sleeping);
         if (!state.sleeping)
         {
            mPhysicsRep->setLinVelocity(state.linVelocity);
            mPhysicsRep->setAngVelocity(state.angVelocity);
         }

         mPhysicsRep->getState(&mState);
      }

      // If there is no physics object then just set the
      // new state... the tick will take care of the 
      // interpolation and extrapolation.
      if (!mPhysicsRep || !mPhysicsRep->isDynamic())
         mState = state;
   }*/
}

void PhysicsPlayerExample::interpolateTick(F32 delta)
{
   // Client side interpolation
   Parent::interpolateTick(delta);
}

void PhysicsPlayerExample::processTick(const Move *move)
{
   AssertFatal(mPhysicsRep, "PhysicsShape::processTick - Shouldn't be processing a destroyed shape!");

   // Note that unlike TSStatic, the serverside PhysicsShape does not
   // need to play the ambient animation because even if the animation were
   // to move collision shapes it would not affect the physx representation.

   if (!mWorld->isEnabled())
   {
      mVelocity = VectorF::Zero;
      return;
   }

   updateMove();

   F32 physicsTimescale = PHYSICSMGR->getTimeScale();
   updatePos(TickSec * physicsTimescale);
}

void PhysicsPlayerExample::updateMove()
{
   if (!mWorld || !mPhysicsRep)
      return;

   // Acceleration due to gravity
   VectorF acc(mWorld->getGravity() * TickSec);

   // Determine ground contact normal. Only look for contacts if
   // we can move and aren't mounted.
   VectorF contactNormal(0, 0, 0);
   bool jumpSurface = false, runSurface = false;
   if (!isMounted())
      findContact(&runSurface, &jumpSurface, &contactNormal);

   // If we don't have a runSurface but we do have a contactNormal,
   // then we are standing on something that is too steep.
   // Deflect the force of gravity by the normal so we slide.
   // We could also try aligning it to the runSurface instead,
   // but this seems to work well.
   if (!runSurface && !contactNormal.isZero())
      acc = (acc - 2 * contactNormal * mDot(acc, contactNormal));

   // Acceleration on run surface
   if (runSurface) 
   {
      mContactTimer = 0;

      // Force a 0 move if there is no energy, and only drain
      // move energy if we're moving.
      /*VectorF pv;
      
      pv.set(0.0f, 0.0f, 0.0f);

      // Adjust the player's requested dir. to be parallel
      // to the contact surface.
      F32 pvl = pv.len();

      // Convert to acceleration
      if (pvl)
         pv *= mMoveSpeed / pvl;*/

      VectorF runAcc = mMoveSpeed - (mVelocity + acc);
      F32 runSpeed = runAcc.len();

      // Clamp acceleration, player also accelerates faster when
      // in his hard landing recover state.
      F32 maxAcc = 100;

      if (runSpeed > maxAcc)
         runAcc *= maxAcc / runSpeed;

      acc += runAcc;
   }
   else
      mContactTimer++;

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

   mVelocity -= mVelocity * mDrag * TickSec;

   // Clamp very small velocity to zero
   if (mVelocity.isZero())
      mVelocity = Point3F::Zero;
}

void PhysicsPlayerExample::updatePos(const F32 travelTime)
{
   Point3F newPos;

   Collision col;
   dMemset(&col, 0, sizeof(col));

   // DEBUG:
   //Point3F savedVelocity = mVelocity;

   if (mPhysicsRep)
   {
      static CollisionList collisionList;
      collisionList.clear();

      newPos = mPhysicsRep->move(mVelocity * travelTime, collisionList);

      bool haveCollisions = false;
      //bool wasFalling = mFalling;
      if (collisionList.getCount() > 0)
      {
         //mFalling = false;
         haveCollisions = true;
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
               SceneObject* obj = static_cast<SceneObject*>(col.object);
               if (obj->getTypeMask() & PlayerObjectType)
               {
                  _handleCollision(colCheck);
               }
               else
               {
                  col = colCheck;
               }
            }
         }

         _handleCollision(col);
      }
   }

   //updateContainer();

   setPosition(newPos);

   if (!isGhost())
   {
      // Collisions are only queued on the server and can be
      // generated by either updateMove or updatePos
      notifyCollision();
   }

}

//
void PhysicsPlayerExample::findContact(bool *run, bool *jump, VectorF *contactNormal)
{
   SceneObject *contactObject = NULL;

   Vector<SceneObject*> overlapObjects;

   mPhysicsRep->findContact(&contactObject, contactNormal, &overlapObjects);

   // Check for triggers, corpses and items.
   const U32 filterMask = isGhost() ? sClientCollisionContactMask : sServerCollisionContactMask;
   for (U32 i = 0; i < overlapObjects.size(); i++)
   {
      SceneObject *obj = overlapObjects[i];
      U32 objectMask = obj->getTypeMask();

      if (!(objectMask & filterMask))
         continue;

      // Check: triggers, corpses and items...
      //
      if (objectMask & TriggerObjectType)
      {
         Trigger* pTrigger = dynamic_cast<Trigger*>(obj);
         if (pTrigger)
            pTrigger->potentialEnterObject(this);
         else
         {
            TriggerExample* pTriggerEx = dynamic_cast<TriggerExample*>(obj);
            if (pTriggerEx)
               pTriggerEx->potentialEnterObject(this);
         }
      }
      else if (objectMask & CorpseObjectType)
      {
         // If we've overlapped the worldbounding boxes, then that's it...
         if (getWorldBox().isOverlapped(obj->getWorldBox()))
         {
            ShapeBase* col = static_cast<ShapeBase*>(obj);
            queueCollision(col, getVelocity() - col->getVelocity());
         }
      }
      else if (objectMask & DynamicShapeObjectType)
      {
         PhysicsExample* physExample = dynamic_cast<PhysicsExample*>(obj);
         if (physExample)
         {
            if(TAGLIBRARY->doesEntityHaveTag(obj, "Panda"))
               Con::printf("PHYSICS PLAYER EXAMPLE: COLLIDED WITH A PHYSICS EXAMPLE RIGID BODY!");
         }
      }
      /*else if (objectMask & ItemObjectType)
      {
         // If we've overlapped the worldbounding boxes, then that's it...
         Item* item = static_cast<Item*>(obj);
         if (getWorldBox().isOverlapped(item->getWorldBox()) &&
            item->getCollisionObject() != this &&
            !item->isHidden())
            queueCollision(item, getVelocity() - item->getVelocity());
      }*/
   }

   F32 vd = (*contactNormal).z;
   *run = vd > mCos(mDegToRad(runSurfaceAngle));
   *jump = true;

   mContactInfo.clear();

   mContactInfo.contacted = contactObject != NULL;
   mContactInfo.contactObject = contactObject;

   if (mContactInfo.contacted)
      mContactInfo.contactNormal = *contactNormal;

   mContactInfo.run = *run;
   mContactInfo.jump = *jump;
}

void PhysicsPlayerExample::_handleCollision(const Collision &collision)
{
   // Track collisions
   if (!isGhost() &&
      collision.object &&
      collision.object != mContactInfo.contactObject)
      queueCollision(collision.object, mVelocity - collision.object->getVelocity());
}

void PhysicsPlayerExample::queueCollision(SceneObject *obj, const VectorF &vec)
{
   // Add object to list of collisions.
   SimTime time = Sim::getCurrentTime();
   S32 num = obj->getId();

   CollisionTimeout** adr = &mTimeoutList;
   CollisionTimeout* ptr = mTimeoutList;
   while (ptr) {
      if (ptr->objectNumber == num) {
         if (ptr->expireTime < time) {
            ptr->expireTime = time + CollisionTimeoutValue;
            ptr->object = obj;
            ptr->vector = vec;
         }
         return;
      }
      // Recover expired entries
      if (ptr->expireTime < time) {
         CollisionTimeout* cur = ptr;
         *adr = ptr->next;
         ptr = ptr->next;
         cur->next = sFreeTimeoutList;
         sFreeTimeoutList = cur;
      }
      else {
         adr = &ptr->next;
         ptr = ptr->next;
      }
   }

   // New entry for the object
   if (sFreeTimeoutList != NULL)
   {
      ptr = sFreeTimeoutList;
      sFreeTimeoutList = ptr->next;
      ptr->next = NULL;
   }
   else
   {
      ptr = sTimeoutChunker.alloc();
   }

   ptr->object = obj;
   ptr->objectNumber = obj->getId();
   ptr->vector = vec;
   ptr->expireTime = time + CollisionTimeoutValue;
   ptr->next = mTimeoutList;

   mTimeoutList = ptr;
}

void PhysicsPlayerExample::notifyCollision()
{
   // Notify all the objects that were just stamped during the queueing
   // process.
   SimTime expireTime = Sim::getCurrentTime() + CollisionTimeoutValue;
   for (CollisionTimeout* ptr = mTimeoutList; ptr; ptr = ptr->next)
   {
      if (ptr->expireTime == expireTime && ptr->object)
      {
         SimObjectPtr<SceneObject> safePtr(ptr->object);
         //SimObjectPtr<ShapeBase> safeThis(this);
         onCollision(ptr->object, ptr->vector);
         ptr->object = 0;

         //if (!bool(safeThis))
         //   return;

         if (bool(safePtr))
            safePtr->onCollision(this, ptr->vector);

         //if (!bool(safeThis))
         //   return;
      }
   }
}

void PhysicsPlayerExample::onCollision(SceneObject *object, const VectorF &vec)
{
   //if (!isGhost())
   //   mDataBlock->onCollision_callback(this, object, vec, vec.len());
}

//
void PhysicsPlayerExample::applyImpulse(const Point3F &pos, const VectorF &vec)
{
   AssertFatal(!mIsNaN(vec), "Player::applyImpulse() - The vector is NaN!");

   // Players ignore angular velocity
   VectorF vel;
   vel.x = vec.x / mass;
   vel.y = vec.y / mass;
   vel.z = vec.z / mass;

   // Make sure the impulse isn't too big
   F32 len = vel.magnitudeSafe();
   if (len > sMaxImpulseVelocity)
   {
      Point3F excess = vel * (1.0f - (sMaxImpulseVelocity / len));
      vel -= excess;
   }

   mVelocity += vel;
}

//-----------------------------------------------------------------------------
// Object Rendering
//-----------------------------------------------------------------------------
void PhysicsPlayerExample::createGeometry()
{
   static const Point3F cubePoints[8] =
   {
      Point3F(1.0f, -1.0f, -1.0f), Point3F(1.0f, -1.0f, 1.0f),
      Point3F(1.0f, 1.0f, -1.0f), Point3F(1.0f, 1.0f, 1.0f),
      Point3F(-1.0f, -1.0f, -1.0f), Point3F(-1.0f, 1.0f, -1.0f),
      Point3F(-1.0f, -1.0f, 1.0f), Point3F(-1.0f, 1.0f, 1.0f)
   };

   static const Point3F cubeNormals[6] =
   {
      Point3F(1.0f, 0.0f, 0.0f), Point3F(-1.0f, 0.0f, 0.0f),
      Point3F(0.0f, 1.0f, 0.0f), Point3F(0.0f, -1.0f, 0.0f),
      Point3F(0.0f, 0.0f, 1.0f), Point3F(0.0f, 0.0f, -1.0f)
   };

   static const ColorI cubeColors[3] =
   {
      ColorI(255, 0, 0, 255),
      ColorI(0, 255, 0, 255),
      ColorI(0, 0, 255, 255)
   };

   static const U32 cubeFaces[36][3] =
   {
      { 3, 0, 0 }, { 0, 0, 0 }, { 1, 0, 0 },
      { 2, 0, 0 }, { 0, 0, 0 }, { 3, 0, 0 },
      { 7, 1, 0 }, { 4, 1, 0 }, { 5, 1, 0 },
      { 6, 1, 0 }, { 4, 1, 0 }, { 7, 1, 0 },
      { 3, 2, 1 }, { 5, 2, 1 }, { 2, 2, 1 },
      { 7, 2, 1 }, { 5, 2, 1 }, { 3, 2, 1 },
      { 1, 3, 1 }, { 4, 3, 1 }, { 6, 3, 1 },
      { 0, 3, 1 }, { 4, 3, 1 }, { 1, 3, 1 },
      { 3, 4, 2 }, { 6, 4, 2 }, { 7, 4, 2 },
      { 1, 4, 2 }, { 6, 4, 2 }, { 3, 4, 2 },
      { 2, 5, 2 }, { 4, 5, 2 }, { 0, 5, 2 },
      { 5, 5, 2 }, { 4, 5, 2 }, { 2, 5, 2 }
   };

   // Fill the vertex buffer
   VertexType *pVert = NULL;

   mVertexBuffer.set(GFX, 36, GFXBufferTypeStatic);
   pVert = mVertexBuffer.lock();

   Point3F halfSize = getObjBox().getExtents() * 0.5f;

   for (U32 i = 0; i < 36; i++)
   {
      const U32& vdx = cubeFaces[i][0];
      const U32& ndx = cubeFaces[i][1];
      const U32& cdx = cubeFaces[i][2];

      pVert[i].point = cubePoints[vdx] * halfSize;
      pVert[i].normal = cubeNormals[ndx];
      pVert[i].color = cubeColors[cdx];
   }

   mVertexBuffer.unlock();

   // Set up our normal and reflection StateBlocks
   GFXStateBlockDesc desc;

   // The normal StateBlock only needs a default StateBlock
   mNormalSB = GFX->createStateBlock(desc);

   // The reflection needs its culling reversed
   desc.cullDefined = true;
   desc.cullMode = GFXCullCW;
   mReflectSB = GFX->createStateBlock(desc);
}

void PhysicsPlayerExample::prepRenderImage(SceneRenderState *state)
{
   // Do a little prep work if needed
   if (mVertexBuffer.isNull())
      createGeometry();

   // Allocate an ObjectRenderInst so that we can submit it to the RenderPassManager
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();

   // Now bind our rendering function so that it will get called
   ri->renderDelegate.bind(this, &PhysicsPlayerExample::render);

   // Set our RenderInst as a standard object render
   ri->type = RenderPassManager::RIT_Object;

   // Set our sorting keys to a default value
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;

   // Submit our RenderInst to the RenderPassManager
   state->getRenderPass()->addInst(ri);
}

void PhysicsPlayerExample::render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if (overrideMat)
      return;

   if (mVertexBuffer.isNull())
      return;

   PROFILE_SCOPE(PhysicsPlayerExample_Render);

   // Set up a GFX debug event (this helps with debugging rendering events in external tools)
   GFXDEBUGEVENT_SCOPE(PhysicsPlayerExample_Render, ColorI::RED);

   // GFXTransformSaver is a handy helper class that restores
   // the current GFX matrices to their original values when
   // it goes out of scope at the end of the function
   GFXTransformSaver saver;

   // Calculate our object to world transform matrix
   MatrixF objectToWorld = getRenderTransform();
   objectToWorld.scale(getScale());

   // Apply our object transform
   GFX->multWorld(objectToWorld);

   // Deal with reflect pass otherwise
   // set the normal StateBlock
   if (state->isReflectPass())
      GFX->setStateBlock(mReflectSB);
   else
      GFX->setStateBlock(mNormalSB);

   // Set up the "generic" shaders
   // These handle rendering on GFX layers that don't support
   // fixed function. Otherwise they disable shaders.
   GFX->setupGenericShaders(GFXDevice::GSModColorTexture);

   // Set the vertex buffer
   GFX->setVertexBuffer(mVertexBuffer);

   // Draw our triangles
   GFX->drawPrimitive(GFXTriangleList, 0, 12);
}

DefineEngineMethod(PhysicsPlayerExample, applyImpulse, bool, (Point3F pos, VectorF vel), ,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   object->applyImpulse(pos, vel);
   return true;
}