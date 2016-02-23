registerComponent("PlayerSpawner", "Component", 
			"Player Spawner", "Game", false, "When a client connects, it spawns a player object for them and attaches them to it");

function PlayerSpawner::onAdd(%this)
{
	%this.clientCount = 1;
	%this.friendlyName = "Player Spawner";
	%this.componentType = "Spawner";
	
	%this.addComponentField("GameObjectName", "The name of the game object we spawn for the players", string, "PlayerObject");
}

function PlayerSpawner::onClientConnect(%this, %client)
{
	%playerObj = SGOManager.spawn(%this.GameObjectName);
	
	if(!isObject(%playerObj))
		return;

	%playerObj.position = %this.owner.position;
	
	MissionCleanup.add(%playerObj);
	
	for(%b = 0; %b < %playerObj.getComponentCount(); %b++)
    {
       %comp = %playerObj.getComponentByIndex(%b);

	   if(%comp.isMethod("onClientConnect"))
         %comp.onClientConnect(%client);
    }
	
	switchControlObject(%client, %playerObj);
	switchCamera(%client, %playerObj);
	
	//%playerObj.getComponent(FPSControls).setupControls(%client);
	
	%this.clientCount++;
}

function PlayerSpawner::onClientDisConnect(%this, %client)
{
	
}

function PlayerSpawner::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}