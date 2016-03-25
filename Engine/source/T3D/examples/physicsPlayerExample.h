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

#ifndef _PhysicsPlayerExample_H_
#define _PhysicsPlayerExample_H_

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
class PhysicsPlayer;
class PhysicsWorld;

//-----------------------------------------------------------------------------
// This class implements a basic SceneObject that can exist in the world at a
// 3D position and render itself. Note that PhysicsPlayerExample handles its own
// rendering by submitting itself as an ObjectRenderInst (see
// renderInstance\renderPassmanager.h) along with a delegate for its render()
// function. However, the preffered rendering method in the engine is to submit
// a MeshRenderInst along with a Material, vertex buffer, primitive buffer, and
// transform and allow the RenderMeshMgr handle the actual rendering. You can
// see this implemented in RenderMeshExample.
//-----------------------------------------------------------------------------

class PhysicsPlayerExample : public GameBase
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
   PhysicsPlayer *mPhysicsRep;

   PhysicsWorld *mWorld;

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

   F32 mDrag;

   VectorF mMoveSpeed;
   S32 mContactTimer;               ///< Ticks since last contact

   F32 horizMaxSpeed;         ///< Max speed attainable in the horizontal
   F32 horizResistSpeed;      ///< Speed at which resistance will take place
   F32 horizResistFactor;     ///< Factor of resistance once horizResistSpeed has been reached

   F32 upMaxSpeed;            ///< Max vertical speed attainable
   F32 upResistSpeed;         ///< Speed at which resistance will take place
   F32 upResistFactor;        ///< Factor of resistance once upResistSpeed has been reached

   F32 runSurfaceAngle;
   F32 maxStepHeight;

   VectorF mVelocity;               ///< Velocity

   struct ContactInfo
   {
      bool contacted, jump, run;
      SceneObject *contactObject;
      VectorF  contactNormal;

      void clear()
      {
         contacted = jump = run = false;
         contactObject = NULL;
         contactNormal.set(1, 1, 1);
      }

      ContactInfo() { clear(); }

   } mContactInfo;

	Point3F oldPos;
	VectorF oldVel;
	S32 updateCounter;

public:
   PhysicsPlayerExample();
   virtual ~PhysicsPlayerExample();

   // Declare this object as a ConsoleObject so that we can
   // instantiate it into the world and network it
   DECLARE_CONOBJECT(PhysicsPlayerExample);

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

   //player stuff
   enum PublicConstants {
      CollisionTimeoutValue = 250      ///< Timeout in ms.
   };

   struct CollisionTimeout
   {
      CollisionTimeout* next;
      SceneObject* object;
      U32 objectNumber;
      SimTime expireTime;
      VectorF vector;
   };
   CollisionTimeout* mTimeoutList;
   static CollisionTimeout* sFreeTimeoutList;

   void updateMove();
   void updatePos(const F32 travelTime = TickSec);
   void findContact(bool *run, bool *jump, VectorF *contactNormal);
   void _handleCollision(const Collision &collision);
   void queueCollision(SceneObject *obj, const VectorF &vec);
   void notifyCollision();
   void onCollision(SceneObject *object, const VectorF &vec);
};

#endif // _PhysicsPlayerExample_H_