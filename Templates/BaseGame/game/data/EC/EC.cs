
// The general flow of a gane - server's creation, loading and hosting clients, and then destruction is as follows:

// First, a client will always create a server in the event that they want to host a single player
// game. Torque3D treats even single player connections as a soft multiplayer game, with some stuff
// in the networking short-circuited to sidestep around lag and packet transmission times.

// initServer() is called, loading the default server scripts.
// After that, if this is a dedicated server session, initDedicated() is called, otherwise initClient is called
// to prep a playable client session.

// When a local game is started - a listen server - via calling StartGame() a server is created and then the client is
// connected to it via createAndConnectToLocalServer().

function EC::create( %this )
{
   //relevent game script execution
   new SimSet(ComponentSet);
   
   for( %file = findFirstFile( "data/EC/scripts/*.cs" );
   %file !$= "";
   %file = findNextFile( "data/EC/scripts/*.cs" ))
   {
      exec(%file);
   }
   
   //Load game objects
   SGOManager.readGameObjects();
   
   //add DBs
   if(isObject(DatablockFilesList))
   {
      for( %file = findFirstFile( "data/EC/scripts/datablocks/*.cs.dso" );
      %file !$= "";
      %file = findNextFile( "data/EC/scripts/datablocks/*.cs.dso" ))
      {
         // Only execute, if we don't have the source file.
         %csFileName = getSubStr( %file, 0, strlen( %file ) - 4 );
         if( !isFile( %csFileName ) )
            DatablockFilesList.add(%csFileName);
      }
      
      // Load all source material files.
      for( %file = findFirstFile( "data/EC/scripts/datablocks/*.cs" );
      %file !$= "";
      %file = findNextFile( "data/EC/scripts/datablocks/*.cs" ))
      {
         DatablockFilesList.add(%file);
      }
   }
   
   if(isObject(LevelFilesList))
   {
      for( %file = findFirstFile( "data/EC/levels/*.mis" );
      %file !$= "";
      %file = findNextFile( "data/EC/levels/*.mis" ))
      {
         LevelFilesList.add(%file);
      }
   }
   
   if (!$Server::Dedicated)
   {
      //client scripts
      $KeybindPath = "data/EC/scripts/client/default.keybinds.cs";
      exec($KeybindPath);
      
      %prefPath = getPrefpath();
      if(isFile(%prefPath @ "/keybinds.cs"))
         exec(%prefPath @ "/keybinds.cs");
         
      exec("data/EC/scripts/client/inputCommands.cs");
      
      //guis
      exec("./scripts/gui/playGui.gui");
      exec("./scripts/gui/playGui.cs");
   }
}

function EC::destroy( %this )
{
   
}

function registerComponent(%componentName, %componentClass, %friendlyName, %componentType, /*%networked,*/ %description)
{
   if($ComponentEditor::componentTypesCount $= "")
      $ComponentEditor::componentTypesCount = 1;
   else
      $ComponentEditor::componentTypesCount++;
   
   $ComponentEditor::componentTypes[$ComponentEditor::componentTypesCount] = %componentName 
      TAB %componentClass TAB %friendlyName TAB %componentType TAB %description;
   
   ComponentSet.add(%componentName);
}