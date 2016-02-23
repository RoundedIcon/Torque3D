//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/Components/Collision/collisionComponent.h"
#include "T3D/Components/Collision/collisionComponent_ScriptBinding.h"
#include "T3D/Components/Physics/physicsBehavior.h"

//#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
//#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
//#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
#include "scene/sceneRenderState.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDrawUtil.h"
//#include "gfx/primBuilder.h"
#include "console/engineAPI.h"
//#include "lighting/lightQuery.h"
#include "T3D/physics/physicsPlugin.h"
#include "T3D/physics/physicsBody.h"
#include "T3D/physics/physicsCollision.h"
#include "T3D/gameBase/gameConnection.h"
#include "collision/extrudedPolyList.h"
#include "math/mathIO.h"
#include "gfx/sim/debugDraw.h"  

#include "collision/concretePolyList.h"

#include "T3D/trigger.h"

#include "opcode/Opcode.h"
#include "opcode/Ice/IceAABB.h"
#include "opcode/Ice/IcePoint.h"
#include "opcode/OPC_AABBTree.h"
#include "opcode/OPC_AABBCollider.h"

#include "math/mathUtils.h"

#include "materials/baseMatInstance.h"

#include "collision/vertexPolyList.h"

//#include "T3D/Components/Render/animatedMesh.h"

extern bool gEditingMission;

static bool sRenderColliders = false;

//Docs
ConsoleDocClass(CollisionComponent,
   "@brief The Box Collider component uses a box or rectangular convex shape for collisions.\n\n"

   "Colliders are individualized components that are similarly based off the CollisionInterface core.\n"
   "They are basically the entire functionality of how Torque handles collisions compacted into a single component.\n"
   "A collider will both collide against and be collided with, other entities.\n"
   "Individual colliders will offer different shapes. This box collider will generate a box/rectangle convex, \n"
   "while the mesh collider will take the owner Entity's rendered shape and do polysoup collision on it, etc.\n\n"

   "The general flow of operations for how collisions happen is thus:\n"
   "  -When the component is added(or updated) prepCollision() is called.\n"
   "    This will set up our initial convex shape for usage later.\n\n"

   "  -When we update via processTick(), we first test if our entity owner is mobile.\n"
   "    If our owner isn't mobile(as in, they have no components that provide it a velocity to move)\n"
   "    then we skip doing our active collision checks. Collisions are checked by the things moving, as\n"
   "    opposed to being reactionary. If we're moving, we call updateWorkingCollisionSet().\n"
   "    updateWorkingCollisionSet() estimates our bounding space for our current ticket based on our position and velocity.\n"
   "    If our bounding space has changed since the last tick, we proceed to call updateWorkingList() on our convex.\n"
   "    This notifies any object in the bounding space that they may be collided with, so they will call buildConvex().\n"
   "    buildConvex() will set up our ConvexList with our collision convex info.\n\n"

   "  -When the component that is actually causing our movement, such as SimplePhysicsBehavior, updates, it will check collisions.\n"
   "    It will call checkCollisions() on us. checkCollisions() will first build a bounding shape for our convex, and test\n"
   "    if we can early out because we won't hit anything based on our starting point, velocity, and tick time.\n"
   "    If we don't early out, we proceed to call updateCollisions(). This builds an ExtrudePolyList, which is then extruded\n"
   "    based on our velocity. We then test our extruded polies on our working list of objects we build\n"
   "    up earlier via updateWorkingCollisionSet. Any collisions that happen here will be added to our mCollisionList.\n"
   "    Finally, we call handleCollisionList() on our collisionList, which then queues out the colliison notice\n"
   "    to the object(s) we collided with so they can do callbacks and the like. We also report back on if we did collide\n"
   "    to the physics component via our bool return in checkCollisions() so it can make the physics react accordingly.\n\n"

   "One interesting point to note is the usage of mBlockColliding.\n"
   "This is set so that it dictates the return on checkCollisions(). If set to false, it will ensure checkCollisions()\n"
   "will return false, regardless if we actually collided. This is useful, because even if checkCollisions() returns false,\n"
   "we still handle the collisions so the callbacks happen. This enables us to apply a collider to an object that doesn't block\n"
   "objects, but does have callbacks, so it can act as a trigger, allowing for arbitrarily shaped triggers, as any collider can\n"
   "act as a trigger volume(including MeshCollider).\n\n"

   "@tsexample\n"
   "new CollisionComponentInstance()\n"
   "{\n"
   "   template = CollisionComponentTemplate;\n"
   "   colliderSize = \"1 1 2\";\n"
   "   blockColldingObject = \"1\";\n"
   "};\n"
   "@endtsexample\n"

   "@see SimplePhysicsBehavior\n"
   "@ingroup Collision\n"
   "@ingroup Components\n"
   );
//Docs

/////////////////////////////////////////////////////////////////////////
ImplementEnumType(CollisionMeshMeshType,
   "Type of mesh data available in a shape.\n"
   "@ingroup gameObjects")
{ CollisionComponent::None, "None", "No mesh data." },
{ CollisionComponent::Bounds, "Bounds", "Bounding box of the shape." },
{ CollisionComponent::CollisionMesh, "Collision Mesh", "Specifically desingated \"collision\" meshes." },
{ CollisionComponent::VisibleMesh, "Visible Mesh", "Rendered mesh polygons." },
EndImplementEnumType;

//
CollisionComponent::CollisionComponent() : Component()
{
   mNetFlags.set(Ghostable | ScopeAlways);

   mFriendlyName = "Collision(Component)";

   mOwnerRenderInterface = NULL;
   mOwnerPhysicsInterface = NULL;

   mBlockColliding = true;

   mCollisionType = CollisionMesh;
   mLOSType = CollisionMesh;
   mDecalType = CollisionMesh;

   colisionMeshPrefix = StringTable->insert("Collision");

   CollisionMoveMask = (TerrainObjectType | PlayerObjectType |
      StaticShapeObjectType | VehicleObjectType |
      VehicleBlockerObjectType | DynamicShapeObjectType | StaticObjectType | EntityObjectType | TriggerObjectType);

   mPhysicsRep = NULL;
   mPhysicsWorld = NULL;

   mTimeoutList = NULL;
}

CollisionComponent::~CollisionComponent()
{
   for (S32 i = 0; i < mFields.size(); ++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(CollisionComponent);

void CollisionComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   RenderComponentInterface *renderInterface = mOwner->getComponent<RenderComponentInterface>();
   if (renderInterface)
   {
      renderInterface->onShapeInstanceChanged.notify(this, &CollisionComponent::targetShapeChanged);
      mOwnerRenderInterface = renderInterface;
   }

   //physicsInterface
   PhysicsComponentInterface *physicsInterface = mOwner->getComponent<PhysicsComponentInterface>();
   if (!physicsInterface)
   {
      mPhysicsRep = PHYSICSMGR->createBody();
   }

   prepCollision();

   //updatePhysics();
}

void CollisionComponent::onComponentRemove()
{
   SAFE_DELETE(mPhysicsRep);

   Parent::onComponentRemove();
}

void CollisionComponent::componentAddedToOwner(Component *comp)
{
   if (comp->getId() == getId())
      return;

   //test if this is a shape component!
   RenderComponentInterface *renderInterface = dynamic_cast<RenderComponentInterface*>(comp);
   if (renderInterface)
   {
      renderInterface->onShapeInstanceChanged.notify(this, &CollisionComponent::targetShapeChanged);
      mOwnerRenderInterface = renderInterface;
      prepCollision();
   }

   PhysicsComponentInterface *physicsInterface = dynamic_cast<PhysicsComponentInterface*>(comp);
   if (physicsInterface)
   {
      if (mPhysicsRep)
         SAFE_DELETE(mPhysicsRep);

      prepCollision();
   }
}

void CollisionComponent::componentRemovedFromOwner(Component *comp)
{
   if (comp->getId() == getId()) //?????????
      return;

   //test if this is a shape component!
   RenderComponentInterface *renderInterface = dynamic_cast<RenderComponentInterface*>(comp);
   if (renderInterface)
   {
      renderInterface->onShapeInstanceChanged.remove(this, &CollisionComponent::targetShapeChanged);
      mOwnerRenderInterface = NULL;
      prepCollision();
   }

   //physicsInterface
   PhysicsComponentInterface *physicsInterface = dynamic_cast<PhysicsComponentInterface*>(comp);
   if (physicsInterface)
   {
      mPhysicsRep = PHYSICSMGR->createBody();

      prepCollision();
   }
}

void CollisionComponent::checkDependencies()
{
}

void CollisionComponent::initPersistFields()
{
   Parent::initPersistFields();

   addGroup("Collision");

      addField("CollisionType", TypeCollisionMeshMeshType, Offset(mCollisionType, CollisionComponent),
         "The type of mesh data to use for collision queries.");

      addField("LineOfSightType", TypeCollisionMeshMeshType, Offset(mLOSType, CollisionComponent),
         "The type of mesh data to use for collision queries.");

      addField("DecalType", TypeCollisionMeshMeshType, Offset(mDecalType, CollisionComponent),
         "The type of mesh data to use for collision queries.");

      addField("CollisionMeshPrefix", TypeString, Offset(colisionMeshPrefix, CollisionComponent),
         "The type of mesh data to use for collision queries.");

      addField("BlockCollisions", TypeBool, Offset(mBlockColliding, CollisionComponent), "");

   endGroup("Collision");
}

void CollisionComponent::inspectPostApply()
{
   // Apply any transformations set in the editor
   Parent::inspectPostApply();

   if (isServerObject())
   {
      setMaskBits(ColliderMask);
      prepCollision();
   }
}

U32 CollisionComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & (ColliderMask | InitialUpdateMask)))
   {
      stream->write((U32)mCollisionType);
      stream->writeString(colisionMeshPrefix);
   }

   return retMask;
}

void CollisionComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag()) // UpdateMask
   {
      U32 collisionType = CollisionMesh;

      stream->read(&collisionType);

      // Handle it if we have changed CollisionType's
      if ((MeshType)collisionType != mCollisionType)
      {
         mCollisionType = (MeshType)collisionType;

         prepCollision();
      }

      char readBuffer[1024];

      stream->readString(readBuffer);
      colisionMeshPrefix = StringTable->insert(readBuffer);
   }
}

void CollisionComponent::ownerTransformSet(MatrixF *mat)
{
   if (mPhysicsRep)
      mPhysicsRep->setTransform(mOwner->getTransform());
}

void CollisionComponent::targetShapeChanged(RenderComponentInterface* instanceInterface)
{
   prepCollision();
}

void CollisionComponent::prepCollision()
{
   if (!mOwner)
      return;

   // Let the client know that the collision was updated
   setMaskBits(ColliderMask);

   mOwner->disableCollision();

   if ((!PHYSICSMGR || mCollisionType == None) ||
      (mOwnerRenderInterface == NULL && (mCollisionType == CollisionMesh || mCollisionType == VisibleMesh)))
      return;

   PhysicsCollision *colShape = NULL;

   if (mCollisionType == Bounds)
   {
      MatrixF offset(true);

      if (mOwnerRenderInterface && mOwnerRenderInterface->getShape())
         offset.setPosition(mOwnerRenderInterface->getShape()->center);

      colShape = PHYSICSMGR->createCollision();
      colShape->addBox(mOwner->getObjBox().getExtents() * 0.5f * mOwner->getScale(), offset);
   }
   else if (mCollisionType == CollisionMesh || (mCollisionType == VisibleMesh /*&& !mOwner->getComponent<AnimatedMesh>()*/))
   {
      colShape = buildColShapes();
   }

   if (colShape)
   {
      mPhysicsWorld = PHYSICSMGR->getWorld(isServerObject() ? "server" : "client");

      if (mPhysicsRep)
      {
         if (mBlockColliding)
            mPhysicsRep->init(colShape, 0, 0, mOwner, mPhysicsWorld);
         else
            mPhysicsRep->init(colShape, 0, PhysicsBody::BF_TRIGGER, mOwner, mPhysicsWorld);

         mPhysicsRep->setTransform(mOwner->getTransform());
      }
   }

   mOwner->enableCollision();

   onCollisionChanged.trigger(colShape);
}

void CollisionComponent::processTick()
{
   if (!isActive())
      return;

   //ProcessTick is where our collision testing begins!

   //callback if we have a persisting contact
   if (mContactInfo.contactObject)
   {
      if (mContactInfo.contactTimer > 0)
      {
         if (isMethod("updateContact"))
            Con::executef(this, "updateContact");

         if (mOwner->isMethod("updateContact"))
            Con::executef(mOwner, "updateContact");
      }

      ++mContactInfo.contactTimer;
   }
   else if (mContactInfo.contactTimer != 0)
      mContactInfo.clear();

   //First, we clear our old collision list. We do this, because if we're still colliding with something, 
   //it'll be added back in, but we don't want to risk colliding with something we've moved past
   //mCollisionList.clear();

   //So, US checking collisions is irrelevent if we're not moving
   //All collision detection is active. Our Entity that is moving will detect collisions against other objects.
   //If we're not moving, we don't do checks outselves, but other moving entities may check and find they'll collide 
   //with us.
   //As such, the first thing we need to do if we're going to check collisions is to see if we have a valid interface
   //to a component that would give us a means to move. This would be the VelocityInterface.
   //If we don't have this, we're not moving, so we can just be done here and now.
   /*if (VelocityInterface *vI = mOwner->getComponent<VelocityInterface>())
   {
      //Welp, looks like we have another component on our entity that has velocity, so grab that.
      //We'll be passing it along so we can correctly estimate our position along the next tick.
      VectorF velocity = vI->getVelocity();

      //If we're not moving, or we never made any convex elements, we won't bother updating out working set
      if (mConvexList && mConvexList->getObject() && !velocity.isZero())
         updateWorkingCollisionSet((U32)-1);
   }*/
}

void CollisionComponent::updatePhysics()
{
   
}

PhysicsCollision* CollisionComponent::getCollisionData()
{
   if ((!PHYSICSMGR || mCollisionType == None) || mOwnerRenderInterface == NULL)
      return NULL;

   PhysicsCollision *colShape = NULL;
   if (mCollisionType == Bounds)
   {
      MatrixF offset(true);
      offset.setPosition(mOwnerRenderInterface->getShape()->center);
      colShape = PHYSICSMGR->createCollision();
      colShape->addBox(mOwner->getObjBox().getExtents() * 0.5f * mOwner->getScale(), offset);
   }
   else if (mCollisionType == CollisionMesh || (mCollisionType == VisibleMesh/* && !mOwner->getComponent<AnimatedMesh>()*/))
   {
      colShape = buildColShapes();
      //colShape = mOwnerShapeInstance->getShape()->buildColShape(mCollisionType == VisibleMesh, mOwner->getScale());
   }
   /*else if (mCollisionType == VisibleMesh && !mOwner->getComponent<AnimatedMesh>())
   {
   //We don't have support for visible mesh collisions with animated meshes currently in the physics abstraction layer
   //so we don't generate anything if we're set to use a visible mesh but have an animated mesh component.
   colShape = mOwnerShapeInstance->getShape()->buildColShape(mCollisionType == VisibleMesh, mOwner->getScale());
   }*/
   else if (mCollisionType == VisibleMesh/* && mOwner->getComponent<AnimatedMesh>()*/)
   {
      Con::printf("CollisionComponent::updatePhysics: Cannot use visible mesh collisions with an animated mesh!");
   }

   return colShape;
}

bool CollisionComponent::castRay(const Point3F &start, const Point3F &end, RayInfo* info)
{
   if (!mCollisionType == None)
   {
      if (mPhysicsWorld)
      {
         bool hit = mPhysicsWorld->castRay(start, end, info, Point3F::Zero);

         return hit;
      }
   }

   return false;
}

PhysicsCollision* CollisionComponent::buildColShapes()
{
   PROFILE_SCOPE(CollisionComponent_buildColShapes);

   PhysicsCollision *colShape = NULL;
   U32 surfaceKey = 0;

   TSShape* shape = mOwnerRenderInterface->getShape();

   if (mCollisionType == VisibleMesh)
   {
      // Here we build triangle collision meshes from the
      // visible detail levels.

      // A negative subshape on the detail means we don't have geometry.
      const TSShape::Detail &detail = shape->details[0];
      if (detail.subShapeNum < 0)
         return NULL;

      // We don't try to optimize the triangles we're given
      // and assume the art was created properly for collision.
      ConcretePolyList polyList;
      polyList.setTransform(&MatrixF::Identity, mOwner->getScale());

      // Create the collision meshes.
      S32 start = shape->subShapeFirstObject[detail.subShapeNum];
      S32 end = start + shape->subShapeNumObjects[detail.subShapeNum];
      for (S32 o = start; o < end; o++)
      {
         const TSShape::Object &object = shape->objects[o];
         if (detail.objectDetailNum >= object.numMeshes)
            continue;

         // No mesh or no verts.... nothing to do.
         TSMesh *mesh = shape->meshes[object.startMeshIndex + detail.objectDetailNum];
         if (!mesh || mesh->mNumVerts == 0)
            continue;

         // Gather the mesh triangles.
         polyList.clear();
         mesh->buildPolyList(0, &polyList, surfaceKey, NULL);

         // Create the collision shape if we haven't already.
         if (!colShape)
            colShape = PHYSICSMGR->createCollision();

         // Get the object space mesh transform.
         MatrixF localXfm;
         shape->getNodeWorldTransform(object.nodeIndex, &localXfm);

         colShape->addTriangleMesh(polyList.mVertexList.address(),
            polyList.mVertexList.size(),
            polyList.mIndexList.address(),
            polyList.mIndexList.size() / 3,
            localXfm);

         /*if (perMesh)
         {
            list->increment();
            list->last().colNode = -1;
            list->last().colShape = colShape;
            colShape = NULL;
         }*/
      }

      // Return what we built... if anything.
      return colShape;
   }
   else if (mCollisionType == CollisionMesh)
   {

      // Scan out the collision hulls...
      //
      // TODO: We need to support LOS collision for physics.
      //
      for (U32 i = 0; i < shape->details.size(); i++)
      {
         const TSShape::Detail &detail = shape->details[i];
         const String &name = shape->names[detail.nameIndex];

         // Is this a valid collision detail.
         if (!dStrStartsWith(name, colisionMeshPrefix) || detail.subShapeNum < 0)
            continue;

         // Now go thru the meshes for this detail.
         S32 start = shape->subShapeFirstObject[detail.subShapeNum];
         S32 end = start + shape->subShapeNumObjects[detail.subShapeNum];
         if (start >= end)
            continue;

         for (S32 o = start; o < end; o++)
         {
            const TSShape::Object &object = shape->objects[o];
            const String &meshName = shape->names[object.nameIndex];

            if (object.numMeshes <= detail.objectDetailNum)
               continue;

            // No mesh, a flat bounds, or no verts.... nothing to do.
            TSMesh *mesh = shape->meshes[object.startMeshIndex + detail.objectDetailNum];
            if (!mesh || mesh->getBounds().isEmpty() || mesh->mNumVerts == 0)
               continue;

            // We need the default mesh transform.
            MatrixF localXfm;
            shape->getNodeWorldTransform(object.nodeIndex, &localXfm);

            // We have some sort of collision shape... so allocate it.
            if (!colShape)
               colShape = PHYSICSMGR->createCollision();

            // We have geometry... what is it?
            /*if (dStrStartsWith(meshName, "Colbox"))
            {
               // The bounds define the box extents directly.
               Point3F halfWidth = mesh->getBounds().getExtents() * 0.5f;

               // Add the offset to the center of the bounds 
               // into the local space transform.
               MatrixF centerXfm(true);
               centerXfm.setPosition(mesh->getBounds().getCenter());
               localXfm.mul(centerXfm);

               colShape->addBox(halfWidth, localXfm);
            }
            else if (dStrStartsWith(meshName, "Colsphere"))
            {
               // Get a sphere inscribed to the bounds.
               F32 radius = mesh->getBounds().len_min() * 0.5f;

               // Add the offset to the center of the bounds 
               // into the local space transform.
               MatrixF primXfm(true);
               primXfm.setPosition(mesh->getBounds().getCenter());
               localXfm.mul(primXfm);

               colShape->addSphere(radius, localXfm);
            }
            else if (dStrStartsWith(meshName, "Colcapsule"))
            {
               // Use the smallest extent as the radius for the capsule.
               Point3F extents = mesh->getBounds().getExtents();
               F32 radius = extents.least() * 0.5f;

               // We need to center the capsule and align it to the Y axis.
               MatrixF primXfm(true);
               primXfm.setPosition(mesh->getBounds().getCenter());

               // Use the longest axis as the capsule height.
               F32 height = -radius * 2.0f;
               if (extents.x > extents.y && extents.x > extents.z)
               {
                  primXfm.setColumn(0, Point3F(0, 0, 1));
                  primXfm.setColumn(1, Point3F(1, 0, 0));
                  primXfm.setColumn(2, Point3F(0, 1, 0));
                  height += extents.x;
               }
               else if (extents.z > extents.x && extents.z > extents.y)
               {
                  primXfm.setColumn(0, Point3F(0, 1, 0));
                  primXfm.setColumn(1, Point3F(0, 0, 1));
                  primXfm.setColumn(2, Point3F(1, 0, 0));
                  height += extents.z;
               }
               else
                  height += extents.y;

               // Add the primitive transform into the local transform.
               localXfm.mul(primXfm);

               // If we didn't find a positive height then fallback to
               // creating a sphere which is better than nothing.
               if (height > 0.0f)
                  colShape->addCapsule(radius, height, localXfm);
               else
                  colShape->addSphere(radius, localXfm);
            }
            else if (dStrStartsWith(meshName, "Colmesh"))
            {
               // For a triangle mesh we gather the triangles raw from the
               // mesh and don't do any optimizations or cleanup.
               ConcretePolyList polyList;
               polyList.setTransform(&MatrixF::Identity, mOwner->getScale());
               mesh->buildPolyList(0, &polyList, surfaceKey, NULL);
               colShape->addTriangleMesh(polyList.mVertexList.address(),
                  polyList.mVertexList.size(),
                  polyList.mIndexList.address(),
                  polyList.mIndexList.size() / 3,
                  localXfm);
            }
            else*/
            {
               // Any other mesh name we assume as a generic convex hull.
               //
               // Collect the verts using the vertex polylist which will 
               // filter out duplicates.  This is importaint as the convex
               // generators can sometimes fail with duplicate verts.
               //
               VertexPolyList polyList;
               MatrixF meshMat(localXfm);

               Point3F t = meshMat.getPosition();
               t.convolve(mOwner->getScale());
               meshMat.setPosition(t);

               polyList.setTransform(&MatrixF::Identity, mOwner->getScale());
               mesh->buildPolyList(0, &polyList, surfaceKey, NULL);
               colShape->addConvex(polyList.getVertexList().address(),
                  polyList.getVertexList().size(),
                  meshMat);
            }

            /*if (perMesh)
            {
               list->increment();

               S32 detailNum;
               String::GetTrailingNumber(name, detailNum);

               String str = String::ToString("%s%i", meshName.c_str(), detailNum);
               S32 found = findNode(str);

               if (found == -1)
               {
                  str = str.replace('-', '_');
                  found = findNode(str);
               }

               list->last().colNode = found;
               list->last().colShape = colShape;

               colShape = NULL;
            }*/

         } // objects

      } // details
   }

   return colShape;
}