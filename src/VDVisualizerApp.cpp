/*
 Copyright (c) 2013-2020, Bruce Lane - All rights reserved.
 This code is intended for use with the Cinder C++ library: http://libcinder.org

 Using Cinder-Warping from Paul Houx.

 Cinder-Warping is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Cinder-Warping is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Cinder-Warping.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

 // Animation
#include "VDAnimation.h"
// Session Facade
#include "VDSessionFacade.h"

// Uniforms
#include "VDUniforms.h"
// Params
#include "VDParams.h"
// Mix
#include "VDMix.h"

// UI
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS 1
#include "VDUI.h"
#define IM_ARRAYSIZE(_ARR)			((int)(sizeof(_ARR)/sizeof(*_ARR)))
using namespace ci;
using namespace ci::app;
using namespace videodromm;

class VDVisualizerApp : public App {
public:
	VDVisualizerApp();
	void cleanup() override;
	void update() override;
	void draw() override;
	void resize() override;
	void mouseMove(MouseEvent event) override;
	void mouseDown(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void keyDown(KeyEvent event) override;
	void keyUp(KeyEvent event) override;
	void fileDrop(FileDropEvent event) override;
private:
	// Settings
	VDSettingsRef					mVDSettings;
	// Animation
	VDAnimationRef					mVDAnimation;
	// Session
	VDSessionFacadeRef				mVDSessionFacade;
	// Mix
	VDMixRef						mVDMix;
	// Uniforms
	VDUniformsRef					mVDUniforms;
	// Params
	VDParamsRef						mVDParams;
	// UI
	VDUIRef							mVDUI;

	bool							mFadeInDelay = true;
	void							toggleCursorVisibility(bool visible);
};


VDVisualizerApp::VDVisualizerApp()
{

	// Settings
	mVDSettings = VDSettings::create("VDViz");
	// Uniform
	mVDUniforms = VDUniforms::create();
	// Params
	mVDParams = VDParams::create();
	// Animation
	mVDAnimation = VDAnimation::create(mVDSettings, mVDUniforms);
	// Mix
	mVDMix = VDMix::create(mVDSettings, mVDAnimation, mVDUniforms);
	// Session
	mVDSessionFacade = VDSessionFacade::createVDSession(mVDSettings, mVDAnimation, mVDUniforms, mVDMix)
		->setUniformValue(mVDUniforms->IBPM, 160.0f)
		->setUniformValue(mVDUniforms->IMOUSEX, 0.27710f)
		->setUniformValue(mVDUniforms->IMOUSEY, 0.5648f)
		->setMode(7)
		->setupWSClient()
		->wsConnect()
		//->setupOSCReceiver()
		//->addOSCObserver(mVDSettings->mOSCDestinationHost, mVDSettings->mOSCDestinationPort)
		->addUIObserver(mVDSettings, mVDUniforms)
		->toggleValue(mVDUniforms->IFLIPPOSTV)
		->toggleValue(mVDUniforms->IFLIPV);

	// sos only mVDSessionFacade->setUniformValue(mVDSettings->IEXPOSURE, 1.93f);
	mFadeInDelay = true;
	mVDUI = VDUI::create(mVDSettings, mVDSessionFacade, mVDUniforms);
}

void VDVisualizerApp::toggleCursorVisibility(bool visible)
{
	if (visible)
	{
		showCursor();
	}
	else
	{
		hideCursor();
	}
}

void VDVisualizerApp::fileDrop(FileDropEvent event)
{
	mVDSessionFacade->fileDrop(event);
}

void VDVisualizerApp::mouseMove(MouseEvent event)
{
	if (!mVDSessionFacade->handleMouseMove(event)) {

	}
}

void VDVisualizerApp::mouseDown(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseDown(event)) {

	}
}

void VDVisualizerApp::mouseDrag(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseDrag(event)) {

	}
}

void VDVisualizerApp::mouseUp(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseUp(event)) {

	}
}

void VDVisualizerApp::keyDown(KeyEvent event)
{

	// warp editor did not handle the key, so handle it here
	if (!mVDSessionFacade->handleKeyDown(event)) {
		switch (event.getCode()) {
		case KeyEvent::KEY_F12:
			// quit the application
			quit();
			break;
		case KeyEvent::KEY_f:
			// toggle full screen
			setFullScreen(!isFullScreen());
			break;

		case KeyEvent::KEY_l:
			mVDSessionFacade->createWarp();
			break;
		}
	}
}

void VDVisualizerApp::keyUp(KeyEvent event)
{

	// let your application perform its keyUp handling here
	if (!mVDSessionFacade->handleKeyUp(event)) {

	}
}
void VDVisualizerApp::cleanup()
{
	CI_LOG_V("cleanup and save");
	ui::Shutdown();

	mVDSettings->save();
	CI_LOG_V("quit");
}

void VDVisualizerApp::update()
{
	mVDSessionFacade->setUniformValue(mVDUniforms->IFPS, getAverageFps());
	mVDSessionFacade->update();
}


void VDVisualizerApp::resize()
{
	mVDUI->resize();
}
void VDVisualizerApp::draw()
{
	// clear the window and set the drawing color to white
	gl::clear();
	gl::color(Color::white());

	gl::setMatricesWindow(mVDParams->getFboWidth(), mVDParams->getFboHeight(), false);
	for (int t = 0; t < mVDSessionFacade->getInputTexturesCount(); t++) {
		mVDSessionFacade->getInputTexture(t);
	}
	/*for (int m = 0; m < mVDSessionFacade->getFboShaderListSize(); m++)
	{
		mVDSessionFacade->buildFboRenderedTexture(m);
	}*/

	//gl::setMatricesWindow(mVDSessionFacade->getIntUniformValueByIndex(mVDSettings->IOUTW), mVDSessionFacade->getIntUniformValueByIndex(mVDSettings->IOUTH), true);
	int m = mVDSessionFacade->getMode();
	if (m < mVDSessionFacade->getFboShaderListSize()) {
		gl::draw(mVDSessionFacade->getFboShaderTexture(m));
	}
	else {
		if (m == 8) {
			gl::draw(mVDSessionFacade->buildRenderedMixetteTexture(0));
		}
		else if (m == 7) {
			gl::draw(mVDSessionFacade->buildPostFboTexture());
		}
		else {
			gl::draw(mVDSessionFacade->buildRenderedMixetteTexture(0), Area(50, 50, mVDParams->getFboWidth() / 2, mVDParams->getFboHeight() / 2));
			gl::draw(mVDSessionFacade->buildPostFboTexture(), Area(mVDParams->getFboWidth() / 2, mVDParams->getFboHeight() / 2, mVDParams->getFboWidth(), mVDParams->getFboHeight()));
		}
	}
	// imgui
	if (mVDSessionFacade->showUI()) {
		mVDUI->Run("UI", (int)getAverageFps());
		if (mVDUI->isReady()) {
		}
	}
	getWindow()->setTitle(toString((int)getAverageFps()) + " fps");
}
void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
}
CINDER_APP(VDVisualizerApp, RendererGl(RendererGl::Options().msaa(8)), prepareSettings)
