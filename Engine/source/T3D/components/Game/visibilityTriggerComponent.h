//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
#ifndef _VISIBILITY_TRIGGER_COMPONENT_H_
#define _VISIBILITY_TRIGGER_COMPONENT_H_

#ifndef _COMPONENT_H_
#include "T3D/Components/Component.h"
#endif

#ifndef _ENTITY_H_
#include "T3D/Entity.h"
#endif

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class VisibilityTriggerComponent : public Component
{
   typedef Component Parent;


   struct clientInfo
   {
      S32 clientID;
      Point2I screenRes;
      bool triggerCurrentlySeen;
      bool triggerEverSeen;

      clientInfo()
      {
         clientID = -1;
         screenRes = Point2I(0, 0);
         triggerCurrentlySeen = false;
         triggerEverSeen = false;
      }
   };

   Vector<clientInfo> mClientInfo;

   bool mVisible;

   String mOnEnterViewCmd;
   String mOnExitViewCmd;
   String mOnUpdateInViewCmd;

public:
   VisibilityTriggerComponent();
   virtual ~VisibilityTriggerComponent();
   DECLARE_CONOBJECT(VisibilityTriggerComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   //virtual void onComponentAdd();
   //virtual void onComponentRemove();

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void processTick();

   GameConnection* getConnection(S32 connectionID);

   void addClient(S32 clientID);
   void removeClient(S32 clientID);

   void visualizeFrustums(F32 renderTimeMS);

   DECLARE_CALLBACK(void, onEnterViewCmd, (Entity* cameraEnt, bool firstTimeSeeing));
   DECLARE_CALLBACK(void, onExitViewCmd, (Entity* cameraEnt));
   DECLARE_CALLBACK(void, onUpdateInViewCmd, (Entity* cameraEnt));
   DECLARE_CALLBACK(void, onUpdateOutOfViewCmd, (Entity* cameraEnt));
};

#endif // _EXAMPLEBEHAVIOR_H_
