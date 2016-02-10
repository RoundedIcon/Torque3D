new SimGroup( MeshQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::TS::detailAdjust"] = 0.5;
      key["$pref::TS::skipRenderDLs"] = 1;      
  };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::TS::detailAdjust"] = 0.75;
      key["$pref::TS::skipRenderDLs"] = 0;      
   };
   
   new ArrayObject( )
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::TS::detailAdjust"] = 1.0;
      key["$pref::TS::skipRenderDLs"] = 0;      
   };

   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::TS::detailAdjust"] = 1.5;
      key["$pref::TS::skipRenderDLs"] = 0;      
   };   
};

new SimGroup( TextureQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Video::textureReductionLevel"] = 2;
      key["$pref::Reflect::refractTexScale"] = 0.5;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::Video::textureReductionLevel"] = 1;
      key["$pref::Reflect::refractTexScale"] = 0.75;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Video::textureReductionLevel"] = 0;
      key["$pref::Reflect::refractTexScale"] = 1;
   };

   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::Video::textureReductionLevel"] = 0;
      key["$pref::Reflect::refractTexScale"] = 1.25;
   };   
};

new SimGroup( GroundCoverDensityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Lowest";

      key["$pref::GroundCover::densityScale"] = 0.25;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::GroundCover::densityScale"] = 0.5;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::GroundCover::densityScale"] = 0.75;
   };

   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::GroundCover::densityScale"] = 1.0;
   };   
};

new SimGroup( DecalLifetimeGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "None";
      
      key["$pref::decalMgr::enabled"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
      
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::Decals::lifeTimeScale"] = 0.25;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";
      
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::Decals::lifeTimeScale"] = 0.5;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::decalMgr::enabled"] = true;
      key["$pref::Decals::lifeTimeScale"] = 1;
   };
};

new SimGroup( TerrainQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Terrain::lodScale"] = 2.0;
      key["$pref::Terrain::detailScale"] = 0.5; 
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
            
      key["$pref::Terrain::lodScale"] = 1.5;
      key["$pref::Terrain::detailScale"] = 0.75;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Terrain::lodScale"] = 1.0;
      key["$pref::Terrain::detailScale"] = 1.0;
   };

   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";

      key["$pref::Terrain::lodScale"] = 0.75;
      key["$pref::Terrain::detailScale"] = 1.5;
   };   
};

//Shadows and Lighting
new SimGroup( ShadowQualityList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "None";
      
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = true;
      key["$pref::Shadows::textureScalar"] = 0.5;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 0.25;

   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 0.5;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::lightManager"] = "Advanced Lighting";
      key["$pref::Shadows::disable"] = false;
      key["$pref::Shadows::textureScalar"] = 1.0;
   };
};

new SimGroup( ShadowDistanceList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Shadows::drawDistance"] = 0.25;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::Shadows::drawDistance"] = 0.5;  
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Shadows::drawDistance"] = 1; 
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::Shadows::drawDistance"] = 1.5;
   };

   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Highest";
      
      key["$pref::Shadows::drawDistance"] = 2; 
   };    
};

new SimGroup( SoftShadowList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Shadows::filterMode"] = "None"; 
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::Shadows::filterMode"] = "SoftShadow"; 
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";
                  
      key["$pref::Shadows::filterMode"] = "SoftShadowHighQuality"; 
   };
};

new SimGroup( ShadowCacheList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Shadows::useShadowCaching"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::Shadows::useShadowCaching"] = true;  
   };
};

new SimGroup( LightDistanceList )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Lowest";
      
      key["$pref::Lights::drawDistance"] = 0.25;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
                  
      key["$pref::Lights::drawDistance"] = 0.5;  
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Medium";

      key["$pref::Lights::drawDistance"] = 1; 
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::Lights::drawDistance"] = 1.5;
   };

   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Highest";
      
      key["$pref::Lights::drawDistance"] = 2; 
   };    
};

//
//Shader settings
new SimGroup( ReflectionsQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Water::disableTrueReflections"] = true;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::Water::disableTrueReflections"] = false;     
   };
};

new SimGroup( ParallaxQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Video::disableParallaxMapping"] = true;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::Video::disableParallaxMapping"] = false;     
   };
};

new SimGroup( HDRQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::PostFX::EnableHDR"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::PostFX::EnableHDR"] = true;     
   };
};

new SimGroup( SSAOQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::PostFX::EnableSSAO"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::PostFX::EnableSSAO"] = true;     
   };
};

new SimGroup( DOFQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::PostFX::EnableDOF"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::PostFX::EnableDOF"] = true;     
   };
};

new SimGroup( LightRaysQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::PostFX::EnableLightRays"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::PostFX::EnableLightRays"] = true;     
   };
};

new SimGroup( VignetteQualityGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::PostFX::EnableVignette"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
                  
      key["$pref::PostFX::EnableVignette"] = true;     
   };
};

new SimGroup( ShaderQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Low";
      
      key["$pref::Video::disablePixSpecular"] = true;
      key["$pref::Video::disableNormalmapping"] = true;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "High";
      
      key["$pref::Video::disablePixSpecular"] = false;
      key["$pref::Video::disableNormalmapping"] = false;
   };
};

//
new SimGroup( VSyncQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Video::disableVerticalSync"] = true;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
      
      key["$pref::Video::disableVerticalSync"] = false;
   };
};

new SimGroup( AnisoQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Video::defaultAnisotropy"] = 0;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "x4";
      
      key["$pref::Video::defaultAnisotropy"] = 4;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "x8";
      
      key["$pref::Video::defaultAnisotropy"] = 8;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "x16";
      
      key["$pref::Video::defaultAnisotropy"] = 16;
   };
};

new SimGroup( AAQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Video::AA"] = 0;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "FXAAx1";
      
      key["$pref::Video::AA"] = 1;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "FXAAx2";
      
      key["$pref::Video::AA"] = 2;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "FXAAx4";
      
      key["$pref::Video::AA"] = 4;
   };
};

new SimGroup( RefreshRateQualityGroup )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "60";
      
      key["$pref::Video::RefreshRate"] = 60;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "75";
      
      key["$pref::Video::RefreshRate"] = 75;
   };
};
//
new SimGroup( DisplayAPISettingGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "D3D9";
      
      key["$pref::Video::displayDevice"] = "D3D9";
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "OpenGL";
                  
      key["$pref::Video::displayDevice"] = "OpenGL";     
   };
};

new SimGroup( FullScreenSettingGroup )
{ 
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Windowed";
      
      key["$pref::Video::FullScreen"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "FullScreen";
                  
      key["$pref::Video::FullScreen"] = true;     
   };
};

//
new SimGroup( InvertVerticalMouse )
{
   class = "GraphicsOptionsMenuGroup";
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "Off";
      
      key["$pref::Input::invertVerticalMouse"] = false;
   };
   
   new ArrayObject()
   {
      class = "OptionsMenuSettingLevel";
      caseSensitive = true;
      
      displayName = "On";
      
      key["$pref::Input::invertVerticalMouse"] = true;
   };
};