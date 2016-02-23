//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "gui/buttons/guiIconButtonCtrl.h"
#include "gui/editor/guiInspector.h"
#include "gui/editor/inspector/componentGroup.h"
//#include "gui/editor/inspector/ComponentField.h"
#include "core/strings/stringUnit.h"
//#include "T3D/Entity.h"
#include "T3D/Components/Component.h"
#include "gui/editor/inspector/field.h"
#include "console/engineAPI.h"

IMPLEMENT_CONOBJECT(GuiInspectorComponentGroup);

ConsoleDocClass( GuiInspectorComponentGroup,
   "@brief Used to inspect an object's FieldDictionary (dynamic fields) instead "
   "of regular persistent fields.\n\n"
   "Editor use only.\n\n"
   "@internal"
);

bool GuiInspectorComponentGroup::onAdd()
{
   if(!Parent::onAdd())
      return false;
}

//-----------------------------------------------------------------------------
// GuiInspectorComponentGroup - add custom controls
//-----------------------------------------------------------------------------
bool GuiInspectorComponentGroup::createContent()
{
   if(!Parent::createContent())
      return false;

   // Create our field stack control
   //mStack = new GuiSimpleStackCtrl();

   // Prefer GuiTransperantProfile for the stack.
   /*mStack->setDataField( StringTable->insert("profile"), NULL, "GuiInspectorStackProfile" );
   if( !mStack->registerObject("InspectorBehaviorStack") )
   {
      SAFE_DELETE( mStack );
      return false;
   }

   //addObject( mStack );
   //mStack->setField( "padding", "4" );
   //mStack->mExtendParent = true;

   //give the necessary padding for the nested controls so it looks nice.
   setMargin(RectI(4,0,4,4));

   Con::evaluatef( "%d.stack = %d;", this->getId(), mStack->getId() );*/

   //Con::executef( this, "createContent", "" );

   // encapsulate the button in a dummy control.
   /*GuiControl* shell = new GuiControl();
   shell->setDataField( StringTable->insert("profile"), NULL, "GuiTransparentProfile" );
   if( !shell->registerObject() )
   {
      delete shell;
      return false;
   }

   // add a button that lets us add new dynamic fields.
   GuiBitmapButtonCtrl* addBehaviorBtn = new GuiBitmapButtonCtrl();
   {
      SimObject* profilePtr = Sim::findObject("InspectorDynamicFieldButton");
      if( profilePtr != NULL )
         addBehaviorBtn->setControlProfile( dynamic_cast<GuiControlProfile*>(profilePtr) );
		
		// FIXME Hardcoded image
      addBehaviorBtn->setBitmap("tools/gui/images/iconAdd.png");

      //char commandBuf[64];
      //dSprintf(commandBuf, 64, "%d.addComponent();", this->getId());
      //addBehaviorBtn->setField("command", commandBuf);
      addBehaviorBtn->setSizing(horizResizeRight,vertResizeCenter);
      //addFieldBtn->setField("buttonMargin", "2 2");
      addBehaviorBtn->resize(Point2I(0,2), Point2I(16, 16));
      addBehaviorBtn->registerObject("AddBehaviorButton");
   }

   mAddBhvrList = new GuiPopUpMenuCtrlEx();
   {
      SimObject* profilePtr = Sim::findObject("GuiPopUpMenuProfile");
      if( profilePtr != NULL )
         mAddBhvrList->setControlProfile( dynamic_cast<GuiControlProfile*>(profilePtr) );

	  // Configure it to update our value when the popup is closed
	  //char szBuffer[512];
	  //dSprintf( szBuffer, 512, "%d.apply( %d.getText() );", getId(), addBehaviorList->getId() );
	  //addBehaviorList->setField("Command", szBuffer );

	  //now add the entries, allow derived classes to override this
	  //_populateMenu( addBehaviorList );

	  // Select the active item, or just set the text field if that fails
	  //S32 id = addBehaviorList->findText(getData());
	  //if (id != -1)
	  //  addBehaviorList->setSelected(id, false);
	  //else
	  //  addBehaviorList->setField("text", getData());

	  mAddBhvrList->setSizing(horizResizeWidth,vertResizeCenter);
	  mAddBhvrList->resize(Point2I(addBehaviorBtn->getPosition().x + addBehaviorBtn->getExtent().x + 2 ,2), Point2I(getWidth() - 16, 16));
      mAddBhvrList->registerObject("eBehaviorList");
   }

   shell->resize(Point2I(0,0), Point2I(getWidth(), 28));
   shell->addObject(addBehaviorBtn);
   shell->addObject(mAddBhvrList);

   // save off the shell control, so we can push it to the bottom of the stack in inspectGroup()
   mAddCtrl = shell;
   mStack->addObject(shell);*/

   return true;
}

//-----------------------------------------------------------------------------
// GuiInspectorComponentGroup - inspectGroup override
//-----------------------------------------------------------------------------
bool GuiInspectorComponentGroup::inspectGroup()
{
   // We can't inspect a group without a target!
   if (!mParent || !mParent->getNumInspectObjects())
      return false;

   // to prevent crazy resizing, we'll just freeze our stack for a sec..
   mStack->freeze(true);

   bool bNoGroup = false;

   // Un-grouped fields are all sorted into the 'general' group
   if (dStricmp(mCaption, "General") == 0)
      bNoGroup = true;

   // Just delete all fields and recreate them (like the dynamicGroup)
   // because that makes creating controls for array fields a lot easier
   clearFields();

   bool bNewItems = false;
   bool bMakingArray = false;
   GuiStackControl *pArrayStack = NULL;
   GuiRolloutCtrl *pArrayRollout = NULL;
   bool bGrabItems = false;

   Component* comp = dynamic_cast<Component*>(getInspector()->getInspectObject(0));

   //if this isn't a component, what are we even doing here?
   if (!comp)
      return false;

   for (U32 i = 0; i < comp->getComponentFieldCount(); i++)
   {
      ComponentField* field = comp->getComponentField(i);

      bNewItems = true;

      GuiInspectorField *fieldGui = constructField(field->mFieldType);
      if (fieldGui == NULL)
         fieldGui = new GuiInspectorField();

      fieldGui->init(mParent, this);

      AbstractClassRep::Field *refField;
      //check statics
      refField = const_cast<AbstractClassRep::Field *>(comp->findField(field->mFieldName));
      if (!refField)
      {
         //check dynamics
         SimFieldDictionary* fieldDictionary = comp->getFieldDictionary();
         SimFieldDictionaryIterator itr(fieldDictionary);

         while (*itr)
         {
            SimFieldDictionary::Entry* entry = *itr;
            if (entry->slotName == field->mFieldName)
            {
               AbstractClassRep::Field f;
               f.pFieldname = StringTable->insert(field->mFieldName);

               if (field->mFieldDescription)
                  f.pFieldDocs = field->mFieldDescription;

               f.type = field->mFieldType;
               f.offset = -1;
               f.elementCount = 1;
               f.validator = NULL;
               f.flag = 0; //change to be the component type

               f.setDataFn = &defaultProtectedSetFn;
               f.getDataFn = &defaultProtectedGetFn;
               f.writeDataFn = &defaultProtectedWriteFn;

               if (!dStrcmp(field->mGroup, ""))
                  f.pGroupname = "Component";
               else
                  f.pGroupname = field->mGroup;

               ConsoleBaseType* conType = ConsoleBaseType::getType(field->mFieldType);
               AssertFatal(conType, "ConsoleObject::addField - invalid console type");
               f.table = conType->getEnumTable();

               tempFields.push_back(f);

               refField = &f;

               break;
            }
            ++itr;
         }
      }

      if (!refField)
         continue;

      fieldGui->setInspectorField(&tempFields[tempFields.size() - 1]);

      if (fieldGui->registerObject())
      {
#ifdef DEBUG_SPEW
         Platform::outputDebugString("[GuiInspectorGroup] Adding field '%s'",
            field->pFieldname);
#endif

         mChildren.push_back(fieldGui);
         mStack->addObject(fieldGui);
      }
      else
      {
         SAFE_DELETE(fieldGui);
      }
   }

   mStack->freeze(false);
   mStack->updatePanes();

   // If we've no new items, there's no need to resize anything!
   if (bNewItems == false && !mChildren.empty())
      return true;

   sizeToContents();

   setUpdate();

   return true;
}

void GuiInspectorComponentGroup::updateAllFields()
{
   // We overload this to just reinspect the group.
   inspectGroup();
}

void GuiInspectorComponentGroup::onMouseMove(const GuiEvent &event)
{
	//mParent->mOverDivider = false;
}
ConsoleMethod(GuiInspectorComponentGroup, inspectGroup, bool, 2, 2, "Refreshes the dynamic fields in the inspector.")
{
   return object->inspectGroup();
}

void GuiInspectorComponentGroup::clearFields()
{
   tempFields.clear();
   // save mAddCtrl
   //Sim::getGuiGroup()->addObject(mAddCtrl);
   // delete everything else
   mStack->clear();
   // clear the mChildren list.
   mChildren.clear();
   // and restore.
   //mStack->addObject(mAddCtrl);
}

SimFieldDictionary::Entry* GuiInspectorComponentGroup::findDynamicFieldInDictionary( StringTableEntry fieldName )
{
   SimFieldDictionary * fieldDictionary = mParent->getInspectObject()->getFieldDictionary();

   for(SimFieldDictionaryIterator ditr(fieldDictionary); *ditr; ++ditr)
   {
      SimFieldDictionary::Entry * entry = (*ditr);

      if( entry->slotName == fieldName )
         return entry;
   }

   return NULL;
}

void GuiInspectorComponentGroup::addDynamicField()
{
   // We can't add a field without a target
   /*if( !mStack )
   {
      Con::warnf("GuiInspectorComponentGroup::addDynamicField - no target SimObject to add a dynamic field to.");
      return;
   }

   // find a field name that is not in use. 
   // But we wont try more than 100 times to find an available field.
   U32 uid = 1;
   char buf[64] = "dynamicField";
   SimFieldDictionary::Entry* entry = findDynamicFieldInDictionary(buf);
   while(entry != NULL && uid < 100)
   {
      dSprintf(buf, sizeof(buf), "dynamicField%03d", uid++);
      entry = findDynamicFieldInDictionary(buf);
   }
   
   const U32 numTargets = mParent->getNumInspectObjects();
   if( numTargets > 1 )
      Con::executef( mParent, "onBeginCompoundEdit" );

   for( U32 i = 0; i < numTargets; ++ i )
   {
      SimObject* target = mParent->getInspectObject( i );
      
      Con::evaluatef( "%d.dynamicField = \"defaultValue\";", target->getId(), buf );
 
      // Notify script.
   
      Con::executef( mParent, "onFieldAdded", target->getIdString(), buf );
   }
   
   if( numTargets > 1 )
      Con::executef( mParent, "onEndCompoundEdit" );

   // now we simply re-inspect the object, to see the new field.
   inspectGroup();
   instantExpand();*/
}

AbstractClassRep::Field* GuiInspectorComponentGroup::findObjectBehaviorField(Component* target, String fieldName)
{
   AbstractClassRep::FieldList& fieldList = target->getClassRep()->mFieldList;
   for( AbstractClassRep::FieldList::iterator itr = fieldList.begin();
		itr != fieldList.end(); ++ itr )
   {
	  AbstractClassRep::Field* field = &( *itr );
	  String fldNm(field->pFieldname);
	  if(fldNm == fieldName)
		  return field;
   }
   return NULL;
}

ConsoleMethod( GuiInspectorComponentGroup, addDynamicField, void, 2, 2, "obj.addDynamicField();" )
{
   object->addDynamicField();
}

ConsoleMethod( GuiInspectorComponentGroup, removeDynamicField, void, 3, 3, "" )
{
}
