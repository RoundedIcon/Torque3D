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

#ifndef _PhysicsExample_H_
#define _PhysicsExample_H_

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
// 3D position and render itself. Note that PhysicsExample handles its own
// rendering by submitting itself as an ObjectRenderInst (see
// renderInstance\renderPassmanager.h) along with a delegate for its render()
// function. However, the preffered rendering method in the engine is to submit
// a MeshRenderInst along with a Material, vertex buffer, primitive buffer, and
// transform and allow the RenderMeshMgr handle the actual rendering. You can
// see this implemented in RenderMeshExample.
//-----------------------------------------------------------------------------

class PhysicsExample : public GameBase
{
   typedef GameBase Parent;

   // Networking masks
   // We need to implement at least one of these to allow
   // the client version of the object to receive updates
   // from the server version (like if it has been moved
   // or edited)
   enum MaskBits
   {
      TransformMask = Parent::NextFreeMask << 0,
      StateMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2
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

   /// The current physics state.
   PhysicsState mState;

   /// The previous and current render states.
   PhysicsState mRenderState[2];

   /// The abstracted physics actor.
   PhysicsBody *mPhysicsRep;

   PhysicsWorld *mWorld;

   /// The starting position to place the shape when
   /// the level begins or is reset.
   MatrixF mResetPos;

   bool isTrigger;

   /// If true then no corrections are sent from the server 
   /// and/or applied from the client.
   ///
   /// This is only ment for debugging.
   ///
   static bool smNoCorrections;

   /// If true then no smoothing is done on the client when
   /// applying server corrections.
   ///
   /// This is only ment for debugging.
   ///
   static bool smNoSmoothing;

   ///
   F32 mass;

   /// 
   F32 dynamicFriction;

   /// 
   F32 staticFriction;

   ///
   F32 restitution;

   ///
   F32 linearDamping;

   ///
   F32 angularDamping;

   /// 
   F32 linearSleepThreshold;

   ///
   F32 angularSleepThreshold;

   // A scale applied to the normal linear and angular damping
   // when the object enters a water volume.
   F32 waterDampingScale;

   // The density of this object used for water buoyancy effects.
   F32 buoyancyDensity;

   enum SimType
   {
      /// This physics representation only exists on the client
      /// world and the server only does ghosting.
      SimType_ClientOnly,

      /// The physics representation only exists on the server world
      /// and the client gets delta updates for rendering.
      SimType_ServerOnly,

      /// The physics representation exists on the client and the server
      /// worlds with corrections occuring when the client gets out of sync.
      SimType_ClientServer,

      /// The bits used to pack the SimType field.
      SimType_Bits = 3,

   } simType;

   struct ContactInfo
   {
      bool contacted;
      SceneObject *contactObject;
      VectorF  contactNormal;

      void clear()
      {
         contacted = false;
         contactObject = NULL;
         contactNormal.set(1, 1, 1);
      }

      ContactInfo() { clear(); }

   } mContactInfo;

public:
   PhysicsExample();
   virtual ~PhysicsExample();

   // Declare this object as a ConsoleObject so that we can
   // instantiate it into the world and network it
   DECLARE_CONOBJECT(PhysicsExample);

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

   void _updateContainerForces();

   //Physics
   void setupPhysics();
   void applyImpulse(const Point3F &pos, const VectorF &vec);
   void storeRestorePos();
   void restore();

   void _applyCorrection(const MatrixF &mat);
   void _onPhysicsReset(PhysicsResetEvent reset);

   void findContact();
};

#endif // _PhysicsExample_H_