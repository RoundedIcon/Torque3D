//-----------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
return;
//Register this to the editor
if(isFile("tools/main.cs"))
	BehaviorEditor::registerComponentType("AmbientAnimation", "AnimationComponent", 
				"Ambient Animation", "Animation", "Plays a specific animation on a loop");
	
function AmbientAnimation::onAdd(%this)
{
   %this.friendlyName = "Ambient Animation";
   %this.behaviorType = "Animation";
   %this.description  = "Plays a specific animation on a loop";
   
   %this.networked = true;
   
   %this.addBehaviorField(animation, "What animation to play", "animationList", "");

   %this.playThread(1, %this.animation);
}

function AmbientAnimation::onInspectorUpdate(%this, %field)
{
   %this.playThread(1, %this.animation);
}