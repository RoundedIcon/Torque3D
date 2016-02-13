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

#include "platform/platform.h"

#include "gui/core/guiControl.h"
#include "gui/3d/guiTSControl.h"
#include "console/consoleTypes.h"
#include "scene/sceneManager.h"
#include "T3D/gameBase/gameConnection.h"
#include "T3D/shapeBase.h"
#include "gfx/gfxDrawUtil.h"
#include "console/engineAPI.h"
#include "gui/shiny/guiTickCtrl.h"


//----------------------------------------------------------------------------
/// Displays name above shape objects and strings at specified points.
///
/// This control displays the name of all named ShapeBase objects on the
/// client. The name of objects within the control's display area are
/// overlayed above the object. Support has been added for free floating
/// elements as well. A string can be rendered with a specified location,
/// color, and lifetime by using the addElement() method of this gui.
///
/// This GUI control must be a child of a TSControl, and a server connection
/// and control object must be present.
///
/// This is a stand-alone control and relies on GuiTickCtrl.
class GuiShapeNameHud : public GuiTickCtrl{
   typedef GuiControl Parent;

   // field data
   ColorF   mFillColor;
   ColorF   mFrameColor;
   ColorF   mTextColor;
   ColorF   mLabelFillColor;
   ColorF   mLabelFrameColor;

   F32      mVerticalOffset;
   F32      mDistanceFade;
   bool     mShowFrame;
   bool     mShowFill;
   bool     mShowLabelFrame;
   bool     mShowLabelFill;

   Point2I  mLabelPadding;

	// element data
	F32 mDefaultLifetime;
	F32 mTime;

	struct mElementInfo
	{
		Point3F position;
		String damage;
		F32 lifeTime;
		ColorF color;
		F32 time;
	};

	std::list<mElementInfo> mElementList;

protected:
   void drawName( Point2I offset, const char *buf, F32 opacity, ColorF color);

	// GuiTickCtrl
	void advanceTime(F32 timeDelta);

public:
   GuiShapeNameHud();

	void addElement(Point3F pos, String damage, F32 lifeTime, ColorF textColor);

	// returns the number of active elements. This does not include Shapebase names
	S32 getElementCount(){ return mElementList.size(); };

	// clears all elements from the internal list. Shapebase names are not affected
	void clearElements(){ mElementList.clear(); };

   // GuiControl
   virtual void onRender(Point2I offset, const RectI &updateRect);

   static void initPersistFields();
   DECLARE_CONOBJECT( GuiShapeNameHud );
   DECLARE_CATEGORY( "Gui Game" );
   DECLARE_DESCRIPTION( "Displays name and damage of ShapeBase objects in its bounds.\n"
      "Must be a child of a GuiTSCtrl and a server connection must be present." );
};


//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiShapeNameHud);

ConsoleDocClass( GuiShapeNameHud,
   "@brief Displays name of ShapeBase objects and strings at specified points in its bounds."
	"Must be a child of a GuiTSCtrl and a server connection must be present.\n\n"

   "This control displays the of all named ShapeBase objects on the client. "
   "The name of objects within the control's display area are overlayed above the object.\n\n"
	"This control displays \"Elements\", user defined strings with certain colors and lifetimes. "
	"The contents of the strings and their position/attributes are stored in an internal list.\n\n"
   "This GUI control must be a child of a TSControl, and a server connection and control object must be present. "
   "This is a stand-alone control and relies on GuiTickCtrl.\n\n"
   
   "@tsexample\n"
		"\n new GuiShapeNameHud()"
		"{\n"
		"	fillColor = \"0.0 1.0 0.0 1.0\"; // Fills with a solid green color\n"
		"	frameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	textColor = \"1.0 1.0 1.0 1.0\"; // Solid white text Color\n"
		"	showFill = \"true\";\n"
		"	showFrame = \"true\";\n"
		"	labelFillColor = \"0.0 1.0 0.0 1.0\"; // Fills with a solid green color\n"
		"	labelFrameColor = \"1.0 1.0 1.0 1.0\"; // Solid white frame color\n"
		"	showLabelFill = \"true\";\n"
		"	showLabelFrame = \"true\";\n"
		"	verticalOffset = \"0.15\";\n"
		"	distanceFade = \"15.0\";\n"
		"};\n"
   "@endtsexample\n\n"
   
   "@ingroup GuiGame\n"
);

/// Default distance for object's information to be displayed.
static const F32 cDefaultVisibleDistance = 500.0f;

GuiShapeNameHud::GuiShapeNameHud()
{
   mFillColor.set( 0.25f, 0.25f, 0.25f, 0.25f );
   mFrameColor.set( 0, 1, 0, 1 );
   mLabelFillColor.set( 0.25f, 0.25f, 0.25f, 0.25f );
   mLabelFrameColor.set( 0, 1, 0, 1 );
   mTextColor.set( 0, 1, 0, 1 );
   mShowFrame = mShowFill = true;
   mShowLabelFrame = mShowLabelFill = false;
   mVerticalOffset = 0.5f;
   mDistanceFade = 0.1f;
	mLabelPadding.set(0, 0);
	mTime = 0;
	mDefaultLifetime = 3;
}

void GuiShapeNameHud::initPersistFields()
{
   addGroup("Colors");     
   addField( "fillColor",  TypeColorF, Offset( mFillColor, GuiShapeNameHud ), "Standard color for the background of the control." );
   addField( "frameColor", TypeColorF, Offset( mFrameColor, GuiShapeNameHud ), "Color for the control's frame."  );
   addField( "textColor",  TypeColorF, Offset( mTextColor, GuiShapeNameHud ), "Color for the text on this control." );
   addField( "labelFillColor",  TypeColorF, Offset( mLabelFillColor, GuiShapeNameHud ), "Color for the background of each shape name label." );
   addField( "labelFrameColor", TypeColorF, Offset( mLabelFrameColor, GuiShapeNameHud ), "Color for the frames around each shape name label."  );
   endGroup("Colors");     

   addGroup("Misc");       
   addField( "showFill",   TypeBool, Offset( mShowFill, GuiShapeNameHud ), "If true, we draw the background color of the control." );
   addField( "showFrame",  TypeBool, Offset( mShowFrame, GuiShapeNameHud ), "If true, we draw the frame of the control."  );
   addField( "showLabelFill",  TypeBool, Offset( mShowLabelFill, GuiShapeNameHud ), "If true, we draw a background for each shape name label." );
   addField( "showLabelFrame", TypeBool, Offset( mShowLabelFrame, GuiShapeNameHud ), "If true, we draw a frame around each shape name label."  );
   addField( "labelPadding", TypePoint2I, Offset( mLabelPadding, GuiShapeNameHud ), "The padding (in pixels) between the label text and the frame." );
   addField( "verticalOffset", TypeF32, Offset( mVerticalOffset, GuiShapeNameHud ), "Amount to vertically offset the control in relation to the ShapeBase object in focus. Does not effect elements" );
   addField( "distanceFade", TypeF32, Offset( mDistanceFade, GuiShapeNameHud ), "Visibility distance (how far the player must be from the ShapeBase object in focus) for this control to render." );
   endGroup("Misc");
   Parent::initPersistFields();
}

//----------------------------------------------------------------------------
/// Primary method for interating with the interal list
///
/// This method will add a node to the internal list of elements.
/// Elements with a lifetime near 100k may not ever fade. Named objects
/// should be used when either a short or infinite life is not wanted.
void GuiShapeNameHud::addElement(Point3F pos, String damage, F32 lifeTime, ColorF textColor)
{
	// Create the new node and assign its values
	mElementInfo elementInfo;

	elementInfo.position = pos;
	elementInfo.damage = damage;
	elementInfo.color = textColor;

	// Get creation time of this node
	elementInfo.time = mTime;

	// Add it to the element list
	mElementList.push_front(elementInfo);
}

//----------------------------------------------------------------------------
/// Timer used when calculating node age
///
/// 100k lifetime issue is caused by the intentional timer rollover.
void GuiShapeNameHud::advanceTime(F32 timeDelta)
{
	// Advance the timer
	mTime += timeDelta;

	// Roll the timer back after it reaches a certain value.
	// Might be unnecessary.
	if (mTime > 100000)
		mTime = 100000 - mTime;
}

//----------------------------------------------------------------------------
/// Core rendering method for this control.
///
/// This method scans through all the current client ShapeBase objects.
/// If one is named, it displays the name and damage information for it.
///
/// Information is offset from the center of the object's bounding box,
/// unless the object is a PlayerObjectType, in which case the eye point
/// is used.
///
/// @param   updateRect   Extents of control.
void GuiShapeNameHud::onRender( Point2I, const RectI &updateRect)
{
   // Background fill first
   if (mShowFill)
      GFX->getDrawUtil()->drawRectFill(updateRect, mFillColor);

   // Must be in a TS Control
   GuiTSCtrl *parent = dynamic_cast<GuiTSCtrl*>(getParent());
   if (!parent) return;

   // Must have a connection and control object
   GameConnection* conn = GameConnection::getConnectionToServer();
   if (!conn) return;
   GameBase * control = dynamic_cast<GameBase*>(conn->getControlObject());
   if (!control) return;

   // Get control camera info
   MatrixF cam;
   Point3F camPos;
   VectorF camDir;
   conn->getControlCameraTransform(0,&cam);
   cam.getColumn(3, &camPos);
   cam.getColumn(1, &camDir);

   F32 camFovCos;
   conn->getControlCameraFov(&camFovCos);
   camFovCos = mCos(mDegToRad(camFovCos) / 2);

   // Visible distance info & name fading
   F32 visDistance = gClientSceneGraph->getVisibleDistance();
   F32 visDistanceSqr = visDistance * visDistance;
   F32 fadeDistance = visDistance * mDistanceFade;

   // Collision info. We're going to be running LOS tests and we
   // don't want to collide with the control object.
   static U32 losMask = TerrainObjectType | ShapeBaseObjectType | StaticObjectType;
   control->disableCollision();

   // All ghosted objects are added to the server connection group,
   // so we can find all the shape base objects by iterating through
   // our current connection.
   for (SimSetIterator itr(conn); *itr; ++itr) {
      ShapeBase* shape = dynamic_cast< ShapeBase* >(*itr);
      if ( shape ) {
         if (shape != control && shape->getShapeName()) 
         {

            // Target pos to test, if it's a player run the LOS to his eye
            // point, otherwise we'll grab the generic box center.
            Point3F shapePos;
            if (shape->getTypeMask() & PlayerObjectType) 
            {
               MatrixF eye;

               // Use the render eye transform, otherwise we'll see jittering
               shape->getRenderEyeTransform(&eye);
               eye.getColumn(3, &shapePos);
            } 
            else 
            {
                // Use the render transform instead of the box center
                // otherwise it'll jitter.
               MatrixF srtMat = shape->getRenderTransform();
               srtMat.getColumn(3, &shapePos);
            }
            VectorF shapeDir = shapePos - camPos;

            // Test to see if it's in range
            F32 shapeDist = shapeDir.lenSquared();
            if (shapeDist == 0 || shapeDist > visDistanceSqr)
               continue;
            shapeDist = mSqrt(shapeDist);

            // Test to see if it's within our viewcone, this test doesn't
            // actually match the viewport very well, should consider
            // projection and box test.
            shapeDir.normalize();
            F32 dot = mDot(shapeDir, camDir);
            if (dot < camFovCos)
               continue;

            // Test to see if it's behind something, and we want to
            // ignore anything it's mounted on when we run the LOS.
            RayInfo info;
            shape->disableCollision();
            SceneObject *mount = shape->getObjectMount();
            if (mount)
               mount->disableCollision();
            bool los = !gClientContainer.castRay(camPos, shapePos,losMask, &info);
            shape->enableCollision();
            if (mount)
               mount->enableCollision();
            if (!los)
               continue;

            // Project the shape pos into screen space and calculate
            // the distance opacity used to fade the labels into the
            // distance.
            Point3F projPnt;
            shapePos.z += mVerticalOffset;
            if (!parent->project(shapePos, &projPnt))
               continue;
            F32 opacity = (shapeDist < fadeDistance)? 1.0:
               1.0 - (shapeDist - fadeDistance) / (visDistance - fadeDistance);

            // Render the shape's name
            drawName(Point2I((S32)projPnt.x, (S32)projPnt.y),shape->getShapeName(),opacity, mTextColor);
         }
      }
   }

	// Check to see if there is anything in our internal list. Iterate through it and render
	// the node contents if so.
	if (mElementList.size() > 0)
	{
		for (std::list<mElementInfo>::iterator i = mElementList.begin(); i != mElementList.end(); ++i)
		{
			// Get the node info
			mElementInfo testInfo = *i;
			Point3F position = testInfo.position;
			String damage = testInfo.damage;

			VectorF shapeDir = position - camPos;

			// Test to see if it's in range
			F32 shapeDist = shapeDir.lenSquared();
			if (shapeDist == 0 || shapeDist > visDistanceSqr)
				continue;
			shapeDist = mSqrt(shapeDist);

			// Test to see if it's within our viewcone, this test doesn't
			// actually match the viewport very well, should consider
			// projection and box test.
			shapeDir.normalize();
			F32 dot = mDot(shapeDir, camDir);
			if (dot < camFovCos)
				continue;

			// Test to see if it's behind something, and we want to
			// ignore anything it's mounted on when we run the LOS.
			RayInfo infor;

			bool los = !gClientContainer.castRay(camPos, position, losMask, &infor);

			if (!los)
				continue;

			// Project the shape pos into screen space and calculate
			// the distance opacity used to fade the labels into the
			// distance.
			Point3F projPnt;
			if (!parent->project(position, &projPnt))
				continue;
			F32 opacity = (shapeDist < fadeDistance) ? 1.0 :
				1.0 - (shapeDist - fadeDistance) / (visDistance - fadeDistance);

			// Render the elements's string
			drawName(Point2I((S32)projPnt.x, (S32)projPnt.y), damage, opacity, testInfo.color);

			// Check the nodes age while we are here.
			F32 timeDelta = mTime - testInfo.time;

			// Is the node is older then its lifetime, remove it
			if (timeDelta > testInfo.lifeTime)
				mElementList.erase(i);

			// If the node age is negative, the timer rolled over. Adjust the lifetime check
			// This check will fail if the node has a long lifetime. Elements were not with
			// persistence in mind.
			else if (timeDelta < 0 && (mTime + 100000 - testInfo.time) > testInfo.lifeTime)
				mElementList.erase(i);
		}
	}

	// Restore control object collision
	control->enableCollision();

   // Border last
   if (mShowFrame)
      GFX->getDrawUtil()->drawRect(updateRect, mFrameColor);
}


//----------------------------------------------------------------------------
/// Render object names.
///
/// Helper function for GuiShapeNameHud::onRender
///
/// @param   offset  Screen coordinates to render name label. (Text is centered
///                  horizontally about this location, with bottom of text at
///                  specified y position.)
/// @param   name    String name to display.
/// @param   opacity Opacity of name (a fraction).
void GuiShapeNameHud::drawName(Point2I offset, const char *name, F32 opacity, ColorF color)
{
   F32 width = mProfile->mFont->getStrWidth((const UTF8 *)name) + mLabelPadding.x * 2;
   F32 height = mProfile->mFont->getHeight() + mLabelPadding.y * 2;
   Point2I extent = Point2I(width, height);

   // Center the name
   offset.x -= width / 2;
   offset.y -= height / 2;

   GFXDrawUtil* drawUtil = GFX->getDrawUtil();

   // Background fill first
   if (mShowLabelFill)
      drawUtil->drawRectFill(RectI(offset, extent), mLabelFillColor);

   // Deal with opacity and draw.
	color.alpha = opacity;
	drawUtil->setBitmapModulation(color);
   drawUtil->drawText(mProfile->mFont, offset + mLabelPadding, name);
   drawUtil->clearBitmapModulation();

   // Border last
   if (mShowLabelFrame)
      drawUtil->drawRect(RectI(offset, extent), mLabelFrameColor);
}

DefineEngineMethod(GuiShapeNameHud, addElement, bool, (Point3F pos, String damage, F32 lifeTime, ColorF inColor), (F32 (3), ColorF(1, 0, 0, 1)),
	"@brief Creates a floating text element at the specified location.\n\n"
	"Will return true if successfull, false if otherwise. A position and name must be provided. Extremely long lived elements may never fade"
	"When a color but no lifetime is provided, (,\"\",)can be used in place of (,,) to avoid torquescript warnings.\n")
{
	if (!(lifeTime > 0) || damage.isEmpty())
		return false;

	object->addElement(pos, damage, lifeTime, inColor);
	return true;
}

DefineEngineMethod(GuiShapeNameHud, getElementCount, S32, (void),,
	"@brief Counts the number of elements on screen.\n\n"
	"Counts the number of active elements. Used for metrics and debugging"
	"Does not count object names.\n")
{
	return object->getElementCount();
}

DefineEngineMethod(GuiShapeNameHud, clearElements, void, (void),,
	"@brief Clears the element list.\n\n"
	"Clears all indicator elements from the guiShapeNameHud"
	"Will not clear names originating from objects.\n")
{
	object->clearElements();
}

