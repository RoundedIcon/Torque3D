//-----------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
registerComponent("SpectatorControls", "Component", "Spectator Controls", "Input", false, "First Person Shooter-type controls.");

function SpectatorControls::onAdd(%this)
{
   Parent::onBehaviorAdd(%this);

   //
	%this.beginGroup("Keys");
		%this.addComponentField(forwardKey, "Key to bind to vertical thrust", keybind, "keyboard w");
		%this.addComponentField(backKey, "Key to bind to vertical thrust", keybind, "keyboard s");
		%this.addComponentField(leftKey, "Key to bind to horizontal thrust", keybind, "keyboard a");
		%this.addComponentField(rightKey, "Key to bind to horizontal thrust", keybind, "keyboard d");
		
		%this.addComponentField(jump, "Key to bind to horizontal thrust", keybind, "keyboard space");
	%this.endGroup();
	
	%this.beginGroup("Mouse");
		%this.addComponentField(pitchAxis, "Key to bind to horizontal thrust", keybind, "mouse yaxis");
		%this.addComponentField(yawAxis, "Key to bind to horizontal thrust", keybind, "mouse xaxis");
	%this.endGroup();
	
	%this.addComponentField(moveSpeed, "Horizontal thrust force", float, 20.0);
	%this.addComponentField(jumpStrength, "Vertical thrust force", float, 3.0);
   //

   %control = %this.owner.getComponent( ControlObjectComponent );
   if(!%control)
   	   return echo("SPECTATOR CONTROLS: No Control Object behavior!");
	
	%this.setupControls(%control.getClientID());
}

function SpectatorControls::onRemove(%this)
{
   Parent::onBehaviorRemove(%this);
   
   %control = %this.owner.getComponent( ControlObjectComponent );
   if(!%control)
   	   return echo("SPECTATOR CONTROLS: No Control Object behavior!");

   commandToClient(%control.clientOwnerID, 'removeSpecCtrlInput', %this.forwardKey);
   commandToClient(%control.clientOwnerID, 'removeSpecCtrlInput', %this.backKey);
   commandToClient(%control.clientOwnerID, 'removeSpecCtrlInput', %this.leftKey);
   commandToClient(%control.clientOwnerID, 'removeSpecCtrlInput', %this.rightKey);
   
   commandToClient(%control.clientOwnerID, 'removeSpecCtrlInput', %this.pitchAxis);
   commandToClient(%control.clientOwnerID, 'removeSpecCtrlInput', %this.yawAxis);
}

function SpectatorControls::onInspectorUpdate(%this, %field)
{
   %controller = %this.owner.getComponent( ControlObjectComponent );
   if(%controller)
      commandToClient(%controller.getClientID(), 'updateSpecCtrlInput', %this.getFieldValue(%field), %field);
}

function SpectatorControls::onClientConnect(%this, %client)
{
   %this.setupControls(%client);
}

function SpectatorControls::setupControls(%this, %client)
{
   %control = %this.owner.getComponent( ControlObjectComponent );
   if(!%control.isControlClient(%client))
   {
      echo("SPECTATOR CONTROLS: Client Did Not Match");
      return;
   }
   
   %inputCommand = "SpectatorControls";

   %test = %this.forwardKey;
   
   /*SetInput(%client, %this.forwardKey.x,  %this.forwardKey.y,  %inputCommand@"_forwardKey");
   SetInput(%client, %this.backKey.x,     %this.backKey.y,     %inputCommand@"_backKey");
   SetInput(%client, %this.leftKey.x,     %this.leftKey.y,     %inputCommand@"_leftKey");
   SetInput(%client, %this.rightKey.x,    %this.rightKey.y,    %inputCommand@"_rightKey");
   
   SetInput(%client, %this.jump.x,        %this.jump.y,        %inputCommand@"_jump");
      
   SetInput(%client, %this.pitchAxis.x,   %this.pitchAxis.y,   %inputCommand@"_pitchAxis");
   SetInput(%client, %this.yawAxis.x,     %this.yawAxis.y,     %inputCommand@"_yawAxis");*/

   SetInput(%client, "keyboard",  "w",  %inputCommand@"_forwardKey");
   SetInput(%client, "keyboard",  "s",     %inputCommand@"_backKey");
   SetInput(%client, "keyboard",  "a",     %inputCommand@"_leftKey");
   SetInput(%client, "keyboard",  "d",    %inputCommand@"_rightKey");
   
   SetInput(%client, "keyboard",  "space",        %inputCommand@"_jump");
      
   SetInput(%client, "mouse",   "yaxis",   %inputCommand@"_pitchAxis");
   SetInput(%client, "mouse",   "xaxis",     %inputCommand@"_yawAxis");
 
}

function SpectatorControls::onMoveTrigger(%this, %triggerID)
{
   //check if our jump trigger was pressed!
   if(%triggerID == 2)
   {
      %this.owner.applyImpulse("0 0 0", "0 0 " @ %this.jumpStrength);
   }
}

function SpectatorControls::Update(%this)
{
   %moveVector = %this.owner.getMoveVector();
   %moveRotation = %this.owner.getMoveRotation();

   if(%moveVector.x)
   {
      %fv = VectorNormalize(%this.owner.getRightVector());
      
      %forMove = VectorScale(%fv, (%moveVector.x * (%this.moveSpeed * 0.032)));
      
      %this.owner.position = VectorAdd(%this.owner.position, %forMove);
   }
   
   if(%moveVector.y)
   {
      %fv = VectorNormalize(%this.owner.getForwardVector());
      
      %forMove = VectorScale(%fv, (%moveVector.y * (%this.moveSpeed * 0.032)));
      
      %this.owner.position = VectorAdd(%this.owner.position, %forMove);
   }
   
   /*if(%moveVector.z)
   {
      %fv = VectorNormalize(%this.owner.getUpVector());
      
      %forMove = VectorScale(%fv, (%moveVector.z * (%this.moveSpeed * 0.032)));
      
      %this.Physics.velocity = VectorAdd(%this.Physics.velocity, %forMove);
   }*/
   
   //eulerRotation is managed in degrees for human-readability. 
   if(%moveRotation.x != 0)
      %this.owner.rotation.x += mRadToDeg(%moveRotation.x);
   
   if(%moveRotation.z != 0)
      %this.owner.rotation.z += mRadToDeg(%moveRotation.z);
}

//
function SpectatorControls_forwardKey(%val){
   $mvForwardAction = %val;
}

function SpectatorControls_backKey(%val){
   $mvBackwardAction = %val;
}

function SpectatorControls_leftKey(%val){
   $mvLeftAction = %val;
}

function SpectatorControls_rightKey(%val){
   $mvRightAction = %val;
}

function SpectatorControls_yawAxis(%val){
   %deg = getMouseAdjustAmount(%val);
   $mvYaw += getMouseAdjustAmount(%val);
}

function SpectatorControls_pitchAxis(%val){
   %deg = getMouseAdjustAmount(%val);
   $mvPitch += getMouseAdjustAmount(%val);
}

function SpectatorControls_jump(%val){
   $mvTriggerCount2++;
}

function NamingCheck()
{
	echo("Component");
	echo(Component);
}