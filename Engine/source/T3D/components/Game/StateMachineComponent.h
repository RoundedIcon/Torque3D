//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _STATE_MACHINE_COMPONENT_H_
#define _STATE_MACHINE_COMPONENT_H_

#ifndef _COMPONENT_H_
   #include "T3D/Components/Component.h"
#endif

#include "T3D/components/Game/stateMachine.h"

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class StateMachineComponent : public Component
{
   typedef Component Parent;

public:
   StateMachine mStateMachine;

protected:
   StringTableEntry		mStateMachineFile;

public:
   StateMachineComponent();
   virtual ~StateMachineComponent();
   DECLARE_CONOBJECT(StateMachineComponent);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void onComponentAdd();
   virtual void onComponentRemove();

   void _onResourceChanged(const Torque::Path &path);

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);

   virtual void processTick();

   virtual void onDynamicModified(const char* slotName, const char* newValue);
   virtual void onStaticModified(const char* slotName, const char* newValue);

   virtual void loadStateMachineFile();

   void setStateMachineFile(const char* fileName) { mStateMachineFile = StringTable->insert(fileName); }

   static bool _setSMFile(void *object, const char *index, const char *data);

   void onStateChanged(StateMachine* sm, S32 stateIdx);

   //Callbacks
   DECLARE_CALLBACK(void, onStateChange, ());
};

#endif // _COMPONENT_H_