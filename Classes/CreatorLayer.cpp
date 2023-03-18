#include "CreatorLayer.h"
#include "GameToolbox.h"
#include "MenuLayer.h"
#include "MenuItemSpriteExtra.h"
#include "ImGui/ImGuiPresenter.h"
#include "ImGui/imgui/imgui.h"
#include <ui/CocosGUI.h>
#include "PlayLayer.h"
#include "base64.h"

USING_NS_AX;
using namespace ax::network;

using GameToolbox::getTextureString;

Scene* CreatorLayer::scene() {
	
	return CreatorLayer::create();
}

bool CreatorLayer::init() {
	if (!Scene::init()) return false;
	
  //  ImGuiPresenter::getInstance()->setViewResolution(1280, 720);
   // ImGuiPresenter::getInstance()->addRenderLoop("#test", AX_CALLBACK_0(CreatorLayer::onDrawImGui, this), this);

	auto winSize = Director::getInstance()->getWinSize();

	auto bg = Sprite::create(getTextureString("GJ_gradientBG.png"));
	bg->setStretchEnabled(false);
	bg->setScaleX(winSize.width / bg->getContentSize().width);
	bg->setScaleY(winSize.height / bg->getContentSize().height);
	bg->setAnchorPoint({0, 0});
	bg->setColor({0, 102, 255});
	this->addChild(bg);

	GameToolbox::createAllCorners(this);

	auto createBtnSpr = Sprite::createWithSpriteFrameName("GJ_createBtn_001.png");
	auto savedBtnSpr = Sprite::createWithSpriteFrameName("GJ_savedBtn_001.png");
	auto featuredBtnSpr = Sprite::createWithSpriteFrameName("GJ_featuredBtn_001.png");
	auto searchBtnSpr = Sprite::createWithSpriteFrameName("GJ_searchBtn_001.png");

	auto savedBtn = MenuItemSpriteExtra::create(savedBtnSpr, [&](Node*) {
		GameToolbox::log("on saved");
	});
	auto featuredBtn = MenuItemSpriteExtra::create(featuredBtnSpr, [&](Node*) {
		GameToolbox::log("on featured");
	});
	auto searchBtn = MenuItemSpriteExtra::create(searchBtnSpr, [&](Node*) {
		std::string levelID = "128";
		std::string postData = fmt::format("levelID={}&secret=Wmfd2893gb7", levelID);
		GameToolbox::log("postData: {}", postData);
		HttpRequest* request = new HttpRequest();
		request->setTag(levelID);
		request->setUrl("http://www.boomlings.com/database/downloadGJLevel22.php");
		request->setRequestType(HttpRequest::Type::POST);
        request->setHeaders(std::vector<std::string>{"User-Agent: "});
		request->setRequestData(postData.c_str(), postData.length());
		request->setResponseCallback(AX_CALLBACK_2(CreatorLayer::onHttpRequestCompleted, this));
		request->setTag("GET test3");
		HttpClient::getInstance()->send(request);
		request->release();
	});
	auto createBtn = MenuItemSpriteExtra::create(createBtnSpr, [&](Node*) {
		GameToolbox::log("on create");
	});

	auto backBtn = MenuItemSpriteExtra::create("GJ_arrow_03_001.png", [&](Node*) {
		Director::getInstance()->replaceScene(TransitionFade::create(.5, MenuLayer::scene()));
	});

	auto menu = Menu::create(createBtn, savedBtn, featuredBtn, searchBtn, nullptr);
	GameToolbox::alignItemsInColumnsWithPadding(menu, 2, 130, 130);
	menu->addChild(backBtn);
	backBtn->setPosition(menu->convertToNodeSpace({25, winSize.height - 25}));
	
	this->addChild(menu);
	
	auto levelField = ui::TextField::create("Enter ID", GameToolbox::getTextureString("bigFont.fnt"), 20);
	levelField->setPlaceHolderColor({ 120, 170, 240 });
	levelField->setMaxLength(10);
	levelField->setMaxLengthEnabled(true);
	levelField->setCursorEnabled(true);
	levelField->setPosition({winSize.width / 2 + 150, winSize.height / 2});
	this->addChild(levelField);
	
	auto path = FileUtils::getInstance()->getWritablePath();
	GameToolbox::log("path: {}", path);

	return true;
	
}

void CreatorLayer::onHttpRequestCompleted(ax::network::HttpClient* sender, ax::network::HttpResponse* response)
{
	
	if(auto str = GameToolbox::getResponse(response))
	{
		std::vector<std::string> levelStuff = GameToolbox::splitByDelim(*str, ':');
		std::string compressedStr = fmt::format("{}", levelStuff.at(7));
		GameToolbox::log("compressed: {}", compressedStr);
		if(!compressedStr.empty())
		{
			//HELP
			// unsigned char* buffer = nullptr;
			// unsigned char* deflated = nullptr;
			// int decode_len = ax::base64::decode(
				// &buffer,
				// compressedStr.c_str(),
				// compressedStr.size()
			// );
			// GameToolbox::log("000000000");
			// ssize_t deflated_len = ZipUtils::inflateMemory(buffer, decode_len, &deflated);
			// GameToolbox::log("11111111");
			// std::string levelStr(reinterpret_cast<char*>(deflated));
			
			// GameToolbox::log("final: {}", levelStr);
			// return;
			// auto level = GJGameLevel::createWithMinimumData("Stereo Madness", "RobTop", 1);
			// level->_LevelString = levelStr;
			// Director::getInstance()->replaceScene(TransitionFade::create(0.5f, PlayLayer::scene(level)));
			
		}
	}
	else
		GameToolbox::log("request failed");
	
}

/*
static bool show_test_window	= true;
static bool show_another_window = true;
static ImVec4 clear_color	   = ImColor(114, 144, 154);

void CreatorLayer::onDrawImGui()
{
	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_color);
		if (ImGui::Button("Test Window"))
			show_test_window ^= 1;
		if (ImGui::Button("Another Window"))
			show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
					ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::SetNextWindowSize(ImVec2(170, 80), ImGuiCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);

		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::ShowDemoWindow();
	}
}
*/