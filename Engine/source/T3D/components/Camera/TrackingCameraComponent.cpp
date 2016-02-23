//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#include "T3D/Components/Camera/TrackingCameraComponent.h"
#include "core/util/safeDelete.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"
#include "math/mathUtils.h"

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

TrackingCameraComponent::TrackingCameraComponent() : Component()
{
   mMaxPitchAngle = 70;
   mMinPitchAngle = -10;

   mTrackedObject = NULL;
}

TrackingCameraComponent::~TrackingCameraComponent()
{
}

IMPLEMENT_CO_NETOBJECT_V1(TrackingCameraComponent);

bool TrackingCameraComponent::onAdd()
{
   if (!Parent::onAdd())
      return false;

   return true;
}

void TrackingCameraComponent::onRemove()
{
   Parent::onRemove();
}
void TrackingCameraComponent::initPersistFields()
{
   Parent::initPersistFields();

   addField("trackObject", TypeSimObjectPtr, Offset(mTrackedObject, TrackingCameraComponent), "Object world orientation.");
}

//This is mostly a catch for situations where the behavior is re-added to the object and the like and we may need to force an update to the behavior
void TrackingCameraComponent::onComponentAdd()
{
   Parent::onComponentAdd();

   CameraComponent *cam = mOwner->getComponent<CameraComponent>();
   if (cam)
   {
      mCamera = cam;
   }
}

void TrackingCameraComponent::onComponentRemove()
{
   Parent::onComponentRemove();
}

U32 TrackingCameraComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);
   return retMask;
}

void TrackingCameraComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);
}

void TrackingCameraComponent::processTick()
{
   Parent::processTick();

   if (mCamera && mTrackedObject)
   {
      // orient the camera to face the object
      Point3F objPos;
      RotationF rotVec;

      mTrackedObject->getWorldBox().getCenter(&objPos);

      Point3F pos = mOwner->getPosition();
      VectorF vec = objPos - pos;
      vec.normalizeSafe();
      F32 pitch, yaw;
      MathUtils::getAnglesFromVector(vec, yaw, pitch);
      rotVec.x = -pitch;
      rotVec.z = yaw;
      if (rotVec.z > M_PI_F)
         rotVec.z -= M_2PI_F;
      else if (rotVec.z < -M_PI_F)
         rotVec.z += M_2PI_F;

      mCamera->setRotation(rotVec);
   }
}