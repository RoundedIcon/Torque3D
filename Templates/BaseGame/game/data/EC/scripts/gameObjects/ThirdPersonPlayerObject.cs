function ThirdPersonPlayerObject::onAdd(%this)
{
   %this.turnRate = 0.3;
/*
   %this.phys = %this.getComponent("PlayerControllerComponent");
   %this.collision = %this.getComponent("CollisionComponent");
   %this.cam = %this.getComponent("CameraComponent");
   %this.camArm = %this.getComponent("CameraOrbiterComponent");
   %this.animation = %this.getComponent("AnimationComponent");
   %this.stateMachine = %this.getComponent("StateMachineComponent");
   %this.mesh = %this.getComponent("MeshComponent");
*/
   %this.stateMachine.forwardVector = 0;

   %this.crouch = false;
   
   %this.firstPerson = false;
   
   %this.crouchSpeedMod = 0.5;
   
   %this.aimOrbitDist = 1.5;
   %this.regularOrbitDist = 5;
   
   %this.regularOrbitMaxPitch = 70;
   %this.regularOrbitMinPitch = -10;
   
   %this.aimedMaxPitch = 90;
   %this.aimedMinPitch = -90;
   
   %this.airControl = 0.3;
   
   %this.arms = SGOManager.spawn("FirstPersonArms", true);
   
   %this.add(arms);
   
   %this.pending = true;
   
   %this.counter = 0;
   
   //%this.camOrb = %this.camArm;
   
   //%this.mesh.mountObject(%this.arms, "Eye");
}

function ThirdPersonPlayerObject::onRemove(%this)
{

}

function ThirdPersonPlayerObject::moveVectorEvent(%this)
{
    %moveVector = %this.getMoveVector();
    
    //echo(%moveVector);

    // forward of the camera on the x-z plane
    %cameraForward = %this.cam.getForwardVector();

    %cameraRight = %this.cam.getRightVector();

    %moveVec = VectorAdd(VectorScale(%cameraRight, %moveVector.x), VectorScale(%cameraForward, %moveVector.y));

   if(%this.aiming || %this.firstPerson)
   {
      %forMove = "0 0 0";
      
      if(%moveVector.x != 0)
      {
         %this.phys.inputVelocity.x = %moveVector.x * 10;
      }
      else
      {
         %this.phys.inputVelocity.x = 0;
      }

      if(%moveVector.y != 0)
      {

         %this.phys.inputVelocity.y = %moveVector.y * 10;
      }
      else
      {
         %this.phys.inputVelocity.y = 0;
      }
   }
   else
   {
      if(%moveVec.x == 0 && %moveVec.y == 0)
      {
         %this.phys.inputVelocity = "0 0 0";
         %this.stateMachine.forwardVector = 0;
      }
      else
      {
         %moveVec.z = 0;

         %curForVec = %this.getForwardVector();

         %newForVec = VectorLerp(%curForVec, %moveVec, %this.turnRate);

         %this.setForwardVector(%newForVec);
         
         %this.phys.inputVelocity.y = 10;

         %this.stateMachine.forwardVector = 1;
      }
   }
   
   //%this.phys.setInputVelocity(%moveVec * 10);
   
   if(%this.crouch)
      %this.phys.inputVelocity = VectorScale(%this.phys.inputVelocity, %this.crouchSpeedMod);
}

function ThirdPersonPlayerObject::moveYawEvent(%this)
{
   %moveRotation = %this.getMoveRotation();
    
    if(%this.aiming || %this.firstPerson)
    {
      %this.rotation.z += %moveRotation.z * 10;
    }

    %this.camArm.rotation.z += %moveRotation.z * 10;
}

function ThirdPersonPlayerObject::movePitchEvent(%this)
{
   %moveRotation = %this.getMoveRotation();
   
   //%camOrb = %this.camArm;

   %this.camArm.rotation.x += %moveRotation.x * 10;
}

function ThirdPersonPlayerObject::moveRollEvent(%this){}

function ThirdPersonPlayerObject::moveTriggerEvent(%this, %triggerNum, %triggerValue)
{
   if(%triggerNum == 3 && %triggerValue)
   {
      if(%triggerValue)
      {
        %this.firstPerson = !%this.firstPerson;
        
        if(%this.firstPerson)
        {
            %this.rotation.z = %this.cam.rotationOffset.z;
            %this.camArm.orbitDistance = 0;
            %this.camArm.maxPitchAngle = %this.aimedMaxPitch;
            %this.camArm.minPitchAngle = %this.aimedMinPitch;
            
            %this.cam.positionOffset = "0 0 0";
            %this.cam.rotationOffset = "0 0 0";
        }
        else if(%this.aiming)
        {
            %this.camArm.orbitDistance = %this.aimOrbitDist;
            
            %this.camArm.maxPitchAngle = %this.aimedMaxPitch;
            %this.camArm.minPitchAngle = %this.aimedMinPitch;
        }
        else
        {
            %this.camArm.orbitDistance = %this.regularOrbitDist;
            
            %this.camArm.maxPitchAngle = %this.regularOrbitMaxPitch;
            %this.camArm.minPitchAngle = %this.regularOrbitMinPitch;
        }
        
        commandToClient(localclientConnection, 'SetClientRenderShapeVisibility', 
            localclientConnection.getGhostID(%this.getComponent("MeshComponent")), !%this.firstPerson);
      }
   }
	else if(%triggerNum == 2 && %triggerValue == true)
	{
	   //get our best collision assuming up is 0 0 1
	   %collisionAngle = %this.collision.getBestCollisionAngle("0 0 1");
	   
	   if(%collisionAngle >= 80)
	   {
	      %surfaceNormal = %this.collision.getCollisionNormal(0);
	      %jumpVector = VectorScale(%surfaceNormal, 200);
	      %jumpVector = VectorAdd(%jumpVector, "0 0 300");
	      echo("Jump surface Angle is at: " @ %surfaceNormal);
	      
	      %this.phys.applyImpulse(%this.position, %jumpVector);
	      %this.setForwardVector(%jumpVector.x SPC %jumpVector.y SPC "0");
	   }
      else
      {
         %this.phys.applyImpulse(%this.position, "0 0 300");
         echo(%collisionAngle);
      }
	}
	else if(%triggerNum == 4)
	{
      %this.crouch = %triggerValue;
	}
	else if(%triggerNum == 1)
	{
	   %this.aiming = %triggerValue;  
	   
	   if(%this.aiming)
      {
         %this.rotation.z = %this.cam.rotationOffset.z;
         %this.camArm.orbitDistance = %this.aimOrbitDist;
         %this.camArm.maxPitchAngle = %this.aimedMaxPitch;
         %this.camArm.minPitchAngle = %this.aimedMinPitch;
      }
      else
      {
         %this.camArm.orbitDistance = %this.regularOrbitDist;
         %this.camArm.maxPitchAngle = %this.regularOrbitMaxPitch;
         %this.camArm.minPitchAngle = %this.regularOrbitMinPitch;
      }
	}
}

function ThirdPersonPlayerObject::onCollisionEvent(%this, %colObject, %colNormal, %colPoint, %colMatID, %velocity)
{
   if(!%this.phys.isContacted())
    echo(%this @ " collided with " @ %colObject);
}

function ThirdPersonPlayerObject::updateEnt(%this)
{
   %moveVec = %this.getMoveVector();
   %bestFit = "";
   if(%this.crouch)
   {
      if(%moveVec.x != 0 || %moveVec.y != 0)
         %bestFit = "Crouch_Forward";
      else
         %bestFit = "Crouch_Root";
   }
   else
   {
      if(%moveVec.x != 0 || %moveVec.y != 0)
         %bestFit = "Run";
      else
         %bestFit = "Root";
   }

   if(%this.animation.getThreadAnimation(0) !$= %bestFit)
      %this.animation.playThread(0, %bestFit);
      
   //%camOrb = %this.getComponent("CameraOrbiterComponent");
   //%camOrb.rotation.z += 0.1;
   if (%this.isMethod( thingy ))
      %this.thingy();
}

//Used for first person mode
function clientCmdSetClientRenderShapeVisibility(%id, %visiblilty)
{
   %localID = ServerConnection.resolveGhostID(%id); 
   %localID.enabled = %visiblilty;
}

function serverToClientObject( %serverObject )
{
   assert( isObject( LocalClientConnection ), "serverToClientObject() - No local client connection found!" );
   assert( isObject( ServerConnection ), "serverToClientObject() - No server connection found!" );      
         
   %ghostId = LocalClientConnection.getGhostId( %serverObject );
   if ( %ghostId == -1 )
      return 0;
                
   return ServerConnection.resolveGhostID( %ghostId );   
}

function ThirdPersonPlayerObject::writePacket(%this)
{
   //$hi += 1;
   //%this.write($hi);
   
   %this.counter += 1;
   if (%this.writeFlag(%this.counter > 20 && %this.pending))
   {
      %this.counter = 25;
      %this.pending = false;
      %this.writeString("thing");
   }
   //if(%this.counter > 22)
   //   warn("something should be here by now");
      
   return false;
}

function ThirdPersonPlayerObject::readPacket(%this)
{
   //%this.methodTest();
   //%hi = %this.read();
   //echo(%hi);
   
   if(%this.readFlag())
   {
      warn("did the thing");
      %this.namespaced = %this.readString();
      %this.addNameSpace(%this.namespaced);
   }
}

$pos= "0 0 0";
$rot= "0 0 0";
   
function ThirdPersonPlayerObject::updateMove(%this, %move, %acc, %conNorm, %falling, %watercov, %mvel)
{
   //%moveVector = %this.getMoveVector();
   
   %moveVec = VectorRot(%move, -%this.getRotation().z);

   %this.phys.setAccel(VectorAdd(%acc, VectorScale(%moveVec, 1)));
   
   //$diff = VectorSub($pos, %this.getPosition());
   //$diffr = VectorSub($rot, %this.getRotation());
   //if (VectorLen($diff) > 0 || VectorLen($diffr) > 0)
   //{
   //   echo("Position change :" SPC $diff);
   //   echo("Rotation change :" SPC $diffr);
   //}

   //echo($diffr);
   //$pos = %this.getPosition();
   //$rot = %this.getRotation();
}

function thing::thingy()
{
   // echo(1);
}

function randomclass::Update()
{
   %this.owner.getComponent( PlayerControllerComponent ).velocity = getRandom() SPC getRandom() SPC "0";
}

function physicstime(%num)
{
   while(%num > 0)
   {
      new physicsplayerexample(){position = "6 -24 50" ; movevector = 1*(getrandom()-0.5) SPC 1*(getrandom()-0.5) SPC 50*getRandom();};
      //%obj = SGOManager.spawn("ThirdPersonPlayerObject", false);
      //%obj.position = "6 -24 5";
      
      //%comp = new Component()
      //{
      //   networked="false"; while(%i < 100){%thing = new physicsexample(){}; %thing.setPosition(getRandom() * 5 SPC getRandom() * 20 SPC getRandom() * 6 + 100); %thing.setVelocity( getRandom() SPC getRandom() SPC getRandom() * 100); %i++;}
      //   enabled="true";
      //   class="randomclass";
      //};
      //%this.addcomponent(%comp);
      //%this.addNameSpace("aiPerson");
      %num--;
   }//15.1973 -35.9251 -63.9015
   //%this.addNameSpace(%this.namespaced);
}