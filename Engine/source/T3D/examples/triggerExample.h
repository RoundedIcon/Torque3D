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

#ifndef _TriggerExample_H_
#define _TriggerExample_H_

#ifndef _SCENEOBJECT_H_
#include "scene/sceneObject.h"
#endif
#ifndef _GAMEBASE_H_
#include "T3D/gameBase/gameBase.h"
#endif
#ifndef _GFXSTATEBLOCK_H_
#include "gfx/gfxStateBlock.h"
#endif
#ifndef _GFXVERTEXBUFFER_H_
#include "gfx/gfxVertexBuffer.h"
#endif
#ifndef _GFXPRIMITIVEBUFFER_H_
#include "gfx/gfxPrimitiveBuffer.h"
#endif
#ifndef _T3D_PHYSICSCOMMON_H_
#include "T3D/physics/physicsCommon.h"
#endif

class BaseMatInstance;
class PhysicsBody;
class PhysicsWorld;

//-----------------------------------------------------------------------------
// This class implements a basic SceneObject that can exist in the world at a
// 3D position and render itself. Note that TriggerExample handles its own
// rendering by submitting itself as an ObjectRenderInst (see
// renderInstance\renderPassmanager.h) along with a delegate for its render()
// function. However, the preffered rendering method in the engine is to submit
// a MeshRenderInst along with a Material, vertex buffer, primitive buffer, and
// transform and allow the RenderMeshMgr handle the actual rendering. You can
// see this implemented in RenderMeshExample.
//-----------------------------------------------------------------------------

class TriggerExample : public SceneObject
{
   typedef SceneObject Parent;

   // Networking masks
   // We need to implement at least one of these to allow
   // the client version of the object to receive updates
   // from the server version (like if it has been moved
   // or edited)
   enum MaskBits
   {
      TransformMask = Parent::NextFreeMask << 0,
      NextFreeMask = Parent::NextFreeMask << 1
   };

   //--------------------------------------------------------------------------
   // Rendering variables
   //--------------------------------------------------------------------------
   // Define our vertex format here so we don't have to
   // change it in multiple spots later
   typedef GFXVertexPCN VertexType;

   // The handles for our StateBlocks
   GFXStateBlockRef mNormalSB;
   GFXStateBlockRef mReflectSB;

   // The GFX vertex and primitive buffers
   GFXVertexBufferHandle< VertexType > mVertexBuffer;

   /// The abstracted physics actor.
   PhysicsBody *mPhysicsRep;

   PhysicsWorld *mWorld;

   Vector<GameBase*> mObjects;
   U32               mLastThink;
   U32               mCurrTick;

   String            mEnterCommand;
   String            mLeaveCommand;
   String            mTickCommand;

   static const U32 CMD_SIZE = 1024;

   S32  tickPeriodMS;

public:
   TriggerExample();
   virtual ~TriggerExample();

   // Declare this object as a ConsoleObject so that we can
   // instantiate it into the world and network it
   DECLARE_CONOBJECT(TriggerExample);

   //--------------------------------------------------------------------------
   // Object Editing
   // Since there is always a server and a client object in Torque and we
   // actually edit the server object we need to implement some basic
   // networking functions
   //--------------------------------------------------------------------------
   // Set up any fields that we want to be editable (like position)
   static void initPersistFields();

   void inspectPostApply();

   // Handle when we are added to the scene and removed from the scene
   bool onAdd();
   void onRemove();

   // Override this so that we can dirty the network flag when it is called
   void setTransform(const MatrixF &mat);

   // This function handles sending the relevant data from the server
   // object to the client object
   U32 packUpdate(NetConnection *conn, U32 mask, BitStream *stream);
   // This function handles receiving relevant data from the server
   // object and applying it to the client object
   void unpackUpdate(NetConnection *conn, BitStream *stream);

   //--------------------------------------------------------------------------
   // Object Rendering
   // Torque utilizes a "batch" rendering system. This means that it builds a
   // list of objects that need to render (via RenderInst's) and then renders
   // them all in one batch. This allows it to optimized on things like
   // minimizing texture, state, and shader switching by grouping objects that
   // use the same Materials. For this example, however, we are just going to
   // get this object added to the list of objects that handle their own
   // rendering.
   //--------------------------------------------------------------------------
   // Create the geometry for rendering
   void createGeometry();

   // This is the function that allows this object to submit itself for rendering
   void prepRenderImage(SceneRenderState *state);

   // This is the function that actually gets called to do the rendering
   // Note that there is no longer a predefined name for this function.
   // Instead, when we submit our ObjectRenderInst in prepRenderImage(),
   // we bind this function as our rendering delegate function
   void render(ObjectRenderInst *ri, SceneRenderState *state, BaseMatInstance *overrideMat);

   //
   void interpolateTick(F32 delta);
   void processTick(const Move *move);

   //Physics
   void setupPhysics();

   //trigger
   static bool setEnterCmd(void *object, const char *index, const char *data);
   static bool setLeaveCmd(void *object, const char *index, const char *data);
   static bool setTickCmd(void *object, const char *index, const char *data);

   bool testObject(GameBase* enter);

   void      potentialEnterObject(GameBase*);
   U32       getNumTriggeringObjects() const;
   GameBase* getObject(const U32);
   const Vector<GameBase*>& getObjects() const { return mObjects; }
};


inline U32 TriggerExample::getNumTriggeringObjects() const
{
   return mObjects.size();
}

inline GameBase* TriggerExample::getObject(const U32 index)
{
   AssertFatal(index < getNumTriggeringObjects(), "Error, out of range object index");

   return mObjects[index];
}

#endif // _TriggerExample_H_