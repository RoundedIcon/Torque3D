#include "console/engineAPI.h"
#include "T3D/Components/Camera/CameraComponent.h"

ConsoleMethod(CameraComponent, getMode, const char*, 2, 2, "() - We get the first behavior of the requested type on our owner object.\n"
   "@return (string name) The type of the behavior we're requesting")
{
   return "fly";
}

DefineConsoleMethod(CameraComponent, getForwardVector, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF cameraMat;
   object->getCameraTransform(&pos, &cameraMat);

   VectorF returnVec = cameraMat.getForwardVector();
   returnVec = VectorF(mRadToDeg(returnVec.x), mRadToDeg(returnVec.y), mRadToDeg(returnVec.z));
   returnVec.normalize();
   return returnVec;
}

DefineConsoleMethod(CameraComponent, getRightVector, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF cameraMat;
   object->getCameraTransform(&pos, &cameraMat);

   VectorF returnVec = cameraMat.getRightVector();
   returnVec = VectorF(mRadToDeg(returnVec.x), mRadToDeg(returnVec.y), mRadToDeg(returnVec.z));
   returnVec.normalize();
   return returnVec;
}

DefineConsoleMethod(CameraComponent, getUpVector, VectorF, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF cameraMat;
   object->getCameraTransform(&pos, &cameraMat);

   VectorF returnVec = cameraMat.getUpVector();
   returnVec = VectorF(mRadToDeg(returnVec.x), mRadToDeg(returnVec.y), mRadToDeg(returnVec.z));
   returnVec.normalize();
   return returnVec;
}

DefineConsoleMethod(CameraComponent, setForwardVector, void, (VectorF newForward), (VectorF(0, 0, 0)),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   object->setForwardVector(newForward);
}

DefineConsoleMethod(CameraComponent, getWorldPosition, Point3F, (), ,
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   F32 pos = 0;
   MatrixF mat;
   object->getCameraTransform(&pos, &mat);

   return mat.getPosition();
}