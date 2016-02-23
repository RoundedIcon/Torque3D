#include "T3D/tagLibrary.h"

TagLibrary* TagLibrary::smSingleton = NULL;

bool TagLibrary::activate()
{
   smSingleton = new TagLibrary();

   return true;
}

void TagLibrary::addEntity(SceneObject* newEntity)
{
   TaggedEntity newTaggedEntity;

   newTaggedEntity.object = newEntity;

   mTaggedEntities.push_back(newTaggedEntity);
}

void TagLibrary::removeEntity(SceneObject* newEntity)
{
   for (U32 i = 0; i < mTaggedEntities.size(); i++)
   {
      if (mTaggedEntities[i].object.getObject() == newEntity)
      {
         mTaggedEntities.erase(i);
         return;
      }
   }
}

TagLibrary::TaggedEntity* TagLibrary::findEntity(SceneObject* newEntity)
{
   for (U32 i = 0; i < mTaggedEntities.size(); i++)
   {
      if (mTaggedEntities[i].object.getObject() == newEntity)
      {
         return &mTaggedEntities[i];
      }
   }

   return NULL;
}

//
S32 TagLibrary::addTag(const char* tagName)
{
   StringTableEntry name = StringTable->insert(tagName);

   for (U32 i = 0; i < mTags.size(); i++)
   {
      if (mTags[i] == name)
         return i; //it's already in here, we don't need to do anything else
   }

   mTags.push_back(name);

   return mTags.size() - 1;
}

void TagLibrary::removeTag(const char* tagName)
{
   StringTableEntry name = StringTable->insert(tagName);

   for (U32 i = 0; i < mTags.size(); i++)
   {
      if (mTags[i] == name)
      {
         mTags.erase(i);

         //next, update the tagged entities' tag indexes

         return;
      }
   }
}

S32 TagLibrary::findTag(const char* tagName)
{
   StringTableEntry name = StringTable->insert(tagName);

   for (U32 i = 0; i < mTags.size(); i++)
   {
      if (mTags[i] == name)
      {
         return i;
      }
   }

   return -1;
}

StringTableEntry TagLibrary::getTag(S32 index)
{
   if (index < 0 || index > mTags.size())
      return "";

   return mTags[index];
}

bool TagLibrary::addTagToEntity(SceneObject* targetEntity, S32 tagIndex)
{
   TaggedEntity* taggedEntity = findEntity(targetEntity);
   if (taggedEntity)
   {
      //found it!
      //check that we haven't assigned the tag to this already
      if (taggedEntity->tags.find_next(tagIndex) == -1)
      {
         taggedEntity->tags.push_back(tagIndex);
         return true;
      }
   }

   return false;
}

bool TagLibrary::addTagToEntity(SceneObject* targetEntity, const char* tag)
{
   TaggedEntity* taggedEntity = findEntity(targetEntity);
   if (taggedEntity)
   {
      //see if this tag actually exists or not
      S32 tagIdx = findTag(tag);

      if (tagIdx == -1)
      {
         //doesn't currently exist, so add it
         tagIdx = addTag(tag);
      }

      //check that we haven't assigned the tag to this already
      if (taggedEntity->tags.find_next(tagIdx) == -1)
      {
         taggedEntity->tags.push_back(tagIdx);
         return true;
      }
   }

   return false;
}

bool TagLibrary::remmoveTagFromEntity(SceneObject* targetEntity, S32 tagIndex)
{
   TaggedEntity* taggedEntity = findEntity(targetEntity);
   if (taggedEntity)
   {
      //found it!
      //check that we haven't assigned the tag to this already
      S32 foundTagIdx = taggedEntity->tags.find_next(tagIndex);

      if (foundTagIdx != -1)
      {
         taggedEntity->tags.erase(foundTagIdx);
         return true;
      }
   }

   return false;
}

bool TagLibrary::remmoveTagFromEntity(SceneObject* targetEntity, const char* tag)
{
   TaggedEntity* taggedEntity = findEntity(targetEntity);
   if (taggedEntity)
   {
      //see if this tag actually exists or not
      S32 tagIdx = findTag(tag);

      if (tagIdx == -1)
      {
         //doesn't currently exist, so add it
         tagIdx = addTag(tag);
      }

      //check that we haven't assigned the tag to this already
      S32 foundTagIdx = taggedEntity->tags.find_next(tagIdx);

      if (foundTagIdx != -1)
      {
         taggedEntity->tags.erase(foundTagIdx);
         return true;
      }
   }

   return false;
}

bool TagLibrary::doesEntityHaveTag(SceneObject* targetEntity, const char* tag)
{
   TaggedEntity* taggedEntity = findEntity(targetEntity);
   if (taggedEntity)
   {
      //found it!

      S32 tagIdx = findTag(tag);
      
      //check that we haven't assigned the tag to this already
      if (taggedEntity->tags.find_next(tagIdx) != -1)
      {
         return true;
      }
   }

   return false;
}