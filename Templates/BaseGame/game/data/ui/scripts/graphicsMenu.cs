// =============================================================================
// GRAPHICS MENU
// =============================================================================
//Mesh and Textures
//
function GraphicsMenu::onWake(%this)
{
    %this.settingsPage = 0;
}

function GraphicsMenuOKButton::onClick(%this)
{
    //save the settings and then back out
    GraphicsMenu.apply();
    OptionsMenu.backOut();
}

function GraphicsMenu::loadSettings(%this)
{
   %this.loadDisplayOptions();
   %this.loadLightingOptions();
   %this.loadMeshesAndTexturesOptions();
   %this.loadShaderOptions();
   %this.changeSettingsPage();
}

function GraphicsMenu::changeSettingsPage(%this)
{
    %lastPageIndex = 3;

    if(%this.settingsPage < 0)
        %this.settingsPage = 0;
        
    else if(%this.settingsPage > %lastPageIndex)
        %this.settingsPage = %lastPageIndex;

    if(%this.settingsPage == 0)
    {
        GraphisMenuPageText.setText("Display");

        ScreenSettingsMenu.hidden = false;
        GraphicsLightingSettingsMenu.hidden = true;
        GraphicsShaderSettingsMenu.hidden = true;
        GraphicsMeshTexSettingsMenu.hidden = true;
    }
    if(%this.settingsPage == 1)
    {
        GraphisMenuPageText.setText("Lighting");

        ScreenSettingsMenu.hidden = true;
        GraphicsLightingSettingsMenu.hidden = false;
        GraphicsShaderSettingsMenu.hidden = true;
        GraphicsMeshTexSettingsMenu.hidden = true;
    }
    else if(%this.settingsPage == 2)
    {
        GraphisMenuPageText.setText("Meshes and Textures");
        
        ScreenSettingsMenu.hidden = true;
        GraphicsLightingSettingsMenu.hidden = true;
        GraphicsShaderSettingsMenu.hidden = true;
        GraphicsMeshTexSettingsMenu.hidden = false;
    }
    else if(%this.settingsPage == 3)
    {
        GraphisMenuPageText.setText("Shaders");
        
        ScreenSettingsMenu.hidden = true;
        GraphicsLightingSettingsMenu.hidden = true;
        GraphicsShaderSettingsMenu.hidden = false;
        GraphicsMeshTexSettingsMenu.hidden = true;
    }
}

//
function GraphicsMenu::Autodetect(%this)
{
   $pref::Video::autoDetect = false;
   
   %shaderVer = getPixelShaderVersion();
   %intel = ( strstr( strupr( getDisplayDeviceInformation() ), "INTEL" ) != -1 ) ? true : false;
   %videoMem = GFXCardProfilerAPI::getVideoMemoryMB();
   
   return %this.Autodetect_Apply( %shaderVer, %intel, %videoMem );
}

function GraphicsMenu::Autodetect_Apply(%this, %shaderVer, %intel, %videoMem )
{
   if ( %shaderVer < 2.0 )
   {      
      return "Your video card does not meet the minimum requirment of shader model 2.0.";
   }
   
   if ( %shaderVer < 3.0 || %intel )
   {
      // Allow specular and normals for 2.0a and 2.0b
      if ( %shaderVer > 2.0 )
      {
         MeshQualityGroup-->Lowest.apply();
         TextureQualityGroup-->Lowest.apply();
         GroundCoverDensityGroup-->Lowest.apply();
         DecalLifetimeGroup-->None.apply();
         TerrainQualityGroup-->Lowest.apply();
         
         ShadowQualityList-->None.apply();
         ShadowDistanceList-->Lowest.apply();   
         SoftShadowList-->Off.apply();
         ShadowCacheList-->On.apply();
         LightDistanceList-->Lowest.apply();
         
         ShaderQualityGroup-->Normal.apply();
         ReflectionsQualityGroup-->Off.apply();
         ParallaxQualityGroup-->Off.apply();
         HDRQualityGroup-->Off.apply();
         SSAOQualityGroup-->Off.apply();
         LightRaysQualityGroup-->Off.apply();
         DOFQualityGroup-->Off.apply();
         VignetteQualityGroup-->Off.apply();   
      }
      else
      {
         MeshQualityGroup-->Lowest.apply();
         TextureQualityGroup-->Lowest.apply();
         GroundCoverDensityGroup-->Lowest.apply();
         DecalLifetimeGroup-->None.apply();
         TerrainQualityGroup-->Lowest.apply();
         
         ShadowQualityList-->None.apply();
         ShadowDistanceList-->Lowest.apply();   
         SoftShadowList-->Off.apply();
         ShadowCacheList-->On.apply();
         LightDistanceList-->Lowest.apply();
         
         ShaderQualityGroup-->Low.apply();
         ReflectionsQualityGroup-->Off.apply();
         ParallaxQualityGroup-->Off.apply();
         HDRQualityGroup-->Off.apply();
         SSAOQualityGroup-->Off.apply();
         LightRaysQualityGroup-->Off.apply();
         DOFQualityGroup-->Off.apply();
         VignetteQualityGroup-->Off.apply();
      }
   }   
   else
   {
      if ( %videoMem > 1000 )
      {
         MeshQualityGroup.applySetting("High");
         TextureQualityGroup.applySetting("High");
         GroundCoverDensityGroup.applySetting("High");
         DecalLifetimeGroup.applySetting("High");
         TerrainQualityGroup.applySetting("High");
         
         ShadowQualityList.applySetting("High");
         ShadowDistanceList.applySetting("Highest");   
         SoftShadowList.applySetting("High");
         ShadowCacheList.applySetting("On");
         LightDistanceList.applySetting("Highest");
         
         ShaderQualityGroup.applySetting("Normal");
         ReflectionsQualityGroup.applySetting("On");
         ParallaxQualityGroup.applySetting("On");
         HDRQualityGroup.applySetting("On");
         SSAOQualityGroup.applySetting("On");
         LightRaysQualityGroup.applySetting("On");
         DOFQualityGroup.applySetting("On");
         VignetteQualityGroup.applySetting("On");
      }
      else if ( %videoMem > 400 || %videoMem == 0 )
      {
         MeshQualityGroup-->Medium.apply();
         TextureQualityGroup-->Medium.apply();
         GroundCoverDensityGroup-->Medium.apply();
         DecalLifetimeGroup-->Medium.apply();
         TerrainQualityGroup-->Medium.apply();
         
         ShadowQualityList-->Medium.apply();
         ShadowDistanceList-->Medium.apply();   
         SoftShadowList-->Low.apply();
         ShadowCacheList-->On.apply();
         LightDistanceList-->Medium.apply();
         
         ShaderQualityGroup-->Normal.apply();
         ReflectionsQualityGroup-->On.apply();
         ParallaxQualityGroup-->Off.apply();
         HDRQualityGroup-->On.apply();
         SSAOQualityGroup-->Off.apply();
         LightRaysQualityGroup-->On.apply();
         DOFQualityGroup-->On.apply();
         VignetteQualityGroup-->On.apply();
         
         if ( %videoMem == 0 )
            return "Torque was unable to detect available video memory. Applying 'Medium' quality.";
      }
      else
      {
         MeshQualityGroup-->Low.apply();
         TextureQualityGroup-->Low.apply();
         GroundCoverDensityGroup-->Low.apply();
         DecalLifetimeGroup-->Low.apply();
         TerrainQualityGroup-->Low.apply();
         
         ShadowQualityList-->Low.apply();
         ShadowDistanceList-->Low.apply();   
         SoftShadowList-->Low.apply();
         ShadowCacheList-->On.apply();
         LightDistanceList-->Low.apply();
         
         ShaderQualityGroup-->Normal.apply();
         ReflectionsQualityGroup-->On.apply();
         ParallaxQualityGroup-->Off.apply();
         HDRQualityGroup-->Off.apply();
         SSAOQualityGroup-->Off.apply();
         LightRaysQualityGroup-->Off.apply();
         DOFQualityGroup-->Off.apply();
         VignetteQualityGroup-->Off.apply();
      }
   }
   
   echo("Exporting client prefs");
   %prefPath = getPrefpath();
   export("$pref::*", %prefPath @ "/clientPrefs.cs", false);
   
   return "Graphics quality settings have been auto detected.";
}

//
function GraphicsMenu::loadDrawDistanceOptions(%this)
{
   %option = OptionsMenu.addSliderOption();
   %option-->nameText.setText("View Distance");
   %option-->slider.variable = "$pref::Video::Gamma";
}

function GraphicsMenu::loadShaderOptions(%this)
{
   GraphicsShaderOptionsArray.clear();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Material Quality");
   %option.qualitySettingGroup = ShaderQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Parallax");
   %option.qualitySettingGroup = ParallaxQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Water Reflections");
   %option.qualitySettingGroup = ReflectionsQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("High Dynamic Range");
   %option.qualitySettingGroup = HDRQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Sceen Space Ambient Occlusion");
   %option.qualitySettingGroup = SSAOQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Light Rays");
   %option.qualitySettingGroup = LightRaysQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Depth of Field");
   %option.qualitySettingGroup = DOFQualityGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(GraphicsShaderOptionsArray);
   %option-->nameText.setText("Vignetting");
   %option.qualitySettingGroup = VignetteQualityGroup;
   %option.init();
   
   GraphicsShaderOptionsArray.refresh();
}

function GraphicsMenu::loadLightingOptions(%this)
{
   GraphicsLightingOptionsArray.clear();
   
    %option = OptionsMenu.addSettingOption(GraphicsLightingOptionsArray);
    %option-->nameText.setText("Shadow Quality");
    %option.qualitySettingGroup = ShadowQualityList;
    %option.init();

    %option = OptionsMenu.addSettingOption(GraphicsLightingOptionsArray);
    %option-->nameText.setText("Shadow Caching");
    %option.qualitySettingGroup = ShadowCacheList;
    %option.init();

    %option = OptionsMenu.addSettingOption(GraphicsLightingOptionsArray);
    %option-->nameText.setText("Soft Shadows");
    %option.qualitySettingGroup = SoftShadowList;
    %option.init();
    
    GraphicsLightingOptionsArray.refresh();
}

function GraphicsMenu::loadMeshesAndTexturesOptions(%this)
{
   GraphicsMeshTexOptionsArray.clear();
   
    %option = OptionsMenu.addSettingOption(GraphicsMeshTexOptionsArray);
    %option-->nameText.setText("Mesh Detail");
    %option.qualitySettingGroup = MeshQualityGroup;
    %option.init();

    %option = OptionsMenu.addSettingOption(GraphicsMeshTexOptionsArray);
    %option-->nameText.setText("Texture Detail");
    %option.qualitySettingGroup = TextureQualityGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(GraphicsMeshTexOptionsArray);
    %option-->nameText.setText("Ground Clutter Density");
    %option.qualitySettingGroup = GroundCoverDensityGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(GraphicsMeshTexOptionsArray);
    %option-->nameText.setText("Terrain Detail");
    %option.qualitySettingGroup = TerrainQualityGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(GraphicsMeshTexOptionsArray);
    %option-->nameText.setText("Decal Lifetime");
    %option.qualitySettingGroup = DecalLifetimeGroup;
    %option.init();
    
    //%option = OptionsMenu.addSliderOption();
    //%option-->nameText.setText("View Distance");
    //%option-->SettingText.setText("On");
    //%option.qualitySettingGroup = ViewDistanceQualityGroup;
    
    GraphicsMeshTexOptionsArray.refresh();
}

function GraphicsMenu::loadDisplayOptions(%this)
{
   ScreenOptionsArray.clear();
   
   %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
   %option-->nameText.setText("Graphics Driver");
   %option.qualitySettingGroup = DisplayAPISettingGroup;
   %option.init();

   //Resolution
   %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
   %option-->nameText.setText("Screen Resolution");
    
   // Loop through all and add all valid resolutions
   if(!isObject(ScreenResolutionSettingGroup))
   {
      new SimGroup( ScreenResolutionSettingGroup );
   }
   else
   {
      ScreenResolutionSettingGroup.clear();
   }
   
   %count = 0;
   %resCount = Canvas.getModeCount();
   for (%i = 0; %i < %resCount; %i++)
   {
      %testResString = Canvas.getMode( %i );
      %testRes = _makePrettyResString( %testResString );
                     
      // Only add to list if it isn't there already.
      //if (%resMenu.findText(%testRes) == -1)
      //{
         //%resMenu.add(%testRes, %i);
         
         %setting = new ArrayObject()
         {
            class = "OptionsMenuSettingLevel";
            caseSensitive = true;
            
            displayName = %testRes;
            
            key["$pref::Video::Resolution"] = getWords(%testResString, 0, 1);
         };
         
         ScreenResolutionSettingGroup.add(%setting);
   
         %count++;
      //}
   }
   %option.qualitySettingGroup = ScreenResolutionSettingGroup;
   %option.init();
   //

    %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
    %option-->nameText.setText("Full Screen");
    %option.qualitySettingGroup = FullScreenSettingGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
    %option-->nameText.setText("Refresh Rate");
    %option.qualitySettingGroup = RefreshRateQualityGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
    %option-->nameText.setText("Anisotropic Filtering");
    %option.qualitySettingGroup = AnisoQualityGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
    %option-->nameText.setText("AntiAliasing");
    %option.qualitySettingGroup = AAQualityGroup;
    %option.init();
    
    %option = OptionsMenu.addSettingOption(ScreenOptionsArray);
    %option-->nameText.setText("VSync");
    %option.qualitySettingGroup = VSyncQualityGroup;
    %option.init();
    
    ScreenOptionsArray.refresh();
}

function _makePrettyResString( %resString )
{
   %width = getWord( %resString, $WORD::RES_X );
   %height = getWord( %resString, $WORD::RES_Y );
   
   %aspect = %width / %height;
   %aspect = mRound( %aspect * 100 ) * 0.01;            
   
   switch$( %aspect )
   {
      case "1.33":
         %aspect = "4:3";
      case "1.78":
         %aspect = "16:9";
      default:
         %aspect = "";
   }
   
   %outRes = %width @ " x " @ %height;
   if ( %aspect !$= "" )
      %outRes = %outRes @ "  (" @ %aspect @ ")";
      
   return %outRes;   
}

//
function GraphicsMenuSetting::init( %this )
{
   assert( isObject( %this ) );
   assert( isObject( %this.qualitySettingGroup ) );
    
   // Fill it.
   %select = -1;
   %selectedName = "";
   for ( %i=0; %i < %this.qualitySettingGroup.getCount(); %i++ )
   {
      %level = %this.qualitySettingGroup.getObject( %i );
      
      %levelName = %level.displayName;
      if ( %level.isCurrent() )
      {
         %select = %i;
         %selectedName = %level.displayName;
      }
   }
   
   // Setup a default selection.
   if ( %select == -1 )
   {
      %this-->SettingText.setText( "Custom" );
      %this.selectedLevel = %this.qualitySettingGroup.getCount();
   }
   else
   {
      %this-->SettingText.setText(%selectedName);
      %this.selectedLevel = %select;
   }
}

function GraphicsOptionsMenuGroup::applySetting(%this, %settingName)
{
   for(%i=0; %i < %this.getCount(); %i++)
   {
      %setting = %this.getObject(%i);
      if(%setting.displayName $= %settingName)
      {
         for ( %s=0; %s < %setting.count(); %s++ )
         {
            %pref = %setting.getKey( %s );
            %value = %setting.getValue( %s );
            setVariable( %pref, %value );
         }
      }
   }
}

function GraphicsMenu::apply(%this)
{
   //go through our settings and apply the changes!  
   for(%i=0; %i < GraphicsLightingOptionsArray.getCount(); %i++)
   {
      %setting = GraphicsLightingOptionsArray.getObject(%i);
      
      if(%setting.qualitySettingGroup.isMethod("onApply"))
         %setting.qualitySettingGroup.onApply();
   }
   
   for(%i=0; %i < GraphicsShaderOptionsArray.getCount(); %i++)
   {
      %setting = GraphicsShaderOptionsArray.getObject(%i);
      
      if(%setting.qualitySettingGroup.isMethod("onApply"))
         %setting.qualitySettingGroup.onApply();
   }
   
   for(%i=0; %i < GraphicsMeshTexOptionsArray.getCount(); %i++)
   {
      %setting = GraphicsMeshTexOptionsArray.getObject(%i);
      
      if(%setting.qualitySettingGroup.isMethod("onApply"))
         %setting.qualitySettingGroup.onApply();
   }
   
   //Update the display settings now
   %newBpp        = 32; // ... its not 1997 anymore.

   if ( $pref::Video::FullScreen == false )
	{
      // If we're in windowed mode switch the fullscreen check
      // if the resolution is bigger than the desktop.
      %deskRes    = getDesktopResolution();      
      %deskResX   = getWord(%deskRes, $WORD::RES_X);
      %deskResY   = getWord(%deskRes, $WORD::RES_Y);
	   if (  getWord( %newRes, $WORD::RES_X ) > %deskResX || 
	         getWord( %newRes, $WORD::RES_Y ) > %deskResY )
      {
         $pref::Video::FullScreen = true;
      }
	}

   // Build the final mode string.
	%newMode = $pref::Video::Resolution SPC $pref::Video::FullScreen SPC %newBpp SPC $pref::Video::RefreshRate SPC $pref::Video::AA;
	
   // Change the video mode.   
   if (  %newMode !$= $pref::Video::mode )
   {
      //if ( %testNeedApply )
      //   return true;

      $pref::Video::mode = %newMode;
      configureCanvas();
   }
   
   echo("Exporting client prefs");
   %prefPath = getPrefpath();
   export("$pref::*", %prefPath @ "/clientPrefs.cs", false);

 
}
function TextureQualityGroup::onApply( %this )
{
   // Note that this can be a slow operation.  
   reloadTextures();
}
function LightingQualityGroup::onApply( %this, %level )
{
   // Set the light manager.  This should do nothing 
   // if its already set or if its not compatible.   
   setLightManager( $pref::lightManager );
}
function DOFQualityGroup::onApply(%this)
{
   PostFXManager.settingsEffectSetEnabled("DOF", $pref::PostFX::EnableDOF);
}
function SSAOQualityGroup::onApply(%this)
{
   PostFXManager.settingsEffectSetEnabled("SSAO", $pref::PostFX::EnableSSAO);
}
function HDRQualityGroup::onApply(%this)
{
   PostFXManager.settingsEffectSetEnabled("HDR", $pref::PostFX::EnableHDR);
}
function LightRaysQualityGroup::onApply(%this)
{
   PostFXManager.settingsEffectSetEnabled("LightRays", $pref::PostFX::EnableLightRays);
}
function VignetteQualityGroup::onApply(%this)
{
   PostFXManager.settingsEffectSetEnabled("Vignette", $pref::PostFX::EnableVignette);
}
