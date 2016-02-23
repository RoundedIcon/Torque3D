function ScriptedTriggerObject::onAdd(%this)
{

}


function ScriptedTriggerObject::onRemove(%this)
{

}

function ScriptedTriggerObject::onCollisionEvent(%this, %colObject, %colNormal, %colPoint, %colMatID, %velocity)
{
    echo(%this @ " collided with " @ %colObject);
}
