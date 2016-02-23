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

#include "T3D/examples/TriggerExample.h"

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

IMPLEMENT_CO_NETOBJECT_V1(TriggerExample);

ConsoleDocClass(TriggerExample,
   "@brief An example scene object which renders using a callback.\n\n"
   "This class implements a basic SceneObject that can exist in the world at a "
   "3D position and render itself. Note that TriggerExample handles its own "
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
TriggerExample::TriggerExample()
{
   // Flag this object so that it will always
   // be sent across the network to clients
   mNetFlags.set(Ghostable | ScopeAlways);

   // Set it as a "static" object
   //mTypeMask |= TriggerObjectType;
   mTypeMask |= TriggerObjectType;

   mPhysicsRep = NULL;

   mLastThink = 0;
   mCurrTick = 0;

   tickPeriodMS = 100;
}

TriggerExample::~TriggerExample()
{
   SAFE_DELETE(mPhysicsRep);
}

//-----------------------------------------------------------------------------
// Object Editing
//-----------------------------------------------------------------------------
void TriggerExample::initPersistFields()
{
   // SceneObject already handles exposing the transform
   Parent::initPersistFields();

   addProtectedField("enterCommand", TypeCommand, Offset(mEnterCommand, TriggerExample), &setEnterCmd, &defaultProtectedGetFn,
      "The command to execute when an object enters this trigger. Object id stored in %%obj. Maximum 1023 characters.");
   addProtectedField("leaveCommand", TypeCommand, Offset(mLeaveCommand, TriggerExample), &setLeaveCmd, &defaultProtectedGetFn,
      "The command to execute when an object leaves this trigger. Object id stored in %%obj. Maximum 1023 characters.");
   addProtectedField("tickCommand", TypeCommand, Offset(mTickCommand, TriggerExample), &setTickCmd, &defaultProtectedGetFn,
      "The command to execute while an object is inside this trigger. Maximum 1023 characters.");
}

void TriggerExample::inspectPostApply()
{
   // SceneObject already handles exposing the transform
   Parent::inspectPostApply();

   setupPhysics();
}

bool TriggerExample::setEnterCmd(void *object, const char *index, const char *data)
{
   //static_cast<Trigger*>(object)->setMaskBits(EnterCmdMask);
   return true; // to update the actual field
}

bool TriggerExample::setLeaveCmd(void *object, const char *index, const char *data)
{
   //static_cast<Trigger*>(object)->setMaskBits(LeaveCmdMask);
   return true; // to update the actual field
}

bool TriggerExample::setTickCmd(void *object, const char *index, const char *data)
{
   //static_cast<Trigger*>(object)->setMaskBits(TickCmdMask);
   return true; // to update the actual field
}

bool TriggerExample::onAdd()
{
   if (!Parent::onAdd())
      return false;

   // Set up a 1x1x1 bounding box
   mObjBox.set(Point3F(-0.5f, -0.5f, -0.5f),
      Point3F(0.5f, 0.5f, 0.5f));

   resetWorldBox();

   // Add this object to the scene
   addToScene();

   setProcessTick(true);

   setupPhysics();

   return true;
}

void TriggerExample::onRemove()
{
   // Remove this object from the scene
   removeFromScene();

   Parent::onRemove();
}

void TriggerExample::setupPhysics()
{
   SAFE_DELETE(mPhysicsRep);

   if (!PHYSICSMGR)
      return;

   PhysicsCollision *colShape = PHYSICSMGR->createCollision();

   MatrixF colMat(true);
   //colMat.displace(Point3F(0, 0, mObjBox.getExtents().z * 0.5f * mObjScale.z));

   colShape->addBox(mObjBox.getExtents() * 0.5f * mObjScale, colMat);

   PhysicsWorld *world = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");
   mPhysicsRep = PHYSICSMGR->createBody();
   mPhysicsRep->init(colShape, 0, PhysicsBody::BF_TRIGGER | PhysicsBody::BF_KINEMATIC, this, world);
   mPhysicsRep->setTransform(getTransform());
}

void TriggerExample::setTransform(const MatrixF & mat)
{
   // Let SceneObject handle all of the matrix manipulation
   Parent::setTransform(mat);

   if (mPhysicsRep)
      mPhysicsRep->setTransform(mat);

   // Dirty our network mask so that the new transform gets
   // transmitted to the client object
   setMaskBits(TransformMask);
}

U32 TriggerExample::packUpdate(NetConnection *conn, U32 mask, BitStream *stream)
{
   // Allow the Parent to get a crack at writing its info
   U32 retMask = Parent::packUpdate(conn, mask, stream);

   // Write our transform information
   if (stream->writeFlag(mask & TransformMask))
   {
      mathWrite(*stream, getTransform());
      mathWrite(*stream, getScale());
   }

   return retMask;
}

void TriggerExample::unpackUpdate(NetConnection *conn, BitStream *stream)
{
   // Let the Parent read any info it sent
   Parent::unpackUpdate(conn, stream);

   if (stream->readFlag())  // TransformMask
   {
      mathRead(*stream, &mObjToWorld);
      mathRead(*stream, &mObjScale);

      setTransform(mObjToWorld);
   }
}

void TriggerExample::interpolateTick(F32 delta)
{
   Parent::interpolateTick(delta);
}

void TriggerExample::processTick(const Move *move)
{
   Parent::processTick(move);

   //
   if (mObjects.size() == 0)
      return;

   if (mLastThink + tickPeriodMS < mCurrTick)
   {
      mCurrTick = 0;
      mLastThink = 0;

      for (S32 i = S32(mObjects.size() - 1); i >= 0; i--)
      {
         if (testObject(mObjects[i]) == false)
         {
            GameBase* remove = mObjects[i];
            mObjects.erase(i);
            clearNotify(remove);

            if (!mLeaveCommand.isEmpty())
            {
               String command = String("%obj = ") + remove->getIdString() + ";" + mLeaveCommand;
               Con::evaluate(command.c_str());
            }
         }
      }

      if (!mTickCommand.isEmpty())
         Con::evaluate(mTickCommand.c_str());
   }
   else
   {
      mCurrTick += TickMs;
   }
}

bool TriggerExample::testObject(GameBase* enter)
{
   return mWorldBox.isOverlapped(enter->getWorldBox());
}


void TriggerExample::potentialEnterObject(GameBase* enter)
{
   for (U32 i = 0; i < mObjects.size(); i++) {
      if (mObjects[i] == enter)
         return;
   }

   if (testObject(enter) == true) {
      mObjects.push_back(enter);
      deleteNotify(enter);

      if (!mEnterCommand.isEmpty())
      {
         String command = String("%obj = ") + enter->getIdString() + ";" + mEnterCommand;
         Con::evaluate(command.c_str());
      }
   }
}

//-----------------------------------------------------------------------------
// Object Rendering
//-----------------------------------------------------------------------------
void TriggerExample::createGeometry()
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

void TriggerExample::prepRenderImage(SceneRenderState *state)
{
   // Do a little prep work if needed
   if (mVertexBuffer.isNull())
      createGeometry();

   // Allocate an ObjectRenderInst so that we can submit it to the RenderPassManager
   ObjectRenderInst *ri = state->getRenderPass()->allocInst<ObjectRenderInst>();

   // Now bind our rendering function so that it will get called
   ri->renderDelegate.bind(this, &TriggerExample::render);

   // Set our RenderInst as a standard object render
   ri->type = RenderPassManager::RIT_Object;

   // Set our sorting keys to a default value
   ri->defaultKey = 0;
   ri->defaultKey2 = 0;

   // Submit our RenderInst to the RenderPassManager
   state->getRenderPass()->addInst(ri);
}

void TriggerExample::render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat)
{
   if (overrideMat)
      return;

   if (mVertexBuffer.isNull())
      return;

   PROFILE_SCOPE(TriggerExample_Render);

   // Set up a GFX debug event (this helps with debugging rendering events in external tools)
   GFXDEBUGEVENT_SCOPE(TriggerExample_Render, ColorI::RED);

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