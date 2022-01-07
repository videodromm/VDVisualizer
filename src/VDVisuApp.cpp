
 
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

using namespace ci;
using namespace ci::app;
using namespace videodromm;

class VDVisuApp : public App {
public:
	VDVisuApp();
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
	// Uniform
	VDUniformsRef					mVDUniforms;
	// Params
	VDParamsRef						mVDParams;

	bool							mFadeInDelay = true;
	void							toggleCursorVisibility(bool visible);
};


VDVisuApp::VDVisuApp()
{

	// Settings
	mVDSettings = VDSettings::create("Visu");
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
		->setUniformValue(mVDUniforms->IDISPLAYMODE, VDDisplayMode::POST)
		->setupSession()
		->setupWSClient()
		->wsConnect()
		//->setupOSCReceiver()
		//->addOSCObserver(mVDSettings->mOSCDestinationHost, mVDSettings->mOSCDestinationPort)
		->addUIObserver(mVDSettings, mVDUniforms)
		->toggleUI()
		->setUniformValue(mVDUniforms->IBPM, 160.0f)
		->setUniformValue(mVDUniforms->IMOUSEX, 0.27710f)
		->setUniformValue(mVDUniforms->IMOUSEY, 0.5648f);

	// sos only mVDSessionFacade->setUniformValue(mVDSettings->IEXPOSURE, 1.93f);
	mFadeInDelay = true;
}

void VDVisuApp::toggleCursorVisibility(bool visible)
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

void VDVisuApp::fileDrop(FileDropEvent event)
{
	mVDSessionFacade->fileDrop(event);
}

void VDVisuApp::mouseMove(MouseEvent event)
{
	if (!mVDSessionFacade->handleMouseMove(event)) {

	}
}

void VDVisuApp::mouseDown(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseDown(event)) {

	}
}

void VDVisuApp::mouseDrag(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseDrag(event)) {

	}
}

void VDVisuApp::mouseUp(MouseEvent event)
{

	if (!mVDSessionFacade->handleMouseUp(event)) {

	}
}

void VDVisuApp::keyDown(KeyEvent event)
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

void VDVisuApp::keyUp(KeyEvent event)
{

	// let your application perform its keyUp handling here
	if (!mVDSessionFacade->handleKeyUp(event)) {
		/*switch (event.getCode()) {
		default:
			CI_LOG_V("main keyup: " + toString(event.getCode()));
			break;
		}*/
	}
}
void VDVisuApp::cleanup()
{
	CI_LOG_V("cleanup and save");
	mVDSessionFacade->saveWarps();
	mVDSettings->save();
	CI_LOG_V("quit");
}

void VDVisuApp::update()
{
	
	mVDSessionFacade->setUniformValue(mVDUniforms->IFPS, getAverageFps());
	mVDSessionFacade->update();

}


void VDVisuApp::resize()
{
}
void VDVisuApp::draw()
{
	// clear the window and set the drawing color to black
	gl::clear();
	gl::color(Color::white());
	if (mFadeInDelay) {
		mVDSettings->iAlpha = 0.0f;
		if (getElapsedFrames() > 10.0) {// mVDSessionFacade->getFadeInDelay()) {
			mFadeInDelay = false;
			timeline().apply(&mVDSettings->iAlpha, 0.0f, 1.0f, 1.5f, EaseInCubic());
		}
	}
	else {
		gl::setMatricesWindow(mVDParams->getFboWidth(), mVDParams->getFboHeight());
		//gl::setMatricesWindow(mVDSessionFacade->getIntUniformValueByIndex(mVDSettings->IOUTW), mVDSessionFacade->getIntUniformValueByIndex(mVDSettings->IOUTH), true);
		// textures needs updating ?
		/*for (int t = 0; t < mVDSessionFacade->getInputTexturesCount(); t++) {
			mVDSessionFacade->getInputTexture(t);
		}*/
		
		int m = mVDSessionFacade->getUniformValue(mVDUniforms->IDISPLAYMODE);
		if (m == VDDisplayMode::MIXETTE) {
			gl::draw(mVDSessionFacade->buildRenderedMixetteTexture(0));
			
		}
		else if (m == VDDisplayMode::POST) {
			gl::draw(mVDSessionFacade->buildPostFboTexture());
			
		}
		else if (m == VDDisplayMode::FX) {
			gl::draw(mVDSessionFacade->buildFxFboTexture());
			
		}
		else {
			if (m < mVDSessionFacade->getFboShaderListSize()) {
				gl::draw(mVDSessionFacade->getFboShaderTexture(m));
				
			}
			else {
				gl::draw(mVDSessionFacade->buildRenderedMixetteTexture(0), Area(50, 50, mVDParams->getFboWidth() / 2, mVDParams->getFboHeight() / 2));
				gl::draw(mVDSessionFacade->buildPostFboTexture(), Area(mVDParams->getFboWidth() / 2, mVDParams->getFboHeight() / 2, mVDParams->getFboWidth(), mVDParams->getFboHeight()));
			}
			//gl::draw(mVDSession->getRenderedMixetteTexture(0), Area(0, 0, mVDSettings->mFboWidth, mVDSettings->mFboHeight));
			// ok gl::draw(mVDSession->getWarpFboTexture(), Area(0, 0, mVDSettings->mFboWidth, mVDSettings->mFboHeight));//getWindowBounds()
		}
	}
	getWindow()->setTitle(toString((int)getAverageFps()) + " fps");
}
void prepareSettings(App::Settings *settings)
{
	settings->setWindowSize(1280, 720);
}
CINDER_APP(VDVisuApp, RendererGl(RendererGl::Options().msaa(8)),  prepareSettings)
