//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

if(!isObject(SGOManager))
{
	new SimXMLDocument(SGOManager)
	{
		File = "data/EC/scripts/gameObjects/GameObjectList.xml";
		Count=0;
	};
}

function SGOManager::clear(%this)
{
	for(%i=0; %i < %this.Count; %i++)
	{
		%this.Objects[%i].delete();
	}
	
	%this.Count = 0;
}

function SGOManager::resetToTop(%this)
{
	%this.clear();
	
	if(!%this.loadFile(%this.File))
	{
		errorf("SGOManager: Unable to read Game Objects List file!");
		return;
	}
	
	%this.pushChildElement(0);
}

function SGOManager::pushBack(%this, %name, %tamlPath, %scriptPath)
{
	%this.Objects[%this.Count] = new ArrayObject();
		
	%this.Objects[%this.Count].add("Name", %name);
	%this.Objects[%this.Count].add("TAMLPath", %tamlPath);
	%this.Objects[%this.Count].add("ScriptPath", %scriptPath);
	
	%this.Count++;
}

function SGOManager::readGameObjects(%this)
{
	%this.resetToTop();
	
	%this.pushChildElement(0);
	
	while(true)
	{
		//get our name
		%this.pushChildElement(0);
		%name = %this.getData();
		//%this.popElement();
		
		%this.nextSiblingElement("TAMLPath");
		%tamlPath = %this.getData();
		//%this.popElement();
		
		%this.nextSiblingElement("ScriptPath");
		%scriptPath = %this.getData();
		%this.popElement();
		
		%this.pushBack(%name, %tamlPath, %scriptPath);
		
		if (!%this.nextSiblingElement("GameObject")) 
			break;
	}
	
	%this.reset();
	
	echo("SGOManager: read" SPC %this.Count SPC " Game Objects");
}

function SGOManager::saveGameObject(%this, %name, %tamlPath, %scriptPath, %updateMatching)
{
	if(%updateMatching $= "")
		%updateMatching = false;
		
	%this.resetToTop();
	
	%this.pushChildElement("GameObject");
	
	%i=0;
	
	//we don't want duplicates, so parse through our current list, and see if we already are in here
	%foundExisting = false;
	while(true)
	{
		//get our name
		%this.pushChildElement("Name");
		%GOname = %this.getData();
		
		if(%GOname $= %name)
		{
			//update it?
			if(%updateMatching == true)
			{
				%this.Objects[%i].setValue("TamlPath", %tamlPath);
				%this.Objects[%i].setValue("ScriptPath", %scriptPath);
				
				echo("SGOManager: Updated Game Object" SPC %name);
			}
			else
			{
				echo("SGOManager: Found duplicate Game Object" SPC %name);
			}
			
			%foundExisting = true;
			break;
		}
		%this.popElement();
		
		%i++;
		
		if (!%this.nextSiblingElement("GameObject")) 
			break;
	}
	
	if(%foundExisting == false)
	{
		%this.resetToTop();
		
		%this.pushNewElement("GameObject");
		
		%this.pushNewElement("Name");
		%this.addData(%name);
		
		%this.addNewElement("TamlPath");
		%this.addData(%tamlPath);
		
		%this.addNewElement("ScriptPath");
		%this.addData(%scriptPath);
		
		echo("SGOManager: Added new Game Object" SPC %name);
	}
	
	%this.saveFile(%this.File);
	
	%this.reset();
}

function SGOManager::execGameObjects(%this)
{
	for(%i=0; %i < %this.Count; %i++)
	{
		exec(%this.Objects[%i].getValue(%this.Objects[%i].getIndexFromKey("scriptPath")));
	}
}

function SGOManager::spawn(%this, %name, %addToMissionGroup)
{
	if(%addToMissionGroup $= "")
		%addToMissionGroup = true;
		
	for ( %i = 0; %i < %this.Count; %i++ )
	{
		%nameIndex = %this.Objects[%i].getIndexFromKey("Name");
		%objName = %this.Objects[%i].getValue(%nameIndex);
		if(%objName $= %name)
		{
			%tamlIndex = %this.Objects[%i].getIndexFromKey("TAMLPath");
			%tamlPath = %this.Objects[%i].getValue(%tamlIndex);
			%newSGOObject = TamlRead(%tamlPath);
			
			if(%addToMissionGroup == true)
				MissionGroup.add(%newSGOObject);
				
			return %newSGOObject;
		}
	}	
	
	return 0;
}