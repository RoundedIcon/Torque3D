//This is basically a helper file that has general-usage behavior interfaces for rendering
#ifndef _RENDER_COMPONENT_INTERFACE_H_
#define _RENDER_COMPONENT_INTERFACE_H_

#ifndef _TSSHAPE_H_
#include "ts/TSShape.h"
#endif
#ifndef _TSSHAPEINSTANCE_H_
#include "ts/TSShapeInstance.h"
#endif
/*#ifndef _GEOMETRY_H_
#include "math/mGeometry.h"
#endif*/

#ifndef _CORE_INTERFACES_H_
#include "T3D/Components/coreInterfaces.h"
#endif

class RenderComponentInterface : public Interface < RenderComponentInterface >
{
public:
   virtual void prepRenderImage(SceneRenderState *state) = 0;

   virtual TSShape* getShape() = 0;

   Signal< void(RenderComponentInterface*) > RenderComponentInterface::onShapeChanged;

   virtual TSShapeInstance* getShapeInstance() = 0;

   virtual MatrixF getNodeTransform(S32 nodeIdx) = 0;

   virtual Vector<MatrixF> getNodeTransforms() = 0;

   virtual void setNodeTransforms(Vector<MatrixF> transforms) = 0;

   Signal< void(RenderComponentInterface*) > RenderComponentInterface::onShapeInstanceChanged;
};
/*
class PrepRenderImageInterface : public Interface<PrepRenderImageInterface>
{
public:
   virtual void prepRenderImage(SceneRenderState *state) = 0;
};

class TSShapeInterface// : public Interface<TSShapeInterface>
{
public:
   virtual TSShape* getShape()=0;

   Signal< void(TSShapeInterface*) > TSShapeInterface::onShapeChanged;
};

class TSShapeInstanceInterface// : public Interface<TSShapeInstanceInterface>
{
public:
   virtual TSShapeInstance* getShapeInstance()=0;

   Signal< void(TSShapeInstanceInterface*) > TSShapeInstanceInterface::onShapeInstanceChanged;
};*/

/*class GeometryInterface
{
public:
virtual Geometry* getGeometry()=0;
};*/

class CastRayRenderedInterface// : public Interface<CastRayRenderedInterface>
{
public:
   virtual bool castRayRendered(const Point3F &start, const Point3F &end, RayInfo* info)=0;
};

#endif