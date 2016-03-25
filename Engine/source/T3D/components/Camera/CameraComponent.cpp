//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "T3D/Components/camera/CameraComponent.h"
#include "T3D/Components/Camera/CameraComponent_ScriptBinding.h"
#include "platform/platform.h"
#include "console/consoleTypes.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "console/consoleTypes.h"
#include "console/consoleObject.h"
#include "ts/tsShapeInstance.h"
#include "core/stream/bitStream.h"
//#include "console/consoleInternal.h"
#include "gfx/gfxTransformSaver.h"
#include "console/engineAPI.h"
#include "lighting/lightQuery.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/gameFunctions.h"
#include "math/mathUtils.h"

#include "T3D/Components/render/renderComponentInterface.h"

IMPLEMENT_CALLBACK( CameraComponent, validateCameraFov, F32, (F32 fov), (fov),
                   "@brief Called on the server when the client has requested a FOV change.\n\n"

                   "When the client requests that its field of view should be changed (because "
                   "they want to use a sniper scope, for example) this new FOV needs to be validated "
                   "by the server.  This method is called if it exists (it is optional) to validate "
                   "the requested FOV, and modify it if necessary.  This could be as simple as checking "
                   "that the FOV falls within a correct range, to making sure that the FOV matches the "
                   "capabilities of the current weapon.\n\n"

                   "Following this method, ShapeBase ensures that the given FOV still falls within "
                   "the datablock's mCameraMinFov and mCameraMaxFov.  If that is good enough for your "
                   "purposes, then you do not need to define the validateCameraFov() callback for "
                   "your ShapeBase.\n\n"

                   "@param fov The FOV that has been requested by the client.\n"
                   "@return The FOV as validated by the server.\n\n"

                   "@see ShapeBaseData\n\n");

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

CameraComponent::CameraComponent() : Component()
{
   mClientScreen = Point2F(1, 1);

   mCameraFov = mCameraDefaultFov = 80;
   mCameraMinFov = 5;
   mCameraMaxFov = 175;

   mTargetNodeIdx = -1;

   mPosOffset = Point3F(0, 0, 0);
   mRotOffset = EulerF(0, 0, 0);

   mTargetNode = "";

   mUseParentTransform = true;

   mFriendlyName = "Camera(Component)";
}

CameraComponent::~CameraComponent()
{
   for(S32 i = 0;i < mFields.size();++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(CameraComponent);

bool CameraComponent::onAdd()
{
   if(! Parent::onAdd())
      return false;

	mNetFlags.set(Ghostable);

   return true;
}

void CameraComponent::onRemove()
{
   Parent::onRemove();
}

void CameraComponent::initPersistFields()
{
   Parent::initPersistFields();

   addProtectedField("FOV", TypeF32, Offset(mCameraFov, CameraComponent), &_setCameraFov, defaultProtectedGetFn, "");

   addField("MinFOV", TypeF32, Offset(mCameraMinFov, CameraComponent), "");

   addField("MaxFOV", TypeF32, Offset(mCameraMaxFov, CameraComponent), "");

   addField("ScreenAspect", TypePoint2I, Offset(mClientScreen, CameraComponent), "");

   addProtectedField("targetNode", TypeString, Offset(mTargetNode, CameraComponent), &_setNode, defaultProtectedGetFn, "");

   addProtectedField("positionOffset", TypePoint3F, Offset(mPosOffset, CameraComponent), &_setPosOffset, defaultProtectedGetFn, "");

   addProtectedField("rotationOffset", TypeRotationF, Offset(mRotOffset, CameraComponent), &_setRotOffset, defaultProtectedGetFn, "");

   addField("useParentTransform", TypeBool, Offset(mUseParentTransform, CameraComponent), "");
}

bool CameraComponent::_setNode(void *object, const char *index, const char *data)
{
   CameraComponent *mcc = static_cast<CameraComponent*>(object);
   
   mcc->mTargetNode = StringTable->insert(data);

   mcc->setMaskBits(OffsetMask);
   //rbI->updateShape(); //make sure we force the update to resize the owner bounds

   return true;
}

bool CameraComponent::_setPosOffset(void *object, const char *index, const char *data)
{
   CameraComponent *mcc = static_cast<CameraComponent*>(object);
   
   if (mcc)
   {
      Point3F pos;
      Con::setData(TypePoint3F, &pos, 0, 1, &data);

      mcc->mPosOffset = pos;

      mcc->setMaskBits(OffsetMask);

      return true;
   }

   return false;
}

bool CameraComponent::_setRotOffset(void *object, const char *index, const char *data)
{
   CameraComponent *mcc = static_cast<CameraComponent*>(object);

   if (mcc)
   {
      RotationF rot;
      Con::setData(TypeRotationF, &rot, 0, 1, &data);

      mcc->mRotOffset = rot;

      mcc->setMaskBits(OffsetMask);

      return true;
   }

   return false;
}

bool CameraComponent::isValidCameraFov(F32 fov)
{
   return((fov >= mCameraMinFov) && (fov <= mCameraMaxFov));
}

bool CameraComponent::_setCameraFov(void *object, const char *index, const char *data)
{
   CameraComponent *cCI = static_cast<CameraComponent*>(object);
   cCI->setCameraFov(dAtof(data));
   return true;
}

void CameraComponent::setCameraFov(F32 fov)
{
   // On server allow for script side checking
   /*if ( !isGhost() && isMethod( "validateCameraFov" ) )
   {
   fov = validateCameraFov_callback( fov );
   }*/

   mCameraFov = mClampF(fov, mCameraMinFov, mCameraMaxFov);

   if (isClientObject())
      GameSetCameraTargetFov(mCameraFov);

   if (isServerObject())
      setMaskBits(FOVMask);
}

void CameraComponent::onCameraScopeQuery(NetConnection *cr, CameraScopeQuery * query)
{
   // update the camera query
   query->camera = this;

   if(GameConnection * con = dynamic_cast<GameConnection*>(cr))
   {
      // get the fov from the connection (in deg)
      F32 fov;
      if (con->getControlCameraFov(&fov))
      {
         query->fov = mDegToRad(fov/2);
         query->sinFov = mSin(query->fov);
         query->cosFov = mCos(query->fov);
      }
      else
      {
         query->fov = mDegToRad(mCameraFov/2);
         query->sinFov = mSin(query->fov);
         query->cosFov = mCos(query->fov);
      }
   }

   // use eye rather than camera transform (good enough and faster)
   MatrixF camTransform = mOwner->getTransform();
   camTransform.getColumn(3, &query->pos);
   camTransform.getColumn(1, &query->orientation);

   // Get the visible distance.
   if (mOwner->getSceneManager() != NULL)
      query->visibleDistance = mOwner->getSceneManager()->getVisibleDistance();

   //mOwner->Parent::onCameraScopeQuery( cr, query );
}

bool CameraComponent::getCameraTransform(F32* pos,MatrixF* mat)
{
   // Returns camera to world space transform
   // Handles first person / third person camera position
   bool isServer = isServerObject();

   if (mTargetNodeIdx == -1)
   {
      if (mUseParentTransform)
      {
         MatrixF rMat = mOwner->getRenderTransform();

         rMat.mul(mRotOffset.asMatrixF());
         
         mat->set(rMat.toEuler(), rMat.getPosition() + mPosOffset);
      }
      else
      {
         mat->set(mRotOffset.asEulerF(), mPosOffset);
      }

      return true;
   }
   else
   {
      RenderComponentInterface *renderInterface = mOwner->getComponent<RenderComponentInterface>();

      if (!renderInterface/* || !renderInterface->getShapeInstance()*/)
         return false;

      //TSShapeInstance *shapeInstance = renderInterface->getShapeInstance();

      //shapeInstance->animateNodeSubtrees(true);

      //if (isServerObject() && mShapeInstance)
      //   mShapeInstance->animateNodeSubtrees(true);

      /*if (*pos != 0)
      {
         F32 min, max;
         Point3F offset;
         MatrixF trans, rot;
         getCameraParameters(&min, &max, &offset, &rot);
         trans = mOwner->getRenderTransform();

         mat->mul(trans, rot);

         // Use the eye transform to orient the camera
         VectorF vp, vec;
         vp.x = vp.z = 0;
         vp.y = -(max - min) * *pos;
         trans.mulV(vp, &vec);

         VectorF minVec;
         vp.y = -min;
         trans.mulV(vp, &minVec);

         // Use the camera node's pos.
         Point3F osp, sp;

         mTargetNodeIdx = renderInterface->getShape()->findNode(mTargetNode);
         if (mTargetNodeIdx != -1)
         {
            renderInterface->getNodeTransform(mTargetNodeIdx).getColumn(3, &osp);

            // Scale the camera position before applying the transform
            const Point3F& scale = mOwner->getScale();
            osp.convolve(scale);

            mOwner->getRenderTransform().mulP(osp, &sp);
         }
         else
            mOwner->getRenderTransform().getColumn(3, &sp);

         // Make sure we don't extend the camera into anything solid
         Point3F ep = sp + minVec + vec + offset;

         mat->setColumn(3, ep);
      }
      else*/
      {
         if (mUseParentTransform)
         {
            MatrixF rMat = mOwner->getRenderTransform();

            Point3F position = rMat.getPosition();

            RotationF rot = mRotOffset;

            if (mTargetNodeIdx != -1)
            {
               Point3F nodPos;
               MatrixF nodeTrans = renderInterface->getNodeTransform(mTargetNodeIdx);
               nodeTrans.getColumn(3, &nodPos);

               // Scale the camera position before applying the transform
               const Point3F& scale = mOwner->getScale();
               nodPos.convolve(scale);

               mOwner->getRenderTransform().mulP(nodPos, &position);

               nodeTrans.mul(rMat);

               rot = nodeTrans;
            }
            else
            {
               bool tm = true;
            }

            position += mPosOffset;

            MatrixF rotMat = rot.asMatrixF();

            MatrixF rotOffsetMat = mRotOffset.asMatrixF();

            rotMat.mul(rotOffsetMat);

            rot = RotationF(rotMat);

            mat->set(rot.asEulerF(), position);
         }
         else
         {
            MatrixF rMat = mOwner->getRenderTransform();

            Point3F position = rMat.getPosition();

            RotationF rot = mRotOffset;

            if (mTargetNodeIdx != -1)
            {
               Point3F nodPos;
               MatrixF nodeTrans = renderInterface->getNodeTransform(mTargetNodeIdx);
               nodeTrans.getColumn(3, &nodPos);

               // Scale the camera position before applying the transform
               const Point3F& scale = mOwner->getScale();
               nodPos.convolve(scale);

               position = nodPos;

               //rMat.mulP(nodPos, &position);
            }

            position += mPosOffset;

            mat->set(rot.asEulerF(), position);
         }
      }

      return true;
   }
}

void CameraComponent::getCameraParameters(F32 *min, F32* max, Point3F* off, MatrixF* rot)
{
   *min = 0.2f;
   *max = 0.f;
   off->set(0, 0, 0);
   rot->identity();
}

U32 CameraComponent::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retmask = Parent::packUpdate(con, mask, stream);

   if (stream->writeFlag(mask & FOVMask))
   {
      stream->write(mCameraFov);
   }

   if (stream->writeFlag(mask & OffsetMask))
   {
      RenderComponentInterface* renderInterface = getOwner()->getComponent<RenderComponentInterface>();

      if (renderInterface && renderInterface->getShape())
      {
         S32 nodeIndex = renderInterface->getShape()->findNode(mTargetNode);

         mTargetNodeIdx = nodeIndex;
      }

      stream->writeInt(mTargetNodeIdx, 32);
      //send offsets here

      stream->writeCompressedPoint(mPosOffset);
      stream->writeCompressedPoint(mRotOffset.asEulerF());

      stream->writeFlag(mUseParentTransform);
   }

   return retmask;
}

void CameraComponent::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   if (stream->readFlag())
   {
      F32 fov;
      stream->read(&fov);
      setCameraFov(fov);
   }

   if(stream->readFlag())
   {
      F32 fov;
      //stream->read(&fov);

      //setCameraFov(fov);
      //char buf[256];
      //stream->readString(buf);

      //S32 index;
      mTargetNodeIdx = stream->readInt(32);

      bool test = true;
      //mTargetNode = buf;

      stream->readCompressedPoint(&mPosOffset);

      EulerF rot;
      stream->readCompressedPoint(&rot);

      mRotOffset = RotationF(rot);

      mUseParentTransform = stream->readFlag();
   }
}

void CameraComponent::setForwardVector(VectorF newForward, VectorF upVector)
{
   MatrixF mat;
   F32 pos = 0;
   getCameraTransform(&pos, &mat);

   mPosOffset = mat.getPosition();

   VectorF up(0.0f, 0.0f, 1.0f);
   VectorF axisX;
   VectorF axisY = newForward;
   VectorF axisZ;

   if (upVector != VectorF::Zero)
      up = upVector;

   // Validate and normalize input:  
   F32 lenSq;
   lenSq = axisY.lenSquared();
   if (lenSq < 0.000001f)
   {
      axisY.set(0.0f, 1.0f, 0.0f);
      Con::errorf("Entity::setForwardVector() - degenerate forward vector");
   }
   else
   {
      axisY /= mSqrt(lenSq);
   }

   lenSq = up.lenSquared();
   if (lenSq < 0.000001f)
   {
      up.set(0.0f, 0.0f, 1.0f);
      Con::errorf("SceneObject::setForwardVector() - degenerate up vector - too small");
   }
   else
   {
      up /= mSqrt(lenSq);
   }

   if (fabsf(mDot(up, axisY)) > 0.9999f)
   {
      Con::errorf("SceneObject::setForwardVector() - degenerate up vector - same as forward");
      // i haven't really tested this, but i think it generates something which should be not parallel to the previous vector:  
      F32 tmp = up.x;
      up.x = -up.y;
      up.y = up.z;
      up.z = tmp;
   }

   // construct the remaining axes:  
   mCross(axisY, up, &axisX);
   mCross(axisX, axisY, &axisZ);

   mat.setColumn(0, axisX);
   mat.setColumn(1, axisY);
   mat.setColumn(2, axisZ);

   mRotOffset = RotationF(mat.toEuler());
   mRotOffset.y = 0;

   setMaskBits(OffsetMask);
}

void CameraComponent::setPosition(Point3F newPos)
{
   mPosOffset = newPos;
   setMaskBits(OffsetMask);
}

void CameraComponent::setRotation(RotationF newRot)
{
   mRotOffset = newRot;
   setMaskBits(OffsetMask);
}

Frustum CameraComponent::getFrustum()
{
   Frustum visFrustum;
   F32 left, right, top, bottom;
   F32 aspectRatio = mClientScreen.x / mClientScreen.y;
   //MathUtils::makeFrustum( &left, &right, &top, &bottom, mCameraFov, aspectRatio, 0.1f );

   //visFrustum.set(false, left, right, top, bottom, 0.1f, 1000, mOwner->getRenderTransform());
   visFrustum.set(false, mDegToRad(mCameraFov), aspectRatio, 0.1f, 1000, mOwner->getTransform());

   return visFrustum;
}