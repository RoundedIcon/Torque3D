#ifndef _TAG_LIBRARY_H_
#define _TAG_LIBRARY_H_

#ifndef _CONSOLEINTERNAL_H_
#include "console/consoleInternal.h"
#endif
#ifndef _SIMOBJECT_H_
#include "console/simObject.h"
#endif
#ifndef _ENTITY_H_
#include "T3D/Entity.h"
#endif

class Entity;

class TagLibrary
{
   typedef SimObject Parent;

   /// The current active physics plugin.
   static TagLibrary* smSingleton;

   struct TaggedEntity
   {
      SimObjectPtr<SceneObject> object;
      Vector<S32> tags;
   };

   //This is the global list of the tags we have.
   Vector<StringTableEntry> mTags;

   //Note, this should only really ever be used in the case where we're searching for entities with a given tag, or if we have to update our tag IDs. 
   Vector<TaggedEntity> mTaggedEntities;

public:
   S32 buildTextField(String fieldText, RectI bounds);

   S32 addTag(const char* tagName);
   void removeTag(const char* tagName);

   S32 findTag(const char* tagName);
   StringTableEntry getTag(S32 index);

   void addEntity(SceneObject* newEntity);
   void removeEntity(SceneObject* newEntity);
   TaggedEntity* findEntity(SceneObject* newEntity);

   bool addTagToEntity(SceneObject* targetEntity, S32 tagIndex);
   bool addTagToEntity(SceneObject* targetEntity, const char* tag);

   bool remmoveTagFromEntity(SceneObject* targetEntity, S32 tagIndex);
   bool remmoveTagFromEntity(SceneObject* targetEntity, const char* tag);

   bool doesEntityHaveTag(SceneObject* targetEntity, const char* tag);

   static TagLibrary* getSingleton() 
   { 
      if (!smSingleton)
         TagLibrary::activate();

      return smSingleton; 
   }

   static bool activate();
};

#define TAGLIBRARY TagLibrary::getSingleton()

#endif