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

if ( isObject( FPSMoveMap ) )
   FPSMoveMap.delete();
   
new ActionMap(FPSMoveMap);

//------------------------------------------------------------------------------
// Non-remapable binds
//------------------------------------------------------------------------------
FPSMoveMap.bindCmd(keyboard, "escape", "", "Canvas.pushDialog(PauseMenu);");

//------------------------------------------------------------------------------
// Movement Keys
//------------------------------------------------------------------------------
FPSMoveMap.bind( keyboard, a, moveleft );
FPSMoveMap.bind( keyboard, d, moveright );
FPSMoveMap.bind( keyboard, left, moveleft );
FPSMoveMap.bind( keyboard, right, moveright );

FPSMoveMap.bind( keyboard, w, moveforward );
FPSMoveMap.bind( keyboard, s, movebackward );
FPSMoveMap.bind( keyboard, up, moveforward );
FPSMoveMap.bind( keyboard, down, movebackward );

FPSMoveMap.bind( keyboard, e, moveup );
FPSMoveMap.bind( keyboard, c, movedown );

FPSMoveMap.bind( keyboard, space, jump );
FPSMoveMap.bind( mouse, xaxis, yaw );
FPSMoveMap.bind( mouse, yaxis, pitch );

FPSMoveMap.bind( gamepad, thumbrx, "D", "-0.23 0.23", gamepadYaw );
FPSMoveMap.bind( gamepad, thumbry, "D", "-0.23 0.23", gamepadPitch );
FPSMoveMap.bind( gamepad, thumblx, "D", "-0.23 0.23", gamePadMoveX );
FPSMoveMap.bind( gamepad, thumbly, "D", "-0.23 0.23", gamePadMoveY );

FPSMoveMap.bind( gamepad, btn_a, jump );
FPSMoveMap.bindCmd( gamepad, btn_back, "disconnect();", "" );

FPSMoveMap.bindCmd(gamepad, dpadl, "toggleLightColorViz();", "");
FPSMoveMap.bindCmd(gamepad, dpadu, "toggleDepthViz();", "");
FPSMoveMap.bindCmd(gamepad, dpadd, "toggleNormalsViz();", "");
FPSMoveMap.bindCmd(gamepad, dpadr, "toggleLightSpecularViz();", "");


//------------------------------------------------------------------------------
// Mouse Trigger
//------------------------------------------------------------------------------
FPSMoveMap.bind( mouse, button0, mouseFire );
FPSMoveMap.bind( mouse, button1, altTrigger );

//------------------------------------------------------------------------------
// Gamepad Trigger
//------------------------------------------------------------------------------
FPSMoveMap.bind(gamepad, triggerr, gamepadFire);
FPSMoveMap.bind(gamepad, triggerl, gamepadAltTrigger);

FPSMoveMap.bind(keyboard, f, setZoomFOV);
FPSMoveMap.bind(keyboard, r, toggleZoom);
FPSMoveMap.bind( gamepad, btn_b, toggleZoom );

//------------------------------------------------------------------------------
// Camera & View functions
//------------------------------------------------------------------------------
FPSMoveMap.bind( keyboard, z, toggleFreeLook );
FPSMoveMap.bind(keyboard, tab, toggleFirstPerson );
FPSMoveMap.bind(keyboard, "alt c", toggleCamera);

FPSMoveMap.bind( gamepad, btn_back, toggleCamera );

//------------------------------------------------------------------------------
// Demo recording functions
//------------------------------------------------------------------------------
FPSMoveMap.bind( keyboard, F3, startRecordingDemo );
FPSMoveMap.bind( keyboard, F4, stopRecordingDemo );


//------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------
FPSMoveMap.bind(keyboard, "F8", dropCameraAtPlayer);
FPSMoveMap.bind(keyboard, "F7", dropPlayerAtCamera);

GlobalActionMap.bind(keyboard, "ctrl o", bringUpOptions);

//------------------------------------------------------------------------------
// Debugging Functions
//------------------------------------------------------------------------------
GlobalActionMap.bind(keyboard, "ctrl F2", showMetrics);
GlobalActionMap.bind(keyboard, "ctrl F3", doProfile);

//------------------------------------------------------------------------------
// Misc.
//------------------------------------------------------------------------------
GlobalActionMap.bind(keyboard, "tilde", toggleConsole);
GlobalActionMap.bindCmd(keyboard, "alt k", "cls();","");
GlobalActionMap.bindCmd(keyboard, "alt enter", "", "Canvas.attemptFullscreenToggle();");
