//-----------------------------------------------------------------------------
// Torque Game Engine
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "console/simBase.h"
#include "console/consoleTypes.h"
#include "T3D/Components/Component.h"
#include "core/util/safeDelete.h"
#include "core/resourceManager.h"
#include "core/stream/fileStream.h"
#include "core/stream/bitStream.h"
#include "console/engineAPI.h"
#include "sim/netConnection.h"

#include "console/consoleInternal.h"

//#include "T3D/Components/coreInterfaces.h"

#define DECLARE_NATIVE_COMPONENT( ComponentType )                   \
	 Component* staticComponentTemplate = new ComponentType; \
     Sim::gNativeComponentSet->addObject(staticComponentTemplate);

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

Component::Component()
{
   mFriendlyName = StringTable->lookup("");
   mFromResource = StringTable->lookup("");
   mComponentType = StringTable->lookup("");
   mComponentGroup = StringTable->lookup("");
   mNetworkType = StringTable->lookup("");
   mTemplateName = StringTable->lookup("");
	mComponentFieldName = StringTable->lookup("");
   //mDependency = StringTable->lookup("");

   mNetworked = false;

   //This is a hacky stand-in until we get proper serialization
   //this lets us mark if it's our template/reference component and, if so, not network it down
   //mIsTemplate = true;

   // [tom, 1/12/2007] We manage the memory for the description since it
   // could be loaded from a file and thus massive. This is accomplished with
   // protected fields, but since they still call Con::getData() the field
   // needs to always be valid. This is pretty lame.
   mDescription = new char[1];
   ((char *)mDescription)[0] = 0;

   mOwner = NULL;

   mCanSaveFieldDictionary = false;

   mNetFlags.set(Ghostable);
}

Component::~Component()
{
   for (S32 i = 0; i < mFields.size(); ++i)
   {
      ComponentField &field = mFields[i];
      SAFE_DELETE_ARRAY(field.mFieldDescription);
   }

   SAFE_DELETE_ARRAY(mDescription);
}

IMPLEMENT_CO_NETOBJECT_V1(Component);

//////////////////////////////////////////////////////////////////////////

void Component::initPersistFields()
{
   addGroup("Component");
		addField("componentType", TypeCaseString, Offset(mComponentType, Component), "The type of behavior.", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
		addField("networkType", TypeCaseString, Offset(mNetworkType, Component), "The type of behavior.", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
		addField("friendlyName", TypeCaseString, Offset(mFriendlyName, Component), "Human friendly name of this behavior", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
      addProtectedField("description", TypeCaseString, Offset(mDescription, Component), &setDescription, &getDescription,
         "The description of this behavior which can be set to a \"string\" or a fileName\n", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);

      addField("networked", TypeBool, Offset(mNetworked, Component), "Is this behavior ghosted to clients?", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
		addField("fieldName", TypeString, Offset(mComponentFieldName, Component), "aaaaaaaaaaaaaaaaaa", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);

      //This is a hacky stand-in until we get proper serialization
      //this lets us mark if it's our template/reference component and, if so, not network it down
      //addField("isTemplate", TypeBool, Offset(mIsTemplate, Component), "Is this behavior ghosted to clients?", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);

      //addProtectedField("template", TypeSimObjectPtr, Offset(mTemplate, Component), &setTemplate, &defaultProtectedGetFn, "", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
      addProtectedField("Owner", TypeSimObjectPtr, Offset(mOwner, Component), &setOwner, &defaultProtectedGetFn, "", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);

      //addField("hidden", TypeBool, Offset(mHidden, Component), "Flags if this behavior is shown in the editor or not", AbstractClassRep::FieldFlags::FIELD_HideInInspectors);
      addProtectedField("enabled", TypeBool, Offset(mEnabled, Component), &_setEnabled, &defaultProtectedGetFn, "");
   endGroup("Component");

   Parent::initPersistFields();

   //clear out irrelevent fields
   removeField("name");
   //removeField("internalName");
   removeField("parentGroup");
   //removeField("class");
   removeField("superClass");
   removeField("hidden");
   removeField("canSave");
   removeField("canSaveDynamicFields");
   removeField("persistentId");
}

bool Component::_setEnabled(void *object, const char *index, const char *data)
{
   Component *c = static_cast<Component*>(object);

   c->mEnabled = dAtob(data);

   c->setMaskBits(EnableMask);
   //rbI->updateShape(); //make sure we force the update to resize the owner bounds

   return true;
}

//////////////////////////////////////////////////////////////////////////

bool Component::setDescription(void *object, const char *index, const char *data)
{
   Component *bT = static_cast<Component *>(object);
   SAFE_DELETE_ARRAY(bT->mDescription);
   bT->mDescription = bT->getDescriptionText(data);

   // We return false since we don't want the console to mess with the data
   return false;
}

const char * Component::getDescription(void* obj, const char* data)
{
   Component *object = static_cast<Component *>(obj);

   return object->mDescription ? object->mDescription : "";
}

/*bool Component::setTemplate(void *object, const char *index, const char *data)
{
   Component *comp = static_cast<Component *>(object);

   //when we set our template, we're copying data from our template and setting it as our namespace.
   Component *templateComp;
   if (Sim::findObject(data, templateComp))
   {
      // Con::printf(" - Parent object found: %s", parent->getClassName());

      comp->setCopySource(templateComp);
      comp->assignFieldsFrom(templateComp);
      comp->mNameSpace = templateComp->mNameSpace;

      comp->mTemplateName = templateComp->mNameSpace->getName();

      comp->mEnabled = true; //don't inherit the enabled status from a template. We assume a new component should be active
      comp->mIsTemplate = false;
   }

   // We return false since we don't want the console to mess with the data
   return false;
}*/

//////////////////////////////////////////////////////////////////////////

bool Component::onAdd()
{
   if (!Parent::onAdd())
      return false;

   String tmpClsNm = mClassName;

   //setScopeAlways();

   setMaskBits(UpdateMask);

	setMaskBits(NamespaceMask);

   //Sim::gComponentSet->addObject(this);

   //force the callback
   //Con::executef(this, "onAdd");

  // if (mNetworked && !mIsTemplate)
  //    setScopeAlways();

   return true;
}

void Component::onRemove()
{
   //Sim::gComponentSet->removeObject(this);

   bool isServerObj = isServerObject();

   onDataSet.removeAll();

   if (mOwner)
   {
      //looks like we're being deleted directly, so handle our onComponentRemoved call automagically
      //onComponentRemove();

      //notify our removal to the owner, so we have no loose ends
      mOwner->removeComponent(this, false);
   }

   Parent::onRemove();
}

void Component::onComponentAdd()
{
   if (isServerObject())
   {
      if (isMethod("onAdd"))
         Con::executef(this, "onAdd");
   }

   mEnabled = true;
}

void Component::onComponentRemove()
{
   mEnabled = false;

   if (isServerObject())
   {
      if (isMethod("onRemove"))
         Con::executef(this, "onRemove");
   }

   if (mOwner)
   {
      mOwner->onComponentAdded.remove(this, &Component::componentAddedToOwner);
      mOwner->onComponentRemoved.remove(this, &Component::componentRemovedFromOwner);
      mOwner->onTransformSet.remove(this, &Component::ownerTransformSet);
   }

   mOwner = NULL;
   setDataField("owner", NULL, "");
}

void Component::setOwner(Entity* owner)
{
   //first, catch if we have an existing owner, and we're changing from it
   if (mOwner && mOwner != owner)
   {
      mOwner->onComponentAdded.remove(this, &Component::componentAddedToOwner);
      mOwner->onComponentRemoved.remove(this, &Component::componentRemovedFromOwner);
      mOwner->onTransformSet.remove(this, &Component::ownerTransformSet);

      mOwner->removeComponent(this, false);
   }

   mOwner = owner;

   if (mOwner != NULL)
   {
      mOwner->onComponentAdded.notify(this, &Component::componentAddedToOwner);
      mOwner->onComponentRemoved.notify(this, &Component::componentRemovedFromOwner);
      mOwner->onTransformSet.notify(this, &Component::ownerTransformSet);
   }

   if (isServerObject())
	{
      setMaskBits(OwnerMask);
	}
}

void Component::componentAddedToOwner(Component *comp)
{
   return;
}

void Component::componentRemovedFromOwner(Component *comp)
{
   return;
}

void Component::ownerTransformSet(MatrixF *mat)
{
   bool tmp = true;
   return;
}

U32 Component::packUpdate(NetConnection *con, U32 mask, BitStream *stream)
{
   U32 retMask = Parent::packUpdate(con, mask, stream);

   if (mask & OwnerMask)
   {
      if (mOwner != NULL)
      {
         S32 ghostIndex = con->getGhostIndex(mOwner);

         if (ghostIndex == -1)
         {
            stream->writeFlag(false);
            retMask |= OwnerMask;
         }
         else
         {
            stream->writeFlag(true);
            stream->writeFlag(true);
            stream->writeInt(ghostIndex, NetConnection::GhostIdBitSize);
				if (stream->writeFlag(mComponentFieldName != ""))
				{
					stream->writeString(mComponentFieldName);
				}
         }
      }
      else
      {
         stream->writeFlag(true);
         stream->writeFlag(false);
      }
   }
   else
      stream->writeFlag(false);

	if (stream->writeFlag(mask & NamespaceMask))
	{
		const char* name = getName();
		if (stream->writeFlag(name && name[0]))
			stream->writeString(String(name));

		if (stream->writeFlag(mSuperClassName && mSuperClassName[0]))
			stream->writeString(String(mSuperClassName));

		if (stream->writeFlag(mClassName && mClassName[0]))
			stream->writeString(String(mClassName));
	}

   if (stream->writeFlag(mask & EnableMask))
   {
      stream->writeFlag(mEnabled);
   }

   return retMask;
}

void Component::unpackUpdate(NetConnection *con, BitStream *stream)
{
   Parent::unpackUpdate(con, stream);

   /*if (stream->readFlag())
   {
      char buf[256];
      stream->readString(buf);
      mNetworkType = StringTable->insert(buf);

      stream->readString(buf);
      mComponentType = StringTable->insert(buf);

      stream->readString(buf);
      mTemplateName = StringTable->insert(buf);
   }*/

   if (stream->readFlag())
   {
      if (stream->readFlag())
      {
         //we have an owner object, so fetch it
         S32 gIndex = stream->readInt(NetConnection::GhostIdBitSize);

         Entity *e = dynamic_cast<Entity*>(con->resolveGhost(gIndex));
			if (stream->readFlag())
			{
				char buff[255];
				stream->readString(buff);
				mComponentFieldName = buff;
			}
         if (e)
			{
            e->addComponent(this);
				linkNamespaces();
			}
      }
      else
      {
         //it's being nulled out
         setOwner(NULL);
      }
   }

	if (stream->readFlag())
	{
		if (stream->readFlag())
		{
			char name[256];
			stream->readString(name);
			assignName(name);
		}
		if (stream->readFlag())
		{
			char superClassname[256];
			stream->readString(superClassname);
			mSuperClassName = superClassname;
		}
		if (stream->readFlag())
		{
			char classname[256];
			stream->readString(classname);
			mClassName = classname;
		}

		linkNamespaces();
	}

   if (stream->readFlag())
   {
      mEnabled = stream->readFlag();
   }
}

void Component::packToStream(Stream &stream, U32 tabStop, S32 behaviorID, U32 flags /* = 0  */)
{
   //String bufString = "";
   //S32 breakCount = 0;
   char buffer[1024];

   // Write out common info
   //bufString = buffer;
   //stream.write(dStrlen(buffer), buffer);

   //String className = mClassName;

   //String templateName = !dStrcmp(mClassName, className) ? "" : mTemplateName;

   //if (dStrcmp(mClassName, "") && dStrcmp(mClassName, className))
   /*if (dStrcmp(className, ""))
   {
      dSprintf(buffer, sizeof(buffer), "class = %s;\n", className.c_str());

      stream.writeTabs(tabStop);
      stream.write(dStrlen(buffer), buffer);
   }*/

   writeFields(stream, tabStop);

   //stream.write(2, "\r\n");

   // Write out the fields which the behavior template knows about
   for (int i = 0; i < getComponentFieldCount(); i++)
   {
      ComponentField *field = getComponentField(i);
      const char *objFieldValue = getDataField(field->mFieldName, NULL);

      // If the field holds the same value as the template's default value than it
      // will get initialized by the template, and so it won't be included just
      // to try to keep the object files looking as non-horrible as possible.
      if (dStrcmp(field->mDefaultValue, objFieldValue) != 0)
      {
         dSprintf(buffer, sizeof(buffer), "%s = \"%s\";\n", field->mFieldName, (dStrlen(objFieldValue) > 0 ? objFieldValue : "0"));

         stream.writeTabs(tabStop);
         stream.write(dStrlen(buffer), buffer);
      }
   }

   //stream.write(4, "\";\r\n" );
}

void Component::processTick()
{
   if (/*isServerObject() &&*/ mEnabled)
   {
      if (mOwner != NULL && isMethod("Update"))
         Con::executef(this, "Update");
   }
}

void Component::setDataField(StringTableEntry slotName, const char *array, const char *value)
{
   Parent::setDataField(slotName, array, value);

   onDataSet.trigger(this, slotName, value);
}


//catch any behavior field updates
void Component::onStaticModified(const char* slotName, const char* newValue)
{
   Parent::onStaticModified(slotName, newValue);

   //If we don't have an owner yet, then this is probably the initial setup, so we don't need the console callbacks yet.
   if (!mOwner)
      return;

   onDataSet.trigger(this, slotName, newValue);

   checkComponentFieldModified(slotName, newValue);
}

void Component::onDynamicModified(const char* slotName, const char* newValue)
{
   Parent::onDynamicModified(slotName, newValue);

   //If we don't have an owner yet, then this is probably the initial setup, so we don't need the console callbacks yet.
   if (!mOwner)
      return;

   checkComponentFieldModified(slotName, newValue);
}

void Component::checkComponentFieldModified(const char* slotName, const char* newValue)
{
   StringTableEntry slotNameEntry = StringTable->insert(slotName);
   //find if it's a behavior field
   for (int i = 0; i < mFields.size(); i++)
   {
      ComponentField *field = getComponentField(i);
      if (field->mFieldName == slotNameEntry)
      {
         //we have a match, do the script callback that we updated a field
         if (isMethod("onInspectorUpdate"))
            Con::executef(this, "onInspectorUpdate", slotName);

         /*BehaviorFieldInterface *bInterface = mOwner->getInterface<BehaviorFieldInterface)();

         BehaviorInterface *bInterface = dynamic_cast<BehaviorFieldInterface*>(mOwner->getInterface(NULL, "behaviorFieldUpdate", NULL));

         if(bInterface)
         {
         BehaviorFieldInterface *bInterface = dynamic_cast<BehaviorFieldInterface*>(bInterface)
         bInterface->onFieldChange(slotName, newValue);
         }*/


         //Lastly, notify up to our owner's parent(s). If one is a prefab, we inform it it's now dirty
         //Prefab* p = Prefab::getPrefabByChild(mOwner);
         //if (p)
         //   p->setDirty();
         return;
      }
   }
}
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void Component::addComponentField(const char *fieldName, const char *desc, const char *type, const char *defaultValue /* = NULL */, const char *userData /* = NULL */, /*const char* dependency /* = NULL *//*,*/ bool hidden /* = false */)
{
   StringTableEntry stFieldName = StringTable->insert(fieldName);

   for (S32 i = 0; i < mFields.size(); ++i)
   {
      if (mFields[i].mFieldName == stFieldName)
         return;
   }

   ComponentField field;
   field.mFieldName = stFieldName;

   //find the field type
   S32 fieldTypeMask = -1;
   StringTableEntry fieldType = StringTable->insert(type);

   if (fieldType == StringTable->insert("TypeS32"))
      fieldTypeMask = TypeS32;
   else if (fieldType == StringTable->insert("TypeF32"))
      fieldTypeMask = TypeF32;
   else if (fieldType == StringTable->insert("TypePoint3F"))
      fieldTypeMask = TypePoint3F;
   else if (fieldType == StringTable->insert("TypeMaterialName"))
      fieldTypeMask = TypeMaterialName;
   else if (fieldType == StringTable->insert("TypeImageFilename"))
      fieldTypeMask = TypeImageFilename;
   else if (fieldType == StringTable->insert("TypeShapeFilename"))
      fieldTypeMask = TypeShapeFilename;
   else if (fieldType == StringTable->insert("TypeBool"))
      fieldTypeMask = TypeBool;
   else
      fieldTypeMask = TypeString;

   field.mFieldType = fieldTypeMask;

   field.mUserData = StringTable->insert(userData ? userData : "");
   field.mDefaultValue = StringTable->insert(defaultValue ? defaultValue : "");
   field.mFieldDescription = getDescriptionText(desc);

   //field.mDependency = StringTable->insert(dependency ? dependency : "");

   field.mGroup = mComponentGroup;

   field.mHidden = hidden;

   mFields.push_back(field);

   //Before we set this, we need to do a test to see if this field was already set, like from the mission file or a taml file
   const char* curFieldData = getDataField(field.mFieldName, NULL);

   if (dStrIsEmpty(curFieldData))
      setDataField(field.mFieldName, NULL, field.mDefaultValue);
}

ComponentField* Component::getComponentField(const char *fieldName)
{
   StringTableEntry stFieldName = StringTable->insert(fieldName);

   for (S32 i = 0; i < mFields.size(); ++i)
   {
      if (mFields[i].mFieldName == stFieldName)
         return &mFields[i];
   }

   return NULL;
}

//////////////////////////////////////////////////////////////////////////

const char * Component::getDescriptionText(const char *desc)
{
   if (desc == NULL)
      return NULL;

   char *newDesc;

   // [tom, 1/12/2007] If it isn't a file, just do it the easy way
   if (!Platform::isFile(desc))
   {
      newDesc = new char[dStrlen(desc) + 1];
      dStrcpy(newDesc, desc);

      return newDesc;
   }

   FileStream str;
   str.open(desc, Torque::FS::File::Read);

   Stream *stream = &str;
   if (stream == NULL){
      str.close();
      return NULL;
   }

   U32 size = stream->getStreamSize();
   if (size > 0)
   {
      newDesc = new char[size + 1];
      if (stream->read(size, (void *)newDesc))
         newDesc[size] = 0;
      else
      {
         SAFE_DELETE_ARRAY(newDesc);
      }
   }

   str.close();
   delete stream;
   //ResourceManager->closeStream(stream);

   return newDesc;
}
//////////////////////////////////////////////////////////////////////////
/*void Component::pushUpdate()
{
   setMaskBits(UpdateMask);
}*/

void Component::beginFieldGroup(const char* groupName)
{
   if (dStrcmp(mComponentGroup, ""))
   {
      Con::errorf("Component: attempting to begin new field group with a group already begun!");
      return;
   }

   mComponentGroup = StringTable->insert(groupName);
}

void Component::endFieldGroup()
{
   mComponentGroup = StringTable->insert("");
}

void Component::addDependency(StringTableEntry name)
{
   mDependencies.push_back_unique(name);
}

//////////////////////////////////////////////////////////////////////////
// Console Methods
//////////////////////////////////////////////////////////////////////////
ConsoleMethod(Component, beginGroup, void, 3, 3, "(groupName)\n"
   "Starts the grouping for following fields being added to be grouped into\n"
   "@param groupName The name of this group\n"
   "@param desc The Description of this field\n"
   "@param type The DataType for this field (default, int, float, Point2F, bool, enum, Object, keybind, color)\n"
   "@param defaultValue The Default value for this field\n"
   "@param userData An extra data field that can be used for custom data on a per-field basis<br>Usage for default types<br>"
   "-enum: a TAB separated list of possible values<br>"
   "-object: the T2D object type that are valid choices for the field.  The object types observe inheritance, so if you have a t2dSceneObject field you will be able to choose t2dStaticSrpites, t2dAnimatedSprites, etc.\n"
   "@return Nothing\n")
{
   object->beginFieldGroup(argv[2]);
}

ConsoleMethod(Component, endGroup, void, 2, 2, "()\n"
   "Ends the grouping for prior fields being added to be grouped into\n"
   "@param groupName The name of this group\n"
   "@param desc The Description of this field\n"
   "@param type The DataType for this field (default, int, float, Point2F, bool, enum, Object, keybind, color)\n"
   "@param defaultValue The Default value for this field\n"
   "@param userData An extra data field that can be used for custom data on a per-field basis<br>Usage for default types<br>"
   "-enum: a TAB separated list of possible values<br>"
   "-object: the T2D object type that are valid choices for the field.  The object types observe inheritance, so if you have a t2dSceneObject field you will be able to choose t2dStaticSrpites, t2dAnimatedSprites, etc.\n"
   "@return Nothing\n")
{
   object->endFieldGroup();
}

DefineConsoleMethod(Component, addComponentField, void, (String fieldName, String fieldDesc, String fieldType, String defValue, String userData, bool hidden),
   ("", "", "", "", "", false),
   "Get the number of static fields on the object.\n"
   "@return The number of static fields defined on the object.")
{
   if (!dStrcmp(fieldName, "moveSpeed"))
      bool temp = true;

   object->addComponentField(fieldName, fieldDesc, fieldType, defValue, userData, hidden);
}

ConsoleMethod(Component, getComponentFieldCount, S32, 2, 2, "() - Get the number of ComponentField's on this object\n"
   "@return Returns the number of BehaviorFields as a nonnegative integer\n")
{
   return object->getComponentFieldCount();
}

// [tom, 1/12/2007] Field accessors split into multiple methods to allow space
// for long descriptions and type data.

ConsoleMethod(Component, getComponentField, const char *, 3, 3, "(int index) - Gets a Tab-Delimited list of information about a ComponentField specified by Index\n"
   "@param index The index of the behavior\n"
   "@return FieldName, FieldType and FieldDefaultValue, each separated by a TAB character.\n")
{
   ComponentField *field = object->getComponentField(dAtoi(argv[2]));
   if (field == NULL)
      return "";

   char *buf = Con::getReturnBuffer(1024);
   dSprintf(buf, 1024, "%s\t%s\t%s\t%s", field->mFieldName, field->mFieldType, field->mDefaultValue, field->mGroup);

   return buf;
}

ConsoleMethod(Component, setComponentField, const char *, 3, 3, "(int index) - Gets a Tab-Delimited list of information about a ComponentField specified by Index\n"
   "@param index The index of the behavior\n"
   "@return FieldName, FieldType and FieldDefaultValue, each separated by a TAB character.\n")
{
   ComponentField *field = object->getComponentField(dAtoi(argv[2]));
   if (field == NULL)
      return "";

   char *buf = Con::getReturnBuffer(1024);
   dSprintf(buf, 1024, "%s\t%s\t%s", field->mFieldName, field->mFieldType, field->mDefaultValue);

   return buf;
}

ConsoleMethod(Component, getBehaviorFieldUserData, const char *, 3, 3, "(int index) - Gets the UserData associated with a field by index in the field list\n"
   "@param index The index of the behavior\n"
   "@return Returns a string representing the user data of this field\n")
{
   ComponentField *field = object->getComponentField(dAtoi(argv[2]));
   if (field == NULL)
      return "";

   return field->mUserData;
}

ConsoleMethod(Component, getComponentFieldDescription, const char *, 3, 3, "(int index) - Gets a field description by index\n"
   "@param index The index of the behavior\n"
   "@return Returns a string representing the description of this field\n")
{
   ComponentField *field = object->getComponentField(dAtoi(argv[2]));
   if (field == NULL)
      return "";

   return field->mFieldDescription ? field->mFieldDescription : "";
}

ConsoleMethod(Component, addDependency, void, 3, 3, "(string behaviorName) - Gets a field description by index\n"
   "@param index The index of the behavior\n"
   "@return Returns a string representing the description of this field\n")
{
   object->addDependency(argv[2]);
}

ConsoleMethod(Component, setDirty, void, 2, 2, "() - Gets a field description by index\n"
   "@param index The index of the behavior\n"
   "@return Returns a string representing the description of this field\n")
{
   object->setMaskBits(Component::OwnerMask);
}
