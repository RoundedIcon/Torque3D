//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _CAMERA_ORBITER_COMPONENT_H_
#define _CAMERA_ORBITER_COMPONENT_H_

#ifndef _COMPONENT_H_
#include "T3D/Components/Component.h"
#endif

#ifndef _CAMERA_COMPONENT_H_
#include "T3D/Components/camera/cameraComponent.h"
#endif

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class CameraOrbiterComponent : public Component
{
   typedef Component Parent;

   F32 mMinOrbitDist;
   F32 mMaxOrbitDist;
   F32 mCurOrbitDist;
   Point3F mPosition;

   F32 mMaxPitchAngle;
   F32 mMinPitchAngle;

   RotationF mRotation;

   CameraComponent* mCamera;

public:
   CameraOrbiterComponent();
   virtual ~CameraOrbiterComponent();
   DECLARE_CONOBJECT(CameraOrbiterComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void processTick();

   void calcOrbitPoint(MatrixF* mat, const Point3F& rot);
   void lookAt(const Point3F& pos);
   //void autoFitRadius(F32 radius);

   void validateEyePoint(F32 pos, MatrixF *mat);
};

#endif // _EXAMPLEBEHAVIOR_H_
