//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "T3D/Components/Render/MeshComponent.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "sim/netConnection.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "scene/sceneManager.h"

#include "gfx/bitmap/ddsFile.h"
#include "gfx/bitmap/ddsUtils.h"
#include "gfx/gfxTextureManager.h"
#include "materials/materialFeatureTypes.h"

#include "renderInstance/renderImposterMgr.h"
#include "util/imposterCapture.h"

#include "gfx/sim/debugDraw.h"  
#include "gfx/gfxDrawUtil.h"

#include "materials/materialManager.h"
#include "materials/matInstance.h"

#include "core/strings/findMatch.h"
//
#include "gui/controls/guiTreeViewCtrl.h"

#include "T3D/components/Render/MeshComponent_ScriptBinding.h"

/*Vector<Animation*>             TSShapeInstance::smRotationThreads(__FILE__, __LINE__);
Vector<TSThread*>             TSShapeInstance::smTranslationThreads(__FILE__, __LINE__);
Vector<TSThread*>             TSShapeInstance::smScaleThreads(__FILE__, __LINE__);*/

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

MeshComponent::MeshComponent() : Component()
{
   mShapeName = StringTable->insert("");
   mShapeAsset = StringTable->insert("");
   //mShapeInstance = NULL;

   mChangingMaterials.clear();

   mMaterials.clear();

   mFriendlyName = "Mesh Component";
   mComponentType = "Render";

   mDescription = getDescriptionText("Causes the object to render a non-animating 3d shape using the file provided.");

   mNetworked = true;
   mNetFlags.set(Ghostable | ScopeAlways);
}

MeshComponent::~MeshComponent(){}

IMPLEMENT_CO_NETOBJECT_V1(MeshComponent);

//==========================================================================================
void MeshComponent::boneObject::addObject(SimObject* object)
{
   //Parent::addObject(object);
   SceneObject* sc = dynamic_cast<SceneObject*>(object);

   if(sc && mOwner)
   {
      if(TSShape* shape = mOwner->getShape())
      {
         S32 nodeID = shape->findNode(mBoneName);

         //we may have a offset on the shape's center
         //so make sure we accomodate for that when setting up the mount offsets
         MatrixF mat = mOwner->getNodeTransform(nodeID);
         //mat.setPosition(mat.getPosition() + mOwner->getShape()->getShape()->center);

         mOwner->getOwner()->mountObject(sc, nodeID, mat);
      }
   }
}

bool MeshComponent::onAdd()
{
   if(! Parent::onAdd())
      return false;

   // Register for the resource change signal.
   ResourceManager::get().getChangedSignal().notify( this, &MeshComponent::_onResourceChanged );

   //get the default shape, if any
   //updateShape();

   return true;
}

void MeshComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   //get the default shape, if any
   updateShape();
}

void MeshComponent::onRemove()
{
   Parent::onRemove();

   SAFE_DELETE(mShapeInstance);
}

void MeshComponent::onComponentRemove()
{
   if(mOwner)
   {
      Point3F pos = mOwner->getPosition(); //store our center pos
      mOwner->setObjectBox(Box3F(Point3F(-1,-1,-1), Point3F(1,1,1)));
      //mOwner->resetWorldBox();
      mOwner->setPosition(pos);
   }  

   Parent::onComponentRemove();  
}

void MeshComponent::initPersistFields()
{
   Parent::initPersistFields();

   //create a hook to our internal variables
   addGroup("Model");
   //addField("shapeName",   TypeShapeFilename,  Offset( mShapeName, MeshComponent ), 
   //   "%Path and filename of the model file (.DTS, .DAE) to use for this TSStatic.", AbstractClassRep::FieldFlags::FIELD_ComponentInspectors);
   addProtectedField("MeshAsset", TypeAssetId, Offset(mShapeAsset, MeshComponent), &_setMesh, &defaultProtectedGetFn, 
      "The asset Id used for the mesh.", AbstractClassRep::FieldFlags::FIELD_ComponentInspectors);
   endGroup("Model");
   //addProtectedField("shapeName", TypeShapeFilename, Offset(mShapeName, MeshComponent), &_setShape, defaultProtectedGetFn);
}

bool MeshComponent::_setMesh(void *object, const char *index, const char *data)
{
   MeshComponent *rbI = static_cast<MeshComponent*>(object);
   
   // Sanity!
   AssertFatal(data != NULL, "Cannot use a NULL asset Id.");

   return rbI->setMeshAsset(data);
}

bool MeshComponent::_setShape( void *object, const char *index, const char *data )
{
   MeshComponent *rbI = static_cast<MeshComponent*>(object);
   rbI->mShapeName = StringTable->insert(data);
   rbI->updateShape(); //make sure we force the update to resize the owner bounds
   rbI->setMaskBits(ShapeMask);

   return true;
}

bool MeshComponent::setMeshAsset(const char* assetName)
{
   // Fetch the asset Id.
   mMeshAssetId = StringTable->insert(assetName);
   mMeshAsset.setAssetId(mMeshAssetId);

   if (mMeshAsset.isNull())
   {
      Con::errorf("[MeshComponent] Failed to load mesh asset.");
      return false;
   }

   mShapeName = mMeshAssetId;
   mShapeAsset = mShapeName;
   updateShape(); //make sure we force the update to resize the owner bounds
   setMaskBits(ShapeMask);

   return true;
}

void MeshComponent::_onResourceChanged( const Torque::Path &path )
{
   if ( path != Torque::Path( mShapeName ) )
      return;

   updateShape();
   setMaskBits(ShapeMask);
}

void MeshComponent::inspectPostApply()
{
   Parent::inspectPostApply();

   //updateShape();
   //setMaskBits(ShapeMask);

   //updateMaterials();
  // setMaskBits(MaterialsMask);
}

U32 MeshComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (!mOwner || con->getGhostIndex(mOwner) == -1)
   {
      stream->writeFlag(false);
      stream->writeFlag(false);

      if (mask & ShapeMask)
         retMask |= ShapeMask;
      if (mask & MaterialMask)
         retMask |= MaterialMask;
      return retMask;
   }

   if (stream->writeFlag(mask & ShapeMask))
   {
      stream->writeString(mShapeName);
   }

   /*if( mask & (ShapeMask | InitialUpdateMask))
   {
      if(!mOwner)
      {
         stream->writeFlag( false );
      }
      else if( con->getGhostIndex(mOwner) != -1 )
      {
         stream->writeFlag( true );
         stream->writeString( mShapeName );
      }
      else
      {
         retMask |= ShapeMask; //try it again untill our dependency is ghosted
         stream->writeFlag( false );
      }
   }*/

   if (stream->writeFlag( mask & MaterialMask ))
   {
      /*if(!mOwner)
      {
         stream->writeFlag( false );
      }
      else if( con->getGhostIndex(mOwner) != -1 )
      {
         stream->writeFlag( true );*/

         stream->writeInt(mChangingMaterials.size(), 16);

         for(U32 i=0; i < mChangingMaterials.size(); i++)
         {
            stream->writeInt(mChangingMaterials[i].slot, 16);
            con->packNetStringHandleU(stream, NetStringHandle(mChangingMaterials[i].matName));
         }
         //stream->writeInt(mMaterialChangeSlot, 16);
         //con->packNetStringHandleU(stream, mMaterialChangeHandle);

         mChangingMaterials.clear();
         /*U32 materialCount = mMaterialHandles.size();

         int id = getId();
         bool serv = isServerObject();

         stream->writeInt(materialCount, 16);

         for(U32 i=0; i < materialCount; i++)
         {
            String test = String(mMaterialHandles[i].getString());
            con->packNetStringHandleU(stream, mMaterialHandles[i]);
         }*/
      /*}
      else
      {
         retMask |= MaterialMask; //try it again untill our dependency is ghosted
         stream->writeFlag( false );
      }*/
   }

   return retMask;
}

void MeshComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if(stream->readFlag())
   {
      mShapeName = stream->readSTString();
      setMeshAsset(mShapeName);
      updateShape();
   }

   if(stream->readFlag())
   {
      mChangingMaterials.clear();
      U32 materialCount = stream->readInt(16);

      for(U32 i=0; i < materialCount; i++)
      {
         matMap newMatMap;
         newMatMap.slot = stream->readInt(16);
         newMatMap.matName = String(con->unpackNetStringHandleU(stream).getString());

         mChangingMaterials.push_back(newMatMap);
      }

      //U32 mMaterialChangeSlot = stream->readInt(16);
      //mMaterialChangeHandle = con->unpackNetStringHandleU(stream);
      /*for(U32 i=0; i < materialCount; i++)
      {
         NetStringHandle matHandle = con->unpackNetStringHandleU(stream);
         mMaterialHandles.push_back(matHandle);
      }*/

      updateMaterials();
   }
}

void MeshComponent::prepRenderImage( SceneRenderState *state )
{
   if (!mEnabled || !mOwner || !mShapeInstance)
      return;

   Point3F cameraOffset;
   mOwner->getRenderTransform().getColumn(3, &cameraOffset);
   cameraOffset -= state->getDiffuseCameraPosition();
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   Point3F objScale = getOwner()->getScale();
   F32 invScale = (1.0f / getMax(getMax(objScale.x, objScale.y), objScale.z));

   mShapeInstance->setDetailFromDistance(state, dist * invScale);

   if (mShapeInstance->getCurrentDetail() < 0)
      return;

   GFXTransformSaver saver;

   // Set up our TS render state.
   TSRenderState rdata;
   rdata.setSceneState(state);
   rdata.setFadeOverride(1.0f);
   rdata.setOriginSort(false);

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init(mOwner->getWorldSphere());
   rdata.setLightQuery(&query);

   MatrixF mat = mOwner->getTransform();
   Point3F renderPos = mat.getPosition();
   EulerF renderRot = mat.toEuler();
   Point3F renderScale = mat.getScale();
   mat.scale(objScale);
   GFX->setWorldMatrix(mat);

   //mShapeInstance->animate();
   mShapeInstance->render(rdata);

   return;
   //
   //
   //
   // get shape detail...we might not even need to be drawn
   /*Entity *o = dynamic_cast<Entity*>(getOwner());
   Box3F box = getOwner()->getWorldBox();
   Point3F diffuseCamPos = state->getDiffuseCameraPosition();

   Point3F cameraOffset = getOwner()->getWorldBox().getClosestPoint(diffuseCamPos) - diffuseCamPos;
   F32 dist = cameraOffset.len();
   if (dist < 0.01f)
      dist = 0.01f;

   Point3F objScale = getOwner()->getScale();
   F32 invScale = (1.0f/getMax(getMax(objScale.x,objScale.y),objScale.z));

   GFXTransformSaver saver;

   // Set up our TS render state. 
   TSRenderState rdata;
   rdata.setSceneState(state);

   // We might have some forward lit materials
   // so pass down a query to gather lights.
   LightQuery query;
   query.init(getOwner()->getWorldSphere());
   rdata.setLightQuery(&query);

   MatrixF mat = getOwner()->getRenderTransform();
   Point3F center = getShape()->center;
   Point3F position = mat.getPosition();

   getOwner()->getObjToWorld().mulP(center);

   Point3F posOffset = position - center;

   //mat.setPosition(position + posOffset);
   mat.scale(objScale);

   GFX->setWorldMatrix(mat);

   //mShapeInstance->animate();
   mShapeInstance->render(rdata);*/
}

void MeshComponent::updateShape()
{
   bool isServer = isServerObject();

   if ((mShapeName && mShapeName[0] != '\0') || (mShapeAsset && mShapeAsset[0] != '\0'))
   {
      if (mMeshAsset == NULL)
         return;

      /*if (!dStrcmp(mShapeAsset, ""))
      {
         mShape = ResourceManager::get().load(mShapeName);

         if (!mShape)
         {
            Con::errorf("MeshComponent::updateShape : failed to load shape file!");
            return; //if it failed to load, bail out
         }
      }*/

      mShape = mMeshAsset->getShape();

      if (!mShape)
         return;

      setupShape();

      //Do this on both the server and client
      S32 materialCount = mShape->materialList->getMaterialNameList().size();

      //mMaterials.clear();

      /*for(U32 i=0; i < materialCount; i++)
      {
         String materialname = mShape->materialList->getMaterialName(i);
         if(materialname == String("ShapeBounds"))
            continue;

         matMap newMatMap;
         newMatMap.slot = i;
         newMatMap.matName = materialname;

         mMaterials.push_back(newMatMap);
      }*/

      if(isServerObject())
      {
         //we need to update the editor
         for (U32 i = 0; i < mFields.size(); i++)
         {
            //find any with the materialslot title and clear them out
            if (FindMatch::isMatch("MaterialSlot*", mFields[i].mFieldName, false))
            {
               setDataField(mFields[i].mFieldName, NULL, "");
               mFields.erase(i);
               continue;
            }
         }

         //next, get a listing of our materials in the shape, and build our field list for them
         char matFieldName[128];

         if(materialCount > 0)
            mComponentGroup = StringTable->insert("Materials");

         for(U32 i=0; i < materialCount; i++)
         {
            String materialname = mShape->materialList->getMaterialName(i);
            if(materialname == String("ShapeBounds"))
               continue;

            dSprintf(matFieldName, 128, "MaterialSlot%d", i);
            
            addComponentField(matFieldName, "A material used in the shape file", "TypeAssetId", materialname, "");
         }

         if(materialCount > 0)
            mComponentGroup = "";
      }

      if(mOwner != NULL)
      {
         Point3F min, max, pos;
         pos = mOwner->getPosition();

         mOwner->getWorldToObj().mulP(pos);

         //min = mShape->bounds.minExtents - (pos + mShapeInstance->getShape()->center);
         //max = mShape->bounds.maxExtents - (pos + mShapeInstance->getShape()->center);

         min = mShape->bounds.minExtents;
         max = mShape->bounds.maxExtents;

         mShapeBounds.set(min, max);

         mOwner->setObjectBox(Box3F(min, max));
         //e->setObjectBox(Box3F(min, max));

         //mOwner->setObjectBox(Box3F(Point3F(-3, -3, -3), Point3F(3, 3, 3)));
         //mOwner->resetWorldBox();
         //e->setMaskBits(Entity::BoundsMask);

         if( mOwner->getSceneManager() != NULL )
            mOwner->getSceneManager()->notifyObjectDirty( mOwner );
      }

      //finally, notify that our shape was changed
      onShapeInstanceChanged.trigger(this);
   }
}

void MeshComponent::setupShape()
{
   mShapeInstance = new TSShapeInstance(mShape, true);
}

void MeshComponent::updateMaterials()
{
   if (mChangingMaterials.empty() || !mShape)
      return;

   TSMaterialList* pMatList = mShapeInstance->getMaterialList();
   pMatList->setTextureLookupPath(getShapeResource().getPath().getPath());

   const Vector<String> &materialNames = pMatList->getMaterialNameList();
   for ( S32 i = 0; i < materialNames.size(); i++ )
   {
      const String &pName = materialNames[i];

      for(U32 m=0; m < mChangingMaterials.size(); m++)
      {
         if(mChangingMaterials[m].slot == i)
         {
            pMatList->renameMaterial( i, mChangingMaterials[m].matName );
         }
      }
      
      /*for(U32 i=0; i < mMaterials.size(); i++)
      {
         for(U32 m = 0; m < mChangingMaterials.size(); m++)
         {
            if(mMaterials[i].slot == mChangingMaterials[m].slot)
               mMaterials[i].matName = mChangingMaterials[m].matName;
         }
      }*/

      mChangingMaterials.clear();
   }

   // Initialize the material instances
   mShapeInstance->initMaterialList();
}

MatrixF MeshComponent::getNodeTransform(S32 nodeIdx)
{
   if (mShape)
   {
      S32 nodeCount = getShape()->nodes.size();

      if(nodeIdx >= 0 && nodeIdx < nodeCount)
      {
         //animate();
         MatrixF mountTransform = mShapeInstance->mNodeTransforms[nodeIdx];
         mountTransform.mul(mOwner->getRenderTransform());

         return mountTransform;
      }
   }

   return MatrixF::Identity;
}

S32 MeshComponent::getNodeByName(String nodeName)
{
   if (mShape)
   {
      S32 nodeIdx = getShape()->findNode(nodeName);

      return nodeIdx;
   }

   return -1;
}

bool MeshComponent::castRayRendered(const Point3F &start, const Point3F &end, RayInfo *info)
{
   return false;
   /*if ( !mShapeInstance )
   return false;

   // Cast the ray against the currently visible detail
   RayInfo localInfo;
   bool res = mShapeInstance->castRayOpcode( mShapeInstance->getCurrentDetail(), start, end, &localInfo );

   if ( res )
   {
   *info = localInfo;
   info->object = mOwner;
   return true;
   }

   return false;*/
}

void MeshComponent::mountObjectToNode(SceneObject* objB, String node, MatrixF txfm)
{
   const char* test;
   test = node.c_str();
   if(dIsdigit(test[0]))
   {
      getOwner()->mountObject(objB, dAtoi(node), txfm);
   }
   else
   {
      if(TSShape* shape = getShape())
      {
         S32 idx = shape->findNode(node);
         getOwner()->mountObject(objB, idx, txfm);
      }
   }
}

/*Geometry* MeshComponent::getGeometry()
{
return NULL;
}*/

void MeshComponent::onDynamicModified(const char* slotName, const char* newValue)
{
   if(FindMatch::isMatch( "materialslot*", slotName, false ))
   {
      if(!getShape())
         return;

      S32 slot = -1;
      String outStr( String::GetTrailingNumber( slotName, slot ) );

      if(slot == -1)
         return;

      bool found = false;
      for(U32 i=0; i < mChangingMaterials.size(); i++)
      {
         if(mChangingMaterials[i].slot == slot)
         {
            mChangingMaterials[i].matName = String(newValue);
            found = true;
         }
      }

      if(!found)
      {
         matMap newMatMap;
         newMatMap.slot = slot;
         newMatMap.matName = String(newValue);

         mChangingMaterials.push_back(newMatMap);
      }

      setMaskBits(MaterialMask);
   }

   Parent::onDynamicModified(slotName, newValue);
}

void MeshComponent::changeMaterial(U32 slot, const char* newMat)
{
   /*bool server = isServerObject();
   int id = getId();

   if(!getShape())
      return;

   bool found = false;
   for(U32 i=0; i < mChangingMaterials.size(); i++)
   {
      if(mChangingMaterials[i].slot == slot)
      {
         mChangingMaterials[i].matName = String(newMat);
         found = true;
      }
   }

   if(!found)
   {
      matMap newMatMap;
      newMatMap.slot = slot;
      newMatMap.matName = String(newMat);

      mChangingMaterials.push_back(newMatMap);
   }

   //update our server-side list as well.
   /*for(U32 i=0; i < mMaterials.size(); i++)
   {
      if(mMaterials[i].slot == slot)
         mMaterials[i].matName = String(newMat);
   }*/

   char fieldName[512];

   //update our respective field
   dSprintf(fieldName, 512, "materialSlot%d", slot);
   setDataField(fieldName, NULL, newMat);

   /*if(slot < mMaterialHandles.size())
   {
      mMaterialHandles.erase(slot);

      NetStringHandle matHandle = NetStringHandle(newMat);
      mMaterialHandles.push_back(matHandle);
   }*/

   //setMaskBits(MaterialMask);
}

void MeshComponent::onInspect()
{
   return;
   //accumulate a temporary listing of objects to represent the bones
   //then we add these to our object here, and finally add our object to our owner specifically
   //so that we, and all the bones under us, show in the heirarchy of the scene
   //The objects we use are a special simgroup class just for us, that have specific callbacks
   //in the event an entity is mounted to it.

   //mOwner->addObject(this);
   /*GuiTreeViewCtrl *editorTree = dynamic_cast<GuiTreeViewCtrl*>(Sim::findObject("EditorTree"));
   if(!editorTree)
   return;

   if(mNodesList.empty())
   {
   if(!mShapeInstance)
   return;

   GuiTreeViewCtrl::Item *newItem, *parentItem;

   parentItem = editorTree->getItem(editorTree->findItemByObjectId(mOwner->getId()));

   S32 componentID = editorTree->insertItem(parentItem->getID(), "MeshComponent");

   newItem = editorTree->getItem(componentID);
   newItem->mInspectorInfo.mObject = this;
   newItem->mState.set(GuiTreeViewCtrl::Item::DenyDrag);

   TSShape* shape = mShapeInstance->getShape();
   S32 nodeCount = shape->nodes.size();

   String nodeName, parentName;

   for(U32 i=0; i < nodeCount; i++)
   {
      S32 nID = shape->nodes[i].nameIndex;
      S32 pID = shape->nodes[i].parentIndex;
      S32 parentItemID;

      nodeName = shape->getNodeName(shape->nodes[i].nameIndex);
      if(pID != -1)
      {
         bool found = false;
         for(U32 b=0; b < mNodesList.size(); b++)
         {
            if(!dStrcmp(mNodesList[b]->mBoneName, shape->getNodeName(pID)))
            {
               parentItemID = mNodesList[b]->mItemID;
               found = true;
               break;
            }
         }

         if(!found)
            parentItemID = componentID;
      }
      else
      {
         parentItemID = componentID;
      }

      S32 boneID = editorTree->insertItem(parentItemID, nodeName);
      newItem = editorTree->getItem(boneID);

      boneObject *b = new boneObject(this);
      b->mBoneName = nodeName;
      b->mItemID = boneID;

      mNodesList.push_back(b);

      newItem->mInspectorInfo.mObject = b;
      newItem->mState.set(GuiTreeViewCtrl::Item::ForceItemName);
      newItem->mState.set(GuiTreeViewCtrl::Item::InspectorData);
      newItem->mState.set(GuiTreeViewCtrl::Item::ForceDragTarget);
      newItem->mState.set(GuiTreeViewCtrl::Item::DenyDrag);

      //while we're here, check our parent to see if anything is mounted to this node.
      //if it is, hijack the item and move it under us!
      for (SceneObject* itr = mOwner->getMountList(); itr; itr = itr->getMountLink())
      {
         if(itr->getMountNode() == i)
         {
            newItem = editorTree->getItem(editorTree->findItemByObjectId(itr->getId()));
            newItem->mParent = editorTree->getItem(boneID);
         }
      }
   }

   /*GuiTreeViewCtrl::Item *newItem, *parentItem;

   parentItem = editorTree->getItem(editorTree->findItemByObjectId(mOwner->getId()));

   S32 componentID = editorTree->insertItem(parentItem->getID(), "MeshComponent");

   newItem = editorTree->getItem(componentID);
   newItem->mInspectorInfo.mObject = this;
   newItem->mState.set(GuiTreeViewCtrl::Item::DenyDrag);

   boneObject *b = new boneObject(this);
   b->mBoneName = StringTable->insert("Root");

   mNodesList.push_back(b);

   S32 boneID = editorTree->insertItem(componentID, b->mBoneName);
   newItem = editorTree->getItem(boneID);

   newItem->mInspectorInfo.mObject = b;
   newItem->mState.set(GuiTreeViewCtrl::Item::ForceItemName);
   newItem->mState.set(GuiTreeViewCtrl::Item::InspectorData);
   newItem->mState.set(GuiTreeViewCtrl::Item::ForceDragTarget);
   newItem->mState.set(GuiTreeViewCtrl::Item::DenyDrag);*/

   //editorTree->buildVisibleTree(true);
   //}

}

void MeshComponent::onEndInspect()
{
   //mOwner->removeObject(this);
}