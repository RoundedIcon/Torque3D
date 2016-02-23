//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _STATIC_MESH_COMPONENT_H_
#define _STATIC_MESH_COMPONENT_H_

#ifndef _COMPONENT_H_
#include "T3D/Components/Component.h"
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
#ifndef _ACTOR_H_
#include "T3D/Entity.h"
#endif

#ifndef _NETSTRINGTABLE_H_
   #include "sim/netStringTable.h"
#endif
/*#ifndef _PATH_H_
#include "core/util/path.h"
#endif*/

#ifndef _CORE_INTERFACES_H_
#include "T3D/Components/coreInterfaces.h"
#endif

#ifndef _RENDER_COMPONENT_INTERFACE_H_
#include "T3D/Components/Render/renderComponentInterface.h"
#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif 
#ifndef _SHAPE_ASSET_H_
#include "T3D/assets/ShapeAsset.h"
#endif 
#ifndef _GFXVERTEXFORMAT_H_
#include "gfx/gfxVertexFormat.h"
#endif

class TSShapeInstance;
class SceneRenderState;
//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class MeshComponent : public Component,
   public RenderComponentInterface,
   //public GeometryInterface,
   public CastRayRenderedInterface,
   public EditorInspectInterface
{
   typedef Component Parent;

protected:
   enum
   {
      ShapeMask = Parent::NextFreeMask,
      MaterialMask = Parent::NextFreeMask << 1,
      NextFreeMask = Parent::NextFreeMask << 2,
   };

   StringTableEntry		mShapeName;
   StringTableEntry		mShapeAsset;
   TSShape*		         mShape;
   //TSShapeInstance *		mShapeInstance;
   Box3F						mShapeBounds;
   Point3F					mCenterOffset;

   struct matMap
   {
      String matName;
      U32 slot;
   };

   Vector<matMap>  mChangingMaterials;
   Vector<matMap>  mMaterials;

   class boneObject : public SimGroup
   {
      MeshComponent *mOwner;
   public:
      boneObject(MeshComponent *owner){ mOwner = owner; }

      StringTableEntry mBoneName;
      S32 mItemID;

      virtual void addObject(SimObject *obj);
   };

   Vector<boneObject*> mNodesList;

   /// This is only valid when the instance was created from
   /// a resource.  Else it is null.
   //Resource<TSShape> mShapeResource;

   /// This should always point to a valid shape and should
   /// equal mShapeResource if it was created from a resource.
   //TSShape *mShape;

public:
   StringTableEntry       mMeshAssetId;
   AssetPtr<ShapeAsset>   mMeshAsset;

   TSShapeInstance*       mShapeInstance;

public:
   MeshComponent();
   virtual ~MeshComponent();
   DECLARE_CONOBJECT(MeshComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void inspectPostApply();

   virtual void prepRenderImage(SceneRenderState *state);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   Box3F getShapeBounds() { return mShapeBounds; }

   virtual MatrixF getNodeTransform(S32 nodeIdx);
   S32 getNodeByName(String nodeName);

   void setupShape();
   void updateShape();
   void updateMaterials();

   virtual void onComponentRemove();
   virtual void onComponentAdd();

   static bool _setMesh(void *object, const char *index, const char *data);
   static bool _setShape(void *object, const char *index, const char *data);
   const char* _getShape(void *object, const char *data);

   bool setMeshAsset(const char* assetName);

   virtual TSShape* getShape() { if (mMeshAsset)  return mMeshAsset->getShape(); else return NULL; }
   virtual TSShapeInstance* getShapeInstance() { return mShapeInstance; }

   Resource<TSShape> getShapeResource() { if (mMeshAsset)  return mMeshAsset->getShapeResource(); else return NULL; }

   void _onResourceChanged(const Torque::Path &path);

   virtual bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info);

   void mountObjectToNode(SceneObject* objB, String node, MatrixF txfm);

   virtual void onDynamicModified(const char* slotName, const char* newValue);

   void changeMaterial(U32 slot, const char* newMat);

   virtual void onInspect();
   virtual void onEndInspect();

   virtual Vector<MatrixF> getNodeTransforms()
   {
      Vector<MatrixF> bob;
      return bob;
   }

   virtual void setNodeTransforms(Vector<MatrixF> transforms)
   {
      return;
   }
};

#endif // _COMPONENT_H_
