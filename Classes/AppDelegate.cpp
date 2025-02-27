/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 Copyright (c) 2021 Bytedance Inc.

 https://axmolengine.github.io/

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "LoadingLayer.h"
#include "GameToolbox.h"

#define USE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE
#include <audio/AudioEngine.h>
#endif

USING_NS_AX;

static ax::Size designResolutionSize = ax::Size(1280, 720);
static ax::Size smallResolutionSize = ax::Size(480, 320);
static ax::Size mediumResolutionSize = ax::Size(1024, 768);
static ax::Size largeResolutionSize = ax::Size(2048, 1536);

AppDelegate::AppDelegate() {}

AppDelegate::~AppDelegate() {}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
	// set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
	GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

	GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,
// don't modify or remove this function
static int register_all_packages()
{
	return 0; // flag for packages manager
}

int AppDelegate::applicationGetRefreshRate()
{
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
	auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	return mode->refreshRate;
#else
	return 60;
#endif
}
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || \
	(AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
void AppDelegate::onGLFWwindowSizeCallback(GLFWwindow*, int w, int h)
{
	auto director = Director::getInstance();
	auto glView = director->getOpenGLView();

	glView->setFrameSize(w, h);

	
	// uhh stuff to make different aspect ratios work ig?
	if (w > 1280)
		glView->setDesignResolutionSize(569 + (w - 1280), 320,
			ResolutionPolicy::FIXED_HEIGHT);
	else
		glView->setDesignResolutionSize(569, 320 - (w - 1280),
			ResolutionPolicy::FIXED_WIDTH);

	if (h > 720)
		glView->setDesignResolutionSize(569, 320 + (h - 720),
			ResolutionPolicy::FIXED_WIDTH);
	else
		glView->setDesignResolutionSize(569 - (h - 720), 320,
			ResolutionPolicy::FIXED_HEIGHT);
	
	glView->setDesignResolutionSize(569, 320,
		ResolutionPolicy::SHOW_ALL);

	director->getEventDispatcher()->dispatchCustomEvent(GLViewImpl::EVENT_WINDOW_RESIZED, nullptr);
}
#endif
bool AppDelegate::applicationDidFinishLaunching()
{
	// initialize director
	auto director = Director::getInstance();
	auto glView = director->getOpenGLView();
	if (!glView)
	{
#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || \
	(AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)
		glView = GLViewImpl::createWithRect(
			"OpenGD", ax::Rect(0, 0, 1280, 720), 1.f, true);
#else
		glView = GLViewImpl::create("OpenGD");
#endif
		
		/*
		auto full = dynamic_cast<GLViewImpl *>(glView);
		full->setFullscreen();
		*/
		director->setOpenGLView(glView);
	}

	// turn off display FPS
	director->setStatsDisplay(true);

	// set FPS. the default value is 1.0/60 if you don't call this
	director->setAnimationInterval(1.0f / applicationGetRefreshRate());

	// Set the design resolution
	glView->setDesignResolutionSize(569, 320,
									ResolutionPolicy::SHOW_ALL);

	// uhh stuff to make different aspect ratios work ig?
	if (glView->getFrameSize().width > 1280)
		glView->setDesignResolutionSize(569 + (glView->getFrameSize().width - 1280), 320,
			ResolutionPolicy::FIXED_HEIGHT);
	else
		glView->setDesignResolutionSize(569, 320 - (glView->getFrameSize().width - 1280),
			ResolutionPolicy::FIXED_WIDTH);

	if (glView->getFrameSize().height > 720)
		glView->setDesignResolutionSize(569, 320 + (glView->getFrameSize().height - 720),
			ResolutionPolicy::FIXED_WIDTH);
	else
		glView->setDesignResolutionSize(569 - (glView->getFrameSize().height - 720), 320,
			ResolutionPolicy::FIXED_HEIGHT);

#if (AX_TARGET_PLATFORM == AX_PLATFORM_WIN32) || (AX_TARGET_PLATFORM == AX_PLATFORM_MAC) || \
	(AX_TARGET_PLATFORM == AX_PLATFORM_LINUX)

	glfwSetWindowAspectRatio(static_cast<GLViewImpl*>(glView)->getWindow(), 16, 9);

	glfwSetWindowSizeCallback(static_cast<GLViewImpl*>(glView)->getWindow(), AppDelegate::onGLFWwindowSizeCallback);

#endif

	director->setContentScaleFactor(2.0f);

	register_all_packages();

	// create a scene. it's an autorelease object
	auto scene = ax::Scene::create();
	scene->addChild(LoadingLayer::create());
	director->runWithScene(scene);

	return true;
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground()
{
	Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
	AudioEngine::pauseAll();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
	Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
	AudioEngine::resumeAll();
#endif
}
