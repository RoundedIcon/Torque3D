//-----------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------

registerComponent("", "VisibilityTriggerComponent", "Visibility Trigger", "Trigger", false, "Allows the behavior owner to operate as a camera.");

function VisibilityTriggerComponent::onAdd(%this) 
{
   Parent::onBehaviorAdd(%this);

   %clientCount = ClientGroup.getCount();

   for(%i=0; %i < %clientCount; %i++)
   {
	   %this.addClient(ClientGroup.getObject(%i));
   }
}

/*function VisibilityTrigger::onRemove(%this)
{
   %clientID = %this.getClientID();
   if(%clientID)
      %clientID.clearCameraObject();
}*/

function VisibilityTriggerComponent::onClientConnect(%this, %client)
{
   %this.addClient(%client);
}

function VisibilityTriggerComponent::onClientDisconnect(%this, %client)
{
   Parent::onClientDisconnect(%this, %client);
   
   %this.removeClient(%client);
}