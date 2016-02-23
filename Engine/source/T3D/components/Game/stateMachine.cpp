#include "T3D/Components/Game/stateMachine.h"

StateMachine::StateMachine()
{
   //mCurrentState = NULL;
   mStateStartTime = -1;
   mStateTime = 0;

   mStartingState = "";

   mCurCreateState = NULL;
}

StateMachine::~StateMachine()
{
}

void StateMachine::loadStateMachineFile()
{
   if (!mXMLReader)
   {
      SimXMLDocument *xmlrdr = new SimXMLDocument();
      xmlrdr->registerObject();

      mXMLReader = xmlrdr;
   }

   bool hasStartState = false;

   if (!dStrIsEmpty(mStateMachineFile))
   {
      //use our xml reader to parse the file!
      SimXMLDocument *reader = mXMLReader.getObject();
      if (!reader->loadFile(mStateMachineFile))
         Con::errorf("Could not load state machine file: &s", mStateMachineFile);

      if (!reader->pushFirstChildElement("StateMachine"))
         return;

      //find our starting state
      if (reader->pushFirstChildElement("StartingState"))
      {
         mStartingState = reader->getData();
         reader->popElement();
         hasStartState = true;
      }

      readStates();
   }

   if (hasStartState)
      mCurrentState = getStateByName(mStartingState);

   mStateStartTime = -1;
   mStateTime = 0;
}

void StateMachine::readStates()
{
   SimXMLDocument *reader = mXMLReader.getObject();

   //iterate through our states now!
   if (reader->pushFirstChildElement("State"))
   {
      //get our first state
      State firstState;

      readStateName(&firstState, reader);
      readStateScriptFunction(&firstState, reader);

      readTransitions(firstState);

      mStates.push_back(firstState);

      //now, iterate the siblings
      while (reader->nextSiblingElement("State"))
      {
         State newState;
         readStateName(&newState, reader);
         readStateScriptFunction(&newState, reader);

         readTransitions(newState);

         mStates.push_back(newState);
      }

      //reader->popElement();
   }
}

void StateMachine::readTransitions(State &currentState)
{
   SimXMLDocument *reader = mXMLReader.getObject();

   //iterate through our states now!
   if (reader->pushFirstChildElement("Transition"))
   {
      //get our first state
      StateTransition firstTransition;

      readTransitonTarget(&firstTransition, reader);

      readConditions(firstTransition);

      currentState.mTransitions.push_back(firstTransition);

      //now, iterate the siblings
      while (reader->nextSiblingElement("Transition"))
      {
         StateTransition newTransition;
         readTransitonTarget(&newTransition, reader);

         readConditions(newTransition);

         currentState.mTransitions.push_back(newTransition);
      }

      reader->popElement();
   }
}

void StateMachine::readConditions(StateTransition &currentTransition)
{
   SimXMLDocument *reader = mXMLReader.getObject();

   //iterate through our states now!
   if (reader->pushFirstChildElement("Rule"))
   {
      //get our first state
      StateTransition::Condition firstCondition;
      StateField firstField;
      bool fieldRead = false;
      
      readFieldName(&firstField, reader);
      firstCondition.field = firstField;

      readFieldComparitor(&firstCondition, reader);

      readFieldValue(&firstCondition.field, reader);

      currentTransition.mTransitionRules.push_back(firstCondition);

      //now, iterate the siblings
      while (reader->nextSiblingElement("Transition"))
      {
         StateTransition::Condition newCondition;
         StateField newField;

         readFieldName(&newField, reader);
         newCondition.field = newField;

         readFieldComparitor(&newCondition, reader);

         readFieldValue(&newCondition.field, reader);

         currentTransition.mTransitionRules.push_back(newCondition);
      }

      reader->popElement();
   }
}

S32 StateMachine::parseComparitor(const char* comparitorName)
{
   S32 targetType = -1;

   if (!dStrcmp("GreaterThan", comparitorName))
      targetType = StateMachine::StateTransition::Condition::GeaterThan;
   else if (!dStrcmp("GreaterOrEqual", comparitorName))
      targetType = StateMachine::StateTransition::Condition::GreaterOrEqual;
   else if (!dStrcmp("LessThan", comparitorName))
      targetType = StateMachine::StateTransition::Condition::LessThan;
   else if (!dStrcmp("LessOrEqual", comparitorName))
      targetType = StateMachine::StateTransition::Condition::LessOrEqual;
   else if (!dStrcmp("Equals", comparitorName))
      targetType = StateMachine::StateTransition::Condition::Equals;
   else if (!dStrcmp("True", comparitorName))
      targetType = StateMachine::StateTransition::Condition::True;
   else if (!dStrcmp("False", comparitorName))
      targetType = StateMachine::StateTransition::Condition::False;
   else if (!dStrcmp("Negative", comparitorName))
      targetType = StateMachine::StateTransition::Condition::Negative;
   else if (!dStrcmp("Positive", comparitorName))
      targetType = StateMachine::StateTransition::Condition::Positive;
   else if (!dStrcmp("DoesNotEqual", comparitorName))
      targetType = StateMachine::StateTransition::Condition::DoesNotEqual;

   return targetType;
}

void StateMachine::update()
{
   /*if (mCurrentState == NULL)
   {
      //if we don't have a state, pick our default state
      if (!dStrcmp(mStartingState, ""))
         mCurrentState = getStateByName(getStateByIndex(0));
      else
         mCurrentState = getStateByName(mStartingState);

      if (mCurrentState == NULL)
         return;
      else
         mStateStartTime = Sim::getCurrentTime();
   }*/
   //we always check if there's a timout transition, as that's the most generic transition possible.
   F32 curTime = Sim::getCurrentTime();

   if (mStateStartTime == -1)
      mStateStartTime = curTime;

   mStateTime = curTime - mStateStartTime;

   char buffer[64];
   dSprintf(buffer, sizeof(buffer), "%g", mStateTime);

   checkTransitions("stateTime", buffer);
}

void StateMachine::checkTransitions(const char* slotName, const char* newValue)
{
   //because we use our current state's fields as dynamic fields on the instance
   //we'll want to catch any fields being set so we can treat changes as transition triggers if
   //any of the transitions on this state call for it

   //One example would be in order to implement burst fire on a weapon state machine.
   //The behavior instance has a dynamic variable set up like: GunStateMachine.burstShotCount = 0;

   //We also have a transition in our fire state, as: GunStateMachine.addTransition("FireState", "burstShotCount", "DoneShooting", 3);
   //What that does is for our fire state, we check the dynamicField burstShotCount if it's equal or greater than 3. If it is, we perform the transition.

   //As state fields are handled as dynamicFields for the instance, regular dynamicFields are processed as well as state fields. So we can use the regular 
   //dynamic fields for our transitions, to act as 'global' variables that are state-agnostic. Alternately, we can use state-specific fields, such as a transition
   //like this:
   //GunStateMachine.addTransition("IdleState", "Fidget", "Timeout", ">=", 5000);

   //That uses the the timeout field, which is reset each time the state changes, and so state-specific, to see if it's been 5 seconds. If it has been, we transition
   //to our fidget state

   //so, lets check our current transitions
   //if (mCurrentState)
   {
      //now that we have the type, check our transitions!
      for (U32 t = 0; t < mCurrentState.mTransitions.size(); t++)
      {
         //if (!dStrcmp(mCurrentState.mTransitions[t]., slotName))
         {
            //found a transition looking for this variable, so do work
            //first, figure out what data type thie field is
            //S32 type = getVariableType(newValue);

            bool fail = false;
            bool match = false;
            S32 ruleCount = mCurrentState.mTransitions[t].mTransitionRules.size();

            for (U32 r = 0; r < ruleCount; r++)
            {
               const char* fieldName = mCurrentState.mTransitions[t].mTransitionRules[r].field.name;
               if (!dStrcmp(fieldName, slotName))
               {
                  match = true;
                  //now, check the value with the comparitor and see if we do the transition.
                  if (!passComparitorCheck(newValue, mCurrentState.mTransitions[t].mTransitionRules[r]))
                  {
                     fail = true;
                     break;
                  }
               }
            }

            //If we do have a transition rule for this field, and we didn't fail on the condition, go ahead and switch states
            if (match && !fail)
            {
               setState(mCurrentState.mTransitions[t].mStateTarget);

               return;
            }
         }
      }
   }
}

bool StateMachine::passComparitorCheck(const char* var, StateTransition::Condition transitionRule)
{
   F32 num = dAtof(var);
   switch (transitionRule.field.fieldType)
   {
   case StateField::Type::VectorType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::Equals:
      case StateTransition::Condition::GeaterThan:
      case StateTransition::Condition::GreaterOrEqual:
      case StateTransition::Condition::LessThan:
      case StateTransition::Condition::LessOrEqual:
      case StateTransition::Condition::DoesNotEqual:
         //do
         break;
      default:
         return false;
      };
   case StateField::Type::StringType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::Equals:
         if (!dStrcmp(var, transitionRule.field.triggerStringVal))
            return true;
         else
            return false;
      case StateTransition::Condition::DoesNotEqual:
         if (dStrcmp(var, transitionRule.field.triggerStringVal))
            return true;
         else
            return false;
      default:
         return false;
      };
   case StateField::Type::BooleanType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::TriggerValueTarget::True:
         if (dAtob(var))
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::False:
         if (dAtob(var))
            return false;
         else
            return true;
      default:
         return false;
      };
   case StateField::Type::NumberType:
      switch (transitionRule.triggerComparitor)
      {
      case StateTransition::Condition::TriggerValueTarget::Equals:
         if (num == transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::GeaterThan:
         if (num > transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::GreaterOrEqual:
         if (num >= transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::LessThan:
         if (num < transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::LessOrEqual:
         if (num <= transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::DoesNotEqual:
         if (num != transitionRule.field.triggerNumVal)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::Positive:
         if (num > 0)
            return true;
         else
            return false;
      case StateTransition::Condition::TriggerValueTarget::Negative:
         if (num < 0)
            return true;
         else
            return false;
      default:
         return false;
      };
   default:
      return false;
   };
}

void StateMachine::setState(const char* stateName, bool clearFields)
{
   State oldState = mCurrentState;
   StringTableEntry sName = StringTable->insert(stateName);
   for (U32 i = 0; i < mStates.size(); i++)
   {
      //if(!dStrcmp(mStates[i]->stateName, stateName))
      if (!dStrcmp(mStates[i].stateName,sName))
      {
         mCurrentState = mStates[i];
         mStateStartTime = Sim::getCurrentTime();

         onStateChanged.trigger(this, i);

         //clear the current dynamic fields from the old state
         //we have a variable here so we can override the clearing of fields if needed.
         //This allows for 'compound states', where some elements from each state are active.
         //This isn't normally used, but look at the playerController behavior's setup to see
         //it in action with the stances system
         /*if (clearFields)
         {
            if (oldState)
            {
               for (U32 s = 0; s < oldState->mProperties.size(); s++)
               {
                  setDataField(oldState->mProperties[s].name, NULL, "");
               }
            }
         }*/

         //now add the new state's fields as dynamic fields
         /*for (U32 p = 0; p < mCurrentState.mProperties.size(); p++)
         {
            setDataField(mCurrentState.mProperties[p].name, NULL, mCurrentState.mProperties[p].value);
         }

         onStateChange_callback();*/

         //if we have a callback function for this state, call it now
         //if (mCurrentState.callbackName != "")
          //  Con::executef(this, mCurrentState.callbackName);

         return;
      }
   }
}

const char* StateMachine::getStateByIndex(S32 index)
{
   if (index >= 0 && mStates.size() > index)
      return mStates[index].stateName;
   else
      return "";
}

StateMachine::State& StateMachine::getStateByName(const char* name)
{
   StringTableEntry stateName = StringTable->insert(name);

   for (U32 i = 0; i < mStates.size(); i++)
   {
      if (!dStrcmp(stateName, mStates[i].stateName))
         return mStates[i];
   }

   //return NULL;
}

S32 StateMachine::findFieldByName(const char* name)
{
   for (U32 i = 0; i < mFields.size(); i++)
   {
      if (!dStrcmp(mFields[i].name, name))
         return i;
   }

   return -1;
}