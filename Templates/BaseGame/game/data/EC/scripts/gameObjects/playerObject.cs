function playerObject::onAdd(%this)
{

}


function playerObject::onRemove(%this)
{

}

function playerObject::moveVectorEvent(%this)
{
    %moveVec = %this.getMoveVector();

    %cam = %this.getComponent("CameraComponent");

    %phys = %this.getComponent("PlayerControllerComponent");

    if(%moveVec.x != 0)
    {
        %rv = %cam.getRightVector();
      
        %forMove = VectorAdd(VectorScale(%rv, %moveVec.x), %forMove);

        %phys.inputVelocity.x = %moveVec.x;
    }
    else
    {
        %phys.inputVelocity.x = 0;
    }

    if(%moveVec.y != 0)
    {
        %fv = %cam.getForwardVector();

        %forMove = VectorAdd(VectorScale(%fv, %moveVec.y), %forMove);

        %phys.inputVelocity.y = %moveVec.y;
    }
    else
    {
        %phys.inputVelocity.y = 0;
    }
}

function playerObject::moveYawEvent(%this)
{
    %moveRot = %this.getMoveRotation();

    %this.rotation.z += %moveRot.z * 20;
}

function playerObject::movePitchEvent(%this)
{
    %moveRot = %this.getMoveRotation();

    %cam = %this.getComponent("CameraComponent");

    %cam.rotationOffset.x += %moveRot.x * 20;
}

function playerObject::moveRollEvent(%this)
{
}

function playerObject::moveTriggerEvent(%this, %triggerNum, %triggerValue)
{
   if(%triggerNum == 3 && %triggerValue == true)
   {
      echo("Doing the flashlight thing.");
      if(%this.getObject(0).isEnabled)
        %this.getObject(0).setLightEnabled(false);
      else
        %this.getObject(0).setLightEnabled(true);
   }
}

function playerObject::onCollisionEvent(%this, %colObject, %colNormal, %colPoint, %colMatID, %velocity)
{
    //echo(%this @ " collided with " @ %colObject);
}