//-----------------------------------------------------------------------------
// Torque Game Builder
// Copyright (C) GarageGames.com, Inc.
//-----------------------------------------------------------------------------
function AnimationComponent::onAdd(%this)
{
	%this.networked = true;
    %this.friendlyName = "Animation Controller";
    %this.behaviorType = "Animation";
    %this.description  = "Manages animations for an entity";
}

function AnimationComponent::onBehaviorAdd(%this)
{
   		
}

function AnimationComponent::onStateChange(%this)
{
		
}

function AnimationComponent::onAnimationEnd(%this, %animation)
{
   //if(%this.owner.isMethod("onEvent"))
   //   %this.owner.onEvent("animationEnd", %animation);
}

function AnimationComponent::onAnimationTrigger(%this, %animation)
{
  // if(%this.owner.isMethod("onEvent"))
  //    %this.owner.onEvent("animationTrigger", %animation);
}