//-----------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
registerComponent("", "CameraComponent", "Camera", "Game", true, "Allows the behavior owner to operate as a camera.");

function CameraComponent::onAdd(%this) 
{
   Parent::onBehaviorAdd(%this);

   %this.addComponentField(clientOwner, "The client that views this camera", "int", "1", "");

   %test = %this.clientOwner;

   %barf = ClientGroup.getCount();

   %clientID = %this.getClientID();
   if(%clientID && !isObject(%clientID.camera))
   {
      %this.scopeToClient(%clientID);
      %this.setDirty();

      %clientID.setCameraObject(%this.owner);
      %clientID.setControlCameraFov(%this.FOV);
      
      %clientID.camera = %this.owner;
   }

   %res = $pref::Video::mode;
   %derp = 0;
}

function CameraComponent::onRemove(%this)
{
   %clientID = %this.getClientID();
   if(%clientID)
      %clientID.clearCameraObject();
}

function CameraComponent::onInspectorUpdate(%this)
{
   //if(%this.clientOwner)
      //%this.clientOwner.setCameraObject(%this.owner);
}

function CameraComponent::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}

function CameraComponent::isClientCamera(%this, %client)
{
	%clientID = ClientGroup.getObject(%this.clientOwner-1);
	
	if(%client.getID() == %clientID)
		return true;
	else
	    return false;
}

function CameraComponent::onClientConnect(%this, %client)
{
   //if(%this.isClientCamera(%client) && !isObject(%client.camera))
   //{
      %this.scopeToClient(%client);
      %this.setDirty();
      
      %client.setCameraObject(%this.owner);
      %client.setControlCameraFov(%this.FOV);
      
      %client.camera = %this.owner;
   //}
   //else
   //{
   //   echo("CONNECTED CLIENT IS NOT CAMERA OWNER!");
   //}
}

function CameraComponent::onClientDisconnect(%this, %client)
{
   Parent::onClientDisconnect(%this, %client);
   
   if(isClientCamera(%client)){
      %this.clearScopeToClient(%client);
      %client.clearCameraObject();
   }
}

//move to the editor later
GlobalActionMap.bind("keyboard", "alt c", "toggleEditorCam");

function switchCamera(%client, %newCamEntity)
{
	if(!isObject(%client) || !isObject(%newCamEntity))
		return error("SwitchCamera: No client or target camera!");
		
	%cam = %newCamEntity.getComponent(CameraComponent);
		
	if(!isObject(%cam))
		return error("SwitchCamera: Target camera doesn't have a camera behavior!");
		
	//TODO: Cleanup clientOwner for previous camera!
	if(%cam.clientOwner == 0 || %cam.clientOwner $= "")
		%cam.clientOwner = 0;
		
	%cam.scopeToClient(%client);
	%cam.setDirty();
	
	%client.setCameraObject(%newCamEntity);
	%client.setControlCameraFov(%cam.FOV);
	
	%client.camera = %newCamEntity;
}

function buildEditorCamera()
{
	if(isObject("EditorCamera"))
		return EditorCamera;
		
    %camObj = SGOManager.spawn("SpectatorObject", false);
	
	%camObj.name = "EditorCamera";
	
	%client = ClientGroup.getObject(0);
	
	%camObj.getComponent(SpectatorControls).setupControls(%client);
	
	MissionCleanup.add(%camObj);
	
	return %camObj;
}

//TODO: Move this somewhere else!
function toggleEditorCam(%val)
{
   if(!%val)
      return;
      
   %client = ClientGroup.getObject(0);

   if(!isObject(%client.camera))
      return error("ToggleEditorCam: no existing camera!");
   
   %editorCam = buildEditorCamera();

   //if this is our first switch, just go to the editor camera
   if(%client.lastCam $= "" || %client.camera.getId() != %editorCam.getId())
   {
	   if(%client.lastCam $= "")
	   {
	      //set up the position
		  %editorCam.position = %client.camera.position;
		  %editorCam.rotation = %client.camera.rotation;
	   }
	
	   %client.lastCam = %client.camera;
	   %client.lastController = %client.getControlObject();
	   switchCamera(%client, %editorCam); 
	   switchControlObject(%client, %editorCam);
   }
   else  
   {
       switchCamera(%client, %client.lastCam); 
	   switchControlObject(%client, %client.lastController); 
       %client.lastCam = %editorCam;
	   %client.lastController = %editorCam;
   }
}

function serverCmdSetClientAspectRatio(%client, %width, %height)
{
   echo("Client: " @ %client SPC "changing screen res to: " @ %width SPC %height);
   %client.screenExtent = %width SPC %height;
   %cam = %client.getCameraObject();
   
   if(!isObject(%cam))
      return;
      
   %cameraComp = %cam.getComponent(CameraComponent);

   %cameraComp.ScreenAspect = %width SPC %height;
}