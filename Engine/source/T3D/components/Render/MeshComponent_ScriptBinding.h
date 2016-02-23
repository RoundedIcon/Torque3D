#include "console/engineAPI.h"
#include "T3D/components/Render/MeshComponent.h"
#include "scene/sceneObject.h"
#include "math/mTransform.h"

DefineEngineMethod(MeshComponent, getShapeBounds, Box3F, (), ,
   "@brief Get the cobject we're in contact with.\n\n"

   "The controlling client is the one that will send moves to us to act on.\n"

   "@return the ID of the controlling GameConnection, or 0 if this object is not "
   "controlled by any client.\n"

   "@see GameConnection\n")
{
   return object->getShapeBounds();
}

DefineEngineMethod(MeshComponent, mountObject, bool,
   (SceneObject* objB, String node, TransformF txfm), (MatrixF::Identity),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (objB)
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      object->mountObjectToNode(objB, node, /*MatrixF::Identity*/txfm.getMatrix());
      return true;
   }
   return false;
}

DefineEngineMethod(MeshComponent, getNodeTransform, TransformF,
   (S32 node), (-1),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (node != -1)
   {

      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      MatrixF mat = object->getNodeTransform(node);
      return mat;
   }

   return TransformF::Identity;
}

DefineEngineMethod(MeshComponent, getNodeEulerRot, EulerF,
   (S32 node, bool radToDeg), (-1, true),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (node != -1)
   {

      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      MatrixF mat = object->getNodeTransform(node);

      EulerF eul = mat.toEuler();
      if (radToDeg)
         eul = EulerF(mRadToDeg(eul.x), mRadToDeg(eul.y), mRadToDeg(eul.z));

      return eul;
   }

   return EulerF(0, 0, 0);
}

DefineEngineMethod(MeshComponent, getNodePosition, Point3F,
   (S32 node), (-1),
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (node != -1)
   {

      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      MatrixF mat = object->getNodeTransform(node);

      return mat.getPosition();
   }

   return Point3F(0, 0, 0);
}

DefineEngineMethod(MeshComponent, getNodeByName, S32,
   (String nodeName), ,
   "@brief Mount objB to this object at the desired slot with optional transform.\n\n"

   "@param objB  Object to mount onto us\n"
   "@param slot  Mount slot ID\n"
   "@param txfm (optional) mount offset transform\n"
   "@return true if successful, false if failed (objB is not valid)")
{
   if (!nodeName.isEmpty())
   {
      //BUG: Unsure how it broke, but atm the default transform passed in here is rotated 180 degrees. This doesn't happen
      //for the SceneObject mountobject method. Hackish, but for now, just default to a clean MatrixF::Identity
      //object->mountObjectToNode( objB, node, /*MatrixF::Identity*/txfm.getMatrix() );
      S32 node = object->getNodeByName(nodeName);

      return node;
   }

   return -1;
}

DefineEngineMethod(MeshComponent, changeMaterial, void, (U32 slot, const char* newMat), (0, ""),
   "@brief Change one of the materials on the shape.\n\n")
{
   object->changeMaterial(slot, newMat);
}
/*DefineEngineMethod( MeshComponent, changeMaterial, void, ( const char* mapTo, Material* oldMat, Material* newMat ),("",NULL,NULL),
"@brief Change one of the materials on the shape.\n\n"

"This method changes materials per mapTo with others. The material that "
"is being replaced is mapped to unmapped_mat as a part of this transition.\n"

"@note Warning, right now this only sort of works. It doesn't do a live "
"update like it should.\n"

"@param mapTo the name of the material target to remap (from getTargetName)\n"
"@param oldMat the old Material that was mapped \n"
"@param newMat the new Material to map\n\n"

"@tsexample\n"
"// remap the first material in the shape\n"
"%mapTo = %obj.getTargetName( 0 );\n"
"%obj.changeMaterial( %mapTo, 0, MyMaterial );\n"
"@endtsexample\n" )
{
// if no valid new material, theres no reason for doing this
if( !newMat )
{
Con::errorf("TSShape::changeMaterial failed: New material does not exist!");
return;
}

// Check the mapTo name exists for this shape
S32 matIndex = object->getShape()->materialList->getMaterialNameList().find_next(String(mapTo));
if (matIndex < 0)
{
Con::errorf("TSShape::changeMaterial failed: Invalid mapTo name '%s'", mapTo);
return;
}

// Lets remap the old material off, so as to let room for our current material room to claim its spot
if( oldMat )
oldMat->mMapTo = String("unmapped_mat");

newMat->mMapTo = mapTo;

// Map the material by name in the matmgr
MATMGR->mapMaterial( mapTo, newMat->getName() );

// Replace instances with the new material being traded in. Lets make sure that we only
// target the specific targets per inst, this is actually doing more than we thought
delete object->getShape()->materialList->mMatInstList[matIndex];
object->getShape()->materialList->mMatInstList[matIndex] = newMat->createMatInstance();

// Finish up preparing the material instances for rendering
const GFXVertexFormat *flags = getGFXVertexFormat<GFXVertexPNTTB>();
FeatureSet features = MATMGR->getDefaultFeatures();
object->getShape()->materialList->getMaterialInst(matIndex)->init( features, flags );
}*/