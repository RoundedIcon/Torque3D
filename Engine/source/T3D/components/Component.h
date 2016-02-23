//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#ifndef _COMPONENT_H__
#define _COMPONENT_H__

#ifndef _NETOBJECT_H_
#include "sim/netObject.h"
#endif
#ifndef _ENTITY_H_
#include "T3D/Entity.h"
#endif

#ifndef _CORE_INTERFACES_H_
#include "T3D/Components/coreInterfaces.h"
#endif

class Entity;

struct ComponentField
{
   StringTableEntry mFieldName;
   StringTableEntry mFieldDescription;

   S32 mFieldType;
   StringTableEntry mUserData;

   StringTableEntry mDefaultValue;

   StringTableEntry mGroup;

   StringTableEntry mDependency;

   bool mHidden;
};

//////////////////////////////////////////////////////////////////////////
/// 
/// 
//////////////////////////////////////////////////////////////////////////
class Component : public NetObject, public UpdateInterface
{
   typedef NetObject Parent;

protected:
   StringTableEntry mFriendlyName;
   StringTableEntry mDescription;

   StringTableEntry mFromResource;
   StringTableEntry mComponentGroup;
   StringTableEntry mComponentType;
   StringTableEntry mNetworkType;
   StringTableEntry mTemplateName;

   Vector<StringTableEntry> mDependencies;
   Vector<ComponentField> mFields;

   bool mNetworked;

   //This is a hacky stand-in until we get proper serialization
   //this lets us mark if it's our template/reference component and, if so, not network it down
   //bool mIsTemplate;

   U32 componentIdx;

   Entity*               mOwner;
   bool					   mHidden;
   bool					   mEnabled;

   //Component		    *mTemplate;

public:
   Component();
   virtual ~Component();
   DECLARE_CONOBJECT(Component);

   virtual bool onAdd();
   virtual void onRemove();
   static void initPersistFields();

   virtual void packToStream(Stream &stream, U32 tabStop, S32 behaviorID, U32 flags = 0);

   //This is called when we are added to an entity
   virtual void onComponentAdd();            
   //This is called when we are removed from an entity
   virtual void onComponentRemove();         

   //This is called when a different component is added to our owner entity
   virtual void componentAddedToOwner(Component *comp);  
   //This is called when a different component is removed from our owner entity
   virtual void componentRemovedFromOwner(Component *comp);  

   virtual void ownerTransformSet(MatrixF *mat);

   void setOwner(Entity* pOwner);
   inline Entity *getOwner() { return mOwner ? mOwner : NULL; }
   static bool setOwner(void *object, const char *index, const char *data) { return true; }

   bool	isEnabled() { return mEnabled; }
   void  setEnabled(bool toggle) { mEnabled = toggle; setMaskBits(EnableMask); }

   bool isActive() { return mEnabled && mOwner != NULL; }

   static bool _setEnabled(void *object, const char *index, const char *data);

   //void pushUpdate();

   virtual void processTick();
   virtual void interpolateTick(F32 dt){}
   virtual void advanceTime(F32 dt){}

   /// @name Adding Named Fields
   /// @{

   /// Adds a named field to a Component that can specify a description, data type, default value and userData
   ///
   /// @param   fieldName    The name of the Field
   /// @param   desc         The Description of the Field
   /// @param   type         The Type of field that this is, example 'Text' or 'Bool'
   /// @param   defaultValue The Default value of this field
   /// @param   userData     An extra optional field that can be used for user data
   void addComponentField(const char *fieldName, const char *desc, const char *type, const char *defaultValue = NULL, const char *userData = NULL, bool hidden = false);

   /// Returns the number of ComponentField's on this template
   inline S32 getComponentFieldCount() { return mFields.size(); };

   /// Gets a ComponentField by its index in the mFields vector 
   /// @param idx  The index of the field in the mField vector
   inline ComponentField *getComponentField(S32 idx)
   {
      if (idx < 0 || idx >= mFields.size())
         return NULL;

      return &mFields[idx];
   }

   ComponentField *getComponentField(const char* fieldName);

   const char* getComponentType() { return mComponentType; }

   const char *getDescriptionText(const char *desc);

   const char *getName() { return mTemplateName; }

   const char *getFriendlyName() { return mFriendlyName; }

   bool isNetworked() { return mNetworked; }

   void beginFieldGroup(const char* groupName);
   void endFieldGroup();

   void addDependency(StringTableEntry name);
   /// @}

   /// @name Description
   /// @{
   static bool setDescription(void *object, const char *index, const char *data);
   static const char* getDescription(void* obj, const char* data);

   //
   //static bool setTemplate(void *object, const char *index, const char *data);

   /// @Primary usage functions
   /// @These are used by the various engine-based behaviors to integrate with the component classes
   enum NetMaskBits
   {
      InitialUpdateMask = BIT(0),
      OwnerMask = BIT(1),
      UpdateMask = BIT(2),
      EnableMask = BIT(3),
      NextFreeMask = BIT(4)
   };

   virtual U32 packUpdate(NetConnection *con, U32 mask, BitStream *stream);
   virtual void unpackUpdate(NetConnection *con, BitStream *stream);
   /// @}

   Signal< void(SimObject*, String, String) > onDataSet;
   virtual void setDataField(StringTableEntry slotName, const char *array, const char *value);

   virtual void onStaticModified(const char* slotName, const char* newValue); ///< Called when a static field is modified.
   virtual void onDynamicModified(const char* slotName, const char*newValue = NULL); ///< Called when a dynamic field is modified.
   /// This is what we actually use to check if the modified field is one of our behavior fields. If it is, we update and make the correct callbacks
   void checkComponentFieldModified(const char* slotName, const char* newValue);

   virtual void checkDependencies(){}
};

#endif // _COMPONENT_H_
