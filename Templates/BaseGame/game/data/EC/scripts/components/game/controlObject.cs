//-----------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
registerComponent("ControlObjectComponent", "Component", "Control Object", "Game", false, "Allows the behavior owner to operate as a camera.");

function ControlObjectComponent::onAdd(%this)
{
   Parent::onBehaviorAdd(%this);

   %this.addComponentField(clientOwner, "The shape to use for rendering", "int", "1", "");

   %clientID = %this.getClientID();

   if(%clientID && !isObject(%clientID.getControlObject()))
      %clientID.setControlObject(%this.owner);
}

function ControlObjectComponent::onRemove(%this)
{
   %clientID = %this.getClientID();
	
   if(%clientID)
      %clientID.setControlObject(0);
}

function ControlObjectComponent::onClientConnect(%this, %client)
{
   if(%this.isControlClient(%client) && !isObject(%client.getControlObject()))
      %client.setControlObject(%this.owner);
}

function ControlObjectComponent::onClientDisconnect(%this, %client)
{
   if(%this.isControlClient(%client))
      %client.setControlObject(0);
}

function ControlObjectComponent::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}

function ControlObjectComponent::isControlClient(%this, %client)
{
	%clientID = ClientGroup.getObject(%this.clientOwner-1);
	
	if(%client.getID() == %clientID)
		return true;
	else
	    return false;
}

function ControlObjectComponent::onInspectorUpdate(%this, %field)
{
   %clientID = %this.getClientID();
	
   if(%clientID && !isObject(%clientID.getControlObject()))
      %clientID.setControlObject(%this.owner);
}

function switchControlObject(%client, %newControlEntity)
{
	if(!isObject(%client) || !isObject(%newControlEntity))
		return error("SwitchControlObject: No client or target controller!");
		
	%control = %newControlEntity.getComponent(ControlObjectComponent);
		
	if(!isObject(%control))
		return error("SwitchControlObject: Target controller has no conrol object behavior!");
		
    %client.setControlObject(%newControlEntity);
}