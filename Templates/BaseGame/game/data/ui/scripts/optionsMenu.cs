//options settings

//Screen and Display menu
//Renderer Mode
//Screen resolution
//Windowed/fullscreen(borderless?)
//VSync

//Screen brightness
//screen brightness
//screen gamma

//Lighting Menu
//Shadow Distance(Distance shadows are drawn to. Also affects shadowmap slices)
//Shadow Quality(Resolution of shadows rendered, setting to none disables dynamic shadows)
//Soft Shadows(Whether shadow softening is used)
//Shadow caching(If the lights enable it, shadow caching is activated)
//Light Draw Distance(How far away lights are still drawn. Doesn't impact vector lights like the sun)

//Mesh and Textures Menu
//Draw distance(Overall draw distance) -slider
//Object draw distance(Draw distance from small/unimportant objects) -slider
//Mesh quality
//Texture quality
//Foliage draw distance
//Terrain Quality
//Decal Quality

//Effects Menu
//Parallax
//HDR
//Light shafts
//Motion Blur
//Depth of Field
//SSAO
//AA(ModelXAmount)[defualt is FXAA]
//Anisotropic filtering

//Keybinds

//Camera
//horizontal mouse sensitivity
//vert mouse sensitivity
//invert vertical
//zoom mouse sensitivities(both horz/vert)
//headbob
//FOV

function OptionsMenu::onWake(%this)
{
    OptionsMain.hidden = false;
    ControlsMenu.hidden = true;
    GraphicsMenu.hidden = true;
    CameraMenu.hidden = true;
    ScreenBrightnessMenu.hidden = true;
    
    OptionsOKButton.hidden = false;
    OptionsCancelButton.hidden = false;
    OptionsDefaultsButton.hidden = false;

    if(!isObject(%this.tamlReader))
        %this.tamlReader = new Taml();
}

function OptionsMenuOKButton::onClick(%this)
{
    //save the settings and then back out
    
    OptionsMenu.backOut();
}

function OptionsMenuCancelButton::onClick(%this)
{
    //we don't save, so go straight to backing out of the menu    
    OptionsMenu.backOut();
}

function OptionsMenuDefaultsButton::onClick(%this)
{
    //we don't save, so go straight to backing out of the menu    
    OptionsMenu.backOut();
}

function ControlsSettingsMenuButton::onClick(%this)
{
    OptionsMain.hidden = true;
    ControlsMenu.hidden = false;
    
    ControlsMenu.reload();
}

function GraphicsSettingsMenuButton::onClick(%this)
{
    OptionsMain.hidden = true;
    GraphicsMenu.hidden = false;

    GraphicsMenu.loadSettings();
}

function CameraSettingsMenuButton::onClick(%this)
{
    OptionsMain.hidden = true;
    CameraMenu.hidden = false;
    
    CameraMenu.loadSettings();
}

function AudioSettingsMenuButton::onClick(%this)
{
    OptionsMain.hidden = true;
    AudioMenu.hidden = false;
    AudioMenu.loadSettings();
}

function ScreenBrSettingsMenuButton::onClick(%this)
{
    OptionsMain.hidden = true;
    ScreenBrightnessMenu.hidden = false;
}

function OptionsMenu::backOut(%this)
{
   //save the settings and then back out
   if(OptionsMain.hidden == false)
   {
      //we're not in a specific menu, so we're actually exiting
      Canvas.popDialog(OptionsMenu);

      if(isObject(OptionsMenu.returnGui) && OptionsMenu.returnGui.isMethod("onReturnTo"))
         OptionsMenu.returnGui.onReturnTo();
   }
   else
   {
      OptionsMain.hidden = false;
      ControlsMenu.hidden = true;
      GraphicsMenu.hidden = true;
      CameraMenu.hidden = true;
      AudioMenu.hidden = true;
      ScreenBrightnessMenu.hidden = true;
   }
}

function OptionsMenu::addSettingOption(%this, %arrayTarget)
{
    %graphicsOption = OptionsMenu.tamlReader.read("data/ui/scripts/guis/graphicsMenuSettingsCtrl.taml");

    %arrayTarget.add(%graphicsOption);

    return %graphicsOption;
}

function OptionsMenu::addSliderOption(%this, %arrayTarget, %range, %ticks, %variable, %value, %class)
{
    %graphicsOption = OptionsMenu.tamlReader.read("data/ui/scripts/guis/graphicsMenuSettingsSlider.taml");

    %arrayTarget.add(%graphicsOption);
    
    if(%range !$= "")
    {
       %graphicsOption-->slider.range = %range;
    }
    
    if(%ticks !$= "")
    {
       %graphicsOption-->slider.ticks = %ticks;
    }
    
    if(%variable !$= "")
    {
       %graphicsOption-->slider.variable = %variable;
    }
    
    if(%value !$= "")
    {
       %graphicsOption-->slider.setValue(%value);
    }
    
    if(%class !$= "")
    {
       %graphicsOption-->slider.className = %class;
    }
    else
        %graphicsOption-->slider.className = OptionsMenuSlider;
        
    %graphicsOption-->slider.snap = true;
    
    %graphicsOption-->slider.onValueSet();

    return %graphicsOption;
}

function OptionsMenuSlider::onMouseDragged(%this)
{
   %this.onValueSet();
}

function OptionsMenuSlider::onValueSet(%this)
{
   %this.getParent().getParent()-->valueText.setText(mRound(%this.value * 10));  
}

function FOVOptionSlider::onMouseDragged(%this)
{
   %this.onValueSet();
}

function FOVOptionSlider::onValueSet(%this)
{
   %this.getParent().getParent()-->valueText.setText(mRound(%this.value));
}

/// Returns true if the current quality settings equal
/// this graphics quality level.
function OptionsMenuSettingLevel::isCurrent( %this )
{
   // Test each pref to see if the current value
   // equals our stored value.
   
   for ( %i=0; %i < %this.count(); %i++ )
   {
      %pref = %this.getKey( %i );
      %value = %this.getValue( %i );
      
      %prefVarValue = getVariable( %pref );
      if ( getVariable( %pref ) !$= %value )
         return false;
   }
   
   return true;
}
// =============================================================================
// CAMERA MENU
// =============================================================================
function CameraMenu::onWake(%this)
{
    
}

function CameraMenu::apply(%this)
{
   setFOV($pref::Player::defaultFov);  
}

function CameraMenu::loadSettings(%this)
{
   CameraMenuOptionsArray.clear();
   
   %option = OptionsMenu.addSettingOption(CameraMenuOptionsArray);
   %option-->nameText.setText("Invert Vertical");
   %option.qualitySettingGroup = InvertVerticalMouse;
   %option.init();
   
   %option = OptionsMenu.addSliderOption(CameraMenuOptionsArray, "0.1 1", 8, "$pref::Input::VertMouseSensitivity", $pref::Input::VertMouseSensitivity);
   %option-->nameText.setText("Vertical Sensitivity");
   
   %option = OptionsMenu.addSliderOption(CameraMenuOptionsArray, "0.1 1", 8, "$pref::Input::HorzMouseSensitivity", $pref::Input::HorzMouseSensitivity);
   %option-->nameText.setText("Horizontal Sensitivity");
   
   %option = OptionsMenu.addSliderOption(CameraMenuOptionsArray, "0.1 1", 8, "$pref::Input::ZoomVertMouseSensitivity", $pref::Input::ZoomVertMouseSensitivity);
   %option-->nameText.setText("Zoom Vertical Sensitivity");

   %option = OptionsMenu.addSliderOption(CameraMenuOptionsArray, "0.1 1", 8, "$pref::Input::ZoomHorzMouseSensitivity", $pref::Input::ZoomHorzMouseSensitivity);
   %option-->nameText.setText("Zoom Horizontal Sensitivity");
   
   %option = OptionsMenu.addSliderOption(CameraMenuOptionsArray, "65 90", 25, "$pref::Player::defaultFov", $pref::Player::defaultFov, FOVOptionSlider);
   %option-->nameText.setText("Field of View");
   
   CameraMenuOptionsArray.refresh();
}

function CameraMenuOKButton::onClick(%this)
{
   //save the settings and then back out
    CameraMenu.apply();
    OptionsMenu.backOut();
}

function CameraMenuDefaultsButton::onClick(%this)
{
   
}
// =============================================================================
// AUDIO MENU
// =============================================================================
$AudioTestHandle = 0;
// Description to use for playing the volume test sound.  This isn't
// played with the description of the channel that has its volume changed
// because we know nothing about the playback state of the channel.  If it
// is paused or stopped, the test sound would not play then.
$AudioTestDescription = new SFXDescription()
{
   sourceGroup = AudioChannelMaster;
};

function AudioMenu::loadSettings(%this)
{
   // Loop through all and add all valid resolutions
   if(!isObject(SoundAPIGroup))
   {
      new SimGroup( SoundAPIGroup );
   }
   else
   {
      SoundAPIGroup.clear();
   }
   
   %buffer = sfxGetAvailableDevices();
   %count = getRecordCount( %buffer );
   for (%i = 0; %i < %count; %i++)
   {
      %record = getRecord(%buffer, %i);
      %provider = getField(%record, 0);
      %device = getField(%record, 1);
         
      %setting = new ArrayObject()
      {
         class = "OptionsMenuSettingLevel";
         caseSensitive = true;
         
         displayName = %provider;
         
         key["$pref::SFX::provider"] = %provider;
      };
      
      SoundAPIGroup.add(%setting);
   }
   
   %this.loadDevices();
   
   AudioMenuOptionsArray.clear();
   
   %option = OptionsMenu.addSettingOption(AudioMenuOptionsArray);
   %option-->nameText.setText("Sound Provider");
   %option.qualitySettingGroup = SoundAPIGroup;
   %option.init();
   
   %option = OptionsMenu.addSettingOption(AudioMenuOptionsArray);
   %option-->nameText.setText("Sound Device");
   %option.qualitySettingGroup = SoundDeviceGroup;
   %option.init();
   
   %option = OptionsMenu.addSliderOption(AudioMenuOptionsArray, "0.1 1", 8,
      "$pref::SFX::masterVolume", $pref::SFX::masterVolume);
   %option-->nameText.setText("Master Audio Volume");
   
   %option = OptionsMenu.addSliderOption(AudioMenuOptionsArray, "0.1 1", 8,
      "$pref::SFX::channelVolume[" @ $GuiAudioType @ "]", $pref::SFX::channelVolume[$GuiAudioType]);
   %option-->nameText.setText("Gui Volume");
   
   %option = OptionsMenu.addSliderOption(AudioMenuOptionsArray, "0.1 1", 8, 
      "$pref::SFX::channelVolume[" @ $SimAudioType @ "]", $pref::SFX::channelVolume[$SimAudioType]);
   %option-->nameText.setText("Effect Volume");
   
   %option = OptionsMenu.addSliderOption(AudioMenuOptionsArray, "0.1 1", 8, 
      "$pref::SFX::channelVolume[" @ $MusicAudioType @ "]", $pref::SFX::channelVolume[$MusicAudioType]);
   %option-->nameText.setText("Music Volume");
   
   AudioMenuOptionsArray.refresh();
}

function AudioMenu::loadDevices(%this)
{
   if(!isObject(SoundDeviceGroup))
   {
      new SimGroup( SoundDeviceGroup );
   }
   else
   {
      SoundDeviceGroup.clear();
   }
   
   %buffer = sfxGetAvailableDevices();
   %count = getRecordCount( %buffer );
   for (%i = 0; %i < %count; %i++)
   {
      %record = getRecord(%buffer, %i);
      %provider = getField(%record, 0);
      %device = getField(%record, 1);
         
      if($pref::SFX::provider !$= %provider)
         continue;
      
      %setting = new ArrayObject()
      {
         class = "OptionsMenuSettingLevel";
         caseSensitive = true;
         
         displayName = %device;
         
         key["$pref::SFX::Device"] = %device;
      };
      
      SoundDeviceGroup.add(%setting);
   }
}

function AudioMenu::apply(%this)
{
   sfxSetMasterVolume( $pref::SFX::masterVolume );
   
   sfxSetChannelVolume( $GuiAudioType, $pref::SFX::channelVolume[ $GuiAudioType ] );
   sfxSetChannelVolume( $SimAudioType, $pref::SFX::channelVolume[ $SimAudioType ] );
   sfxSetChannelVolume( $MusicAudioType, $pref::SFX::channelVolume[ $MusicAudioType ] );
   
   if ( !sfxCreateDevice(  $pref::SFX::provider, 
                           $pref::SFX::device, 
                           $pref::SFX::useHardware,
                           -1 ) )                              
      error( "Unable to create SFX device: " @ $pref::SFX::provider 
                                             SPC $pref::SFX::device 
                                             SPC $pref::SFX::useHardware );        

   if( !isObject( $AudioTestHandle ) )
   {
      sfxPlay(menuButtonPressed);
   }
}

function AudioMenuOKButton::onClick(%this)
{
   //save the settings and then back out
    AudioMenu.apply();
    OptionsMenu.backOut();
}

function AudioMenuDefaultsButton::onClick(%this)
{
   sfxInit();
   AudioMenu.loadSettings();
}

//===
// BRIGHTNESS MENU
//===
function BrightnessMenuDefaultsButton::onClick(%this)
{
    BrightnessMenuSlider.setValue(1.1005);
}
//
function OptionsMenuBackSetting::onClick(%this)
{
   %optionSetting = %this.getParent().getParent();
   %qualityGroup = %optionSetting.qualitySettingGroup;
   
   //now, lower our setting
   if(%optionSetting.selectedLevel > 0)
   {
      %newLevel = %optionSetting.selectedLevel-1;
      %newQualityGroup = %optionSetting.qualitySettingGroup.getObject( %newLevel );
      
      for ( %i=0; %i < %newQualityGroup.count(); %i++ )
      {
         %pref = %newQualityGroup.getKey( %i );
         %value = %newQualityGroup.getValue( %i );
         setVariable( %pref, %value );
      }
      
      %optionSetting-->SettingText.setText(%newQualityGroup.displayName);
      %optionSetting.selectedLevel = %newLevel;
   }
}

function OptionsMenuForwardSetting::onClick(%this)
{
   %optionSetting = %this.getParent().getParent();
   %qualityGroup = %optionSetting.qualitySettingGroup;
   
   //now, lower our setting
   %maxLevel = %qualityGroup.getCount()-1;
   if(%optionSetting.selectedLevel < %qualityGroup.getCount()-1)
   {
      %newLevel = %optionSetting.selectedLevel+1;
      %newQualityGroup = %optionSetting.qualitySettingGroup.getObject( %newLevel );
      
      for ( %i=0; %i < %newQualityGroup.count(); %i++ )
      {
         %pref = %newQualityGroup.getKey( %i );
         %value = %newQualityGroup.getValue( %i );
         setVariable( %pref, %value );
      }
      
      %optionSetting-->SettingText.setText(%newQualityGroup.displayName);
      %optionSetting.selectedLevel = %newLevel;
   }
}