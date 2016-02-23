registerComponent("SpectatorSpawner", "Component", 
			"Spectator Spawner", "Game", false, "When a client connects, it spawns a spectator camera for them and attaches them to it");

function SpectatorSpawner::onAdd(%this)
{
	%this.clientCount = 1;
}

function SpectatorSpawner::onClientConnect(%this, %client)
{
	//Currently, map requires 1 pre-existing
	//make an entity for them!
	
	%playerPrefab = new Prefab()
	{
		filename = "art/prefabs/SpectatorObject.prefab";
	};
	
	%playerObj = %playerPrefab.getObject(0);

	%playerObj.position = %this.owner.position;
	
	MissionCleanup.add(%playerObj);
	//Missiongroup.add(%playerObj);
	
	switchControlObject(%client, %playerObj);
	switchCamera(%client, %playerObj);
	
	%playerObj.getComponent(SpectatorControls).setupControls(%client);
	
	%this.clientCount++;
}

function SpectatorSpawner::onClientDisConnect(%this, %client)
{
	
}

function SpectatorSpawner::getClientID(%this)
{
	return ClientGroup.getObject(%this.clientOwner-1);
}