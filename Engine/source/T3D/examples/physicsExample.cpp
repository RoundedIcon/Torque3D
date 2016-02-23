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

#include "T3D/examples/PhysicsExample.h"

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

#include "console/engineAPI.h"

#include "T3D/tagLibrary.h"

#include "T3D/examples/TriggerExample.h"
#include "T3D/trigger.h"

bool PhysicsExample::smNoCorrections = false;
bool PhysicsExample::smNoSmoothing = false;

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

IMPLEMENT_CO_NETOBJECT_V1(PhysicsExample);

ConsoleDocClass(PhysicsExample,
   "@brief An example scene object which renders using a callback.\n\n"
   "This class implements a basic SceneObject that can exist in the world at a "
   "3D position and render itself. Note that PhysicsExample handles its own "
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
PhysicsExample::PhysicsExample()
{
   // Flag this object so that it will always
   // be sent across the network to clients
   mNetFlags.set(Ghostable | ScopeAlways);

   simType = SimType_ClientServer;

   // Set it as a "static" object
   //mTypeMask |= TriggerObjectType;
   mTypeMask |= DynamicShapeObjectType;

   mPhysicsRep = NULL;

   mass = 20;
   restitution = 10;
   dynamicFriction = 1;
   staticFriction = 0.1;
   linearDamping = 0;
   angularDamping = 0;
   linearSleepThreshold = 1;
   angularSleepThreshold = 1;
   buoyancyDensity = 0.1;
   waterDampingScale = 1;

   isTrigger = false;

   mResetPos = MatrixF::Identity;
}

PhysicsExample::~PhysicsExample()
{
   SAFE_DELETE(mPhysicsRep);
}

//-----------------------------------------------------------------------------
// Object Editing
//-----------------------------------------------------------------------------
void PhysicsExample::initPersistFields()
{
   // SceneObject already handles exposing the transform
   Parent::initPersistFields();

   addField("mass", TypeF32, Offset(mass, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("restitution", TypeF32, Offset(restitution, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("dynamicFriction", TypeF32, Offset(dynamicFriction, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("staticFriction", TypeF32, Offset(staticFriction, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("linearDamping", TypeF32, Offset(linearDamping, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("angularDamping", TypeF32, Offset(angularDamping, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("linearSleepThreshold", TypeF32, Offset(linearSleepThreshold, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");

   addField("buoyancyDensity", TypeF32, Offset(buoyancyDensity, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
   addField("waterDampingScale", TypeF32, Offset(waterDampingScale, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");

   addField("isTrigger", TypeBool, Offset(isTrigger, PhysicsExample),
      "@brief Path to the .DAE or .DTS file to use for this shape.\n\n"
      "Compatable with Live-Asset Reloading. ");
}

void PhysicsExample::inspectPostApply()
{
   // SceneObject already handles exposing the transform
   Parent::inspectPostApply();

   setupPhysics();
}

bool PhysicsExample::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Set up a 1x1x1 bounding box
   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
      Point3F(0.5f, 0.5f, 0.5f));

   resetWorldBox();

   // Add this object to the scene
   addToScene();

   TAGLIBRARY->addEntity(this);

   TAGLIBRARY->addTagToEntity(this, "Box");

   setProcessTick(true);

   setupPhysics();

   return true;
}

void PhysicsExample::onRemove()
{
   // Remove this object from the scene
   removeFromScene();

   TAGLIBRARY->removeEntity(this);

   if (isServerObject())
   {
      PhysicsPlugin::getPhysicsResetSignal().remove(this, &PhysicsExample::_onPhysicsReset);
   }

   Parent::onRemove();
}

void PhysicsExample::setupPhysics()
{
   SAFE_DELETE(mPhysicsRep);

   if (!PHYSICSMGR)
      return;

   mWorld = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

   PhysicsCollision *colShape = PHYSICSMGR->createCollision();

   MatrixF colMat(true);
   //colMat.displace(Point3F(0, 0, mObjBox.getExtents().z * 0.5f * mObjScale.z));

   colShape->addBox(mObjBox.getExtents() * 0.5f * mObjScale, colMat);

   mPhysicsRep = PHYSICSMGR->createBody();

   if (isTrigger)
   {
      setProcessTick(false);

      mPhysicsRep->init(colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, mWorld);
   }
   else
   {
      setProcessTick(true);

      mPhysicsRep->init(colShape, mass, 0, this, mWorld);

      mPhysicsRep->setMaterial(restitution, dynamicFriction, staticFriction);

      mPhysicsRep->setDamping(linearDamping, angularDamping);
      mPhysicsRep->setSleepThreshold(linearSleepThreshold, angularSleepThreshold);
   }

   mPhysicsRep->setTransform(getTransform());

   // The reset position is the transform on the server
   // at creation time... its not used on the client.
   if (isServerObject())
   {
      storeRestorePos();
      PhysicsPlugin::getPhysicsResetSignal().notify(this, &PhysicsExample::_onPhysicsReset);
   }
}

void PhysicsExample::_onPhysicsReset(PhysicsResetEvent reset)
{
   if (reset == PhysicsResetEvent_Store)
      mResetPos = getTransform();

   else if (reset == PhysicsResetEvent_Restore)
   {
      setTransform(mResetPos);

      // Restore to un-destroyed state.
      restore();

      // Cheat and reset the client from here.
      if (getClientObject())
      {
         PhysicsExample *clientObj = (PhysicsExample*)getClientObject();
         clientObj->setTransform(mResetPos);
         clientObj->restore();
      }
   }
}

void PhysicsExample::storeRestorePos()
{
   mResetPos = getTransform();
}

void PhysicsExample::restore()
{
   // Restore tick processing, add it back to 
   // the scene, and enable collision and simulation.
   if (isTrigger)
      setProcessTick(false);
   else
      setProcessTick(true);

   if (isClientObject())
      addToScene();

   mPhysicsRep->setSimulationEnabled(true);
}

void PhysicsExample::setTransform(const MatrixF & mat)
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

U32 PhysicsExample::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   // Write our transform information
   if (stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   if (stream->writeFlag(mask & StateMask))
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
   }

   return retMask;
}

void PhysicsExample::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())  // TransformMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);
   }

   if (stream->readFlag()) // StateMask
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
   }
}

void PhysicsExample::interpolateTick(F32 delta)
{
   if (!mPhysicsRep->isDynamic())
      return;

   // Interpolate the position and rotation based on the delta.
   PhysicsState state;
   state.interpolate(mRenderState[1], mRenderState[0], delta);

   // Set the transform to the interpolated transform.
   setRenderTransform(state.getTransform());
}

void PhysicsExample::processTick(const Move *move)
{
   AssertFatal(mPhysicsRep, "PhysicsShape::processTick - Shouldn't be processing a destroyed shape!");

   // Note that unlike TSStatic, the serverside PhysicsShape does not
   // need to play the ambient animation because even if the animation were
   // to move collision shapes it would not affect the physx representation.

   if (!mPhysicsRep->isDynamic())
      return;

   // SINGLE PLAYER HACK!!!!
   if (PHYSICSMGR->isSinglePlayer() && isClientObject() && getServerObject())
   {
      PhysicsExample *servObj = (PhysicsExample*)getServerObject();
      setTransform(servObj->mState.getTransform());
      mRenderState[0] = servObj->mRenderState[0];
      mRenderState[1] = servObj->mRenderState[1];

      return;
   }

   // Store the last render state.
   mRenderState[0] = mRenderState[1];

   // If the last render state doesn't match the last simulation 
   // state then we got a correction and need to 
   Point3F errorDelta = mRenderState[1].position - mState.position;
   const bool doSmoothing = !errorDelta.isZero() && !smNoSmoothing;

   const bool wasSleeping = mState.sleeping;

   // Get the new physics state.
   mPhysicsRep->getState(&mState);
   _updateContainerForces();

   // Smooth the correction back into the render state.
   mRenderState[1] = mState;
   if (doSmoothing)
   {
      F32 correction = mClampF(errorDelta.len() / 20.0f, 0.1f, 0.9f);
      mRenderState[1].position.interpolate(mState.position, mRenderState[0].position, correction);
      mRenderState[1].orientation.interpolate(mState.orientation, mRenderState[0].orientation, correction);
   }

   //Check if any collisions occured
   findContact();

   // If we haven't been sleeping then update our transform
   // and set ourselves as dirty for the next client update.
   if (!wasSleeping || !mState.sleeping)
   {
      // Set the transform on the parent so that
      // the physics object isn't moved.
      Parent::setTransform(mState.getTransform());

      // If we're doing server simulation then we need
      // to send the client a state update.
      if (isServerObject() && mPhysicsRep && !smNoCorrections &&
         !PHYSICSMGR->isSinglePlayer() // SINGLE PLAYER HACK!!!!
         )
         setMaskBits(StateMask);
   }
}

void PhysicsExample::findContact()
{
   SceneObject *contactObject = NULL;

   VectorF *contactNormal = new VectorF(0, 0, 0);

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

      int tmp = 0;

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
         int n = 0;
         // If we've overlapped the worldbounding boxes, then that's it...
         /*if (getWorldBox().isOverlapped(obj->getWorldBox()))
         {
            ShapeBase* col = static_cast<ShapeBase*>(obj);
            queueCollision(col, getVelocity() - col->getVelocity());
         }*/
      }
      else if (objectMask & DynamicShapeObjectType)
      {
         int n = 0;
         /*PhysicsExample* physExample = dynamic_cast<PhysicsExample*>(obj);
         if (physExample)
         {
            if (TAGLIBRARY->doesEntityHaveTag(obj, "Panda"))
               Con::printf("PHYSICS PLAYER EXAMPLE: COLLIDED WITH A PHYSICS EXAMPLE RIGID BODY!");
         }*/
      }
   }

   mContactInfo.clear();

   mContactInfo.contacted = contactObject != NULL;
   mContactInfo.contactObject = contactObject;

   if (mContactInfo.contacted)
      mContactInfo.contactNormal = *contactNormal;
}

void PhysicsExample::_updateContainerForces()
{
   PROFILE_SCOPE(PhysicsShape_updateContainerForces);

   // If we're not simulating don't update forces.
   if (!mWorld->isEnabled())
      return;

   ContainerQueryInfo info;
   info.box = getWorldBox();
   info.mass = mass;

   // Find and retreive physics info from intersecting WaterObject(s)
   getContainer()->findObjects(getWorldBox(), WaterObjectType | PhysicalZoneObjectType, findRouter, &info);

   // Calculate buoyancy and drag
   F32 angDrag = angularDamping;
   F32 linDrag = linearDamping;
   F32 buoyancy = 0.0f;
   Point3F cmass = mPhysicsRep->getCMassPosition();

   F32 density = buoyancyDensity;
   if (density > 0.0f)
   {
      if (info.waterCoverage > 0.0f)
      {
         F32 waterDragScale = info.waterViscosity * waterDampingScale;
         F32 powCoverage = mPow(info.waterCoverage, 0.25f);

         angDrag = mLerp(angDrag, angDrag * waterDragScale, powCoverage);
         linDrag = mLerp(linDrag, linDrag * waterDragScale, powCoverage);
      }

      buoyancy = (info.waterDensity / density) * mPow(info.waterCoverage, 2.0f);

      // A little hackery to prevent oscillation
      // Based on this blog post:
      // (http://reinot.blogspot.com/2005/11/oh-yes-they-float-georgie-they-all.html)
      // JCF: disabled!
      Point3F buoyancyForce = buoyancy * -mWorld->getGravity() * TickSec * mass;
      mPhysicsRep->applyImpulse(cmass, buoyancyForce);
   }

   // Update the dampening as the container might have changed.
   mPhysicsRep->setDamping(linDrag, angDrag);

   // Apply physical zone forces.
   if (!info.appliedForce.isZero())
      mPhysicsRep->applyImpulse(cmass, info.appliedForce);
}

void PhysicsExample::applyImpulse(const Point3F &pos, const VectorF &vec)
{
   if (mPhysicsRep && mPhysicsRep->isDynamic())
      mPhysicsRep->applyImpulse(pos, vec);
}

//-----------------------------------------------------------------------------
// Object Rendering
//-----------------------------------------------------------------------------
void PhysicsExample::createGeometry()
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

void PhysicsExample::prepRenderImage(SceneRenderState *state)
{
   // Do a little prep work if needed
   if (mVertexBuffer.isNull())
      createGeometry();

   // Allocate an ObjectRenderInst so that we can submit it to the RenderPassManager
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();

   // Now bind our rendering function so that it will get called
   ri->renderDelegate.bind(this, &PhysicsExample::render);

   // Set our RenderInst as a standard object render
   ri->type = RenderPassManager::RIT_Object;

   // Set our sorting keys to a default value
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;

   // Submit our RenderInst to the RenderPassManager
   state->getRenderPass()->addInst(ri);
}

void PhysicsExample::render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if (overrideMat)
      return;

   if (mVertexBuffer.isNull())
      return;

   PROFILE_SCOPE(PhysicsExample_Render);

   // Set up a GFX debug event (this helps with debugging rendering events in external tools)
   GFXDEBUGEVENT_SCOPE(PhysicsExample_Render, ColorI::RED);

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

DefineEngineMethod(PhysicsExample, applyImpulse, bool, (Point3F pos, VectorF vel), ,
   "@brief Apply an impulse to this object as defined by a world position and velocity vector.\n\n"

   "@param pos impulse world position\n"
   "@param vel impulse velocity (impulse force F = m * v)\n"
   "@return Always true\n"

   "@note Not all objects that derrive from GameBase have this defined.\n")
{
   object->applyImpulse(pos, vel);
   return true;
}