#include "GarageLayer.h"
#include "GameToolbox.h"
#include "GameManager.h"
#include "MenuLayer.h"
#include "MenuItemSpriteExtra.h"

using namespace ax;

Scene* GarageLayer::scene() {
    auto scene = Scene::create();
    scene->addChild(GarageLayer::create());
    return scene;
}

bool GarageLayer::init() {
    if (!Layer::init()) return false;

    auto winSize = Director::getInstance()->getWinSize();
    
    auto background = Sprite::create("GJ_gradientBG.png");
    background->setScale(winSize.width / background->getContentSize().width, winSize.height / background->getContentSize().height);
    background->setAnchorPoint({0, 0});
    background->setColor({175, 175, 175});
    this->addChild(background);

    auto leftcorner = Sprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    leftcorner->setPosition({0, winSize.height});
    leftcorner->setAnchorPoint({0, 1});
    leftcorner->setFlippedY(true);
    this->addChild(leftcorner);

    auto rightcorner = Sprite::createWithSpriteFrameName("GJ_sideArt_001.png");
    rightcorner->setPosition(winSize);
    rightcorner->setAnchorPoint({1, 1});
    rightcorner->setFlippedY(true);
    rightcorner->setFlippedX(true);
    this->addChild(rightcorner); 

    if (GameManager::getInstance()->getVariable<std::string>("player-username").compare("")) {
        GameManager::getInstance()->setVariable("player-username", "Player");
        GameManager::getInstance()->save();
    }
    _pUsernamefield = ui::TextField::create("Player", "bigFont.fnt", 48);
    _pUsernamefield->setString(GameManager::getInstance()->getVariable<std::string>("player-username"));
    _pUsernamefield->setPlaceHolderColor({ 120, 170, 240 });
    _pUsernamefield->setMaxLength(20);
    _pUsernamefield->setMaxLengthEnabled(true);
    _pUsernamefield->setCursorEnabled(true);
    _pUsernamefield->setPosition({winSize.width / 2, winSize.height - 80});
    this->addChild(_pUsernamefield);

    auto nameTxt = Sprite::createWithSpriteFrameName("GJ_nameTxt_001.png");
    nameTxt->setPosition(_pUsernamefield->getPosition() + Vec2({-250, 20}));
    this->addChild(nameTxt);

    auto floor = Sprite::create("floor.png");
    floor->setPosition({winSize.width / 2, (winSize.height / 2) + 20});
    this->addChild(floor);

    auto bottomBG = Sprite::create("edit_barBG_001.png");
    bottomBG->setScaleX(winSize.width / bottomBG->getContentSize().width);
    bottomBG->setAnchorPoint({0, 0});
    this->addChild(bottomBG);

    this->_pPreviewPlayer = SimplePlayer::create(GameManager::getInstance()->getVariable<int>("player-cube"));
    _pPreviewPlayer->setPosition({winSize.width / 2, floor->getPositionY() + floor->getContentSize().height / 2});
    _pPreviewPlayer->setAnchorPoint({0.5, 0});
    _pPreviewPlayer->setScale(1.6f);
    _pPreviewPlayer->setMainColor(GameToolbox::iconColors[GameManager::getInstance()->getVariable<int>("player-main-color")]);
    _pPreviewPlayer->setSecondaryColor(GameToolbox::iconColors[GameManager::getInstance()->getVariable<int>("player-secondary-color")]);
    this->addChild(_pPreviewPlayer);
	
    auto square = ui::Scale9Sprite::create("square02_001.png");
    square->setAnchorPoint({0.5, 1});
    square->setPosition({winSize.width / 2, (winSize.height / 2) - 40});
    square->setContentSize({960, 110});
    square->setOpacity(75);
    this->addChild(square);

    auto hint = Sprite::createWithSpriteFrameName("GJ_unlockTxt_001.png");
    hint->setAnchorPoint({1, 0.5});
    hint->setPosition({winSize.width / 2 + square->getContentSize().width / 2 - 40, square->getPositionY() + 2});
    this->addChild(hint);
    
    auto iconsMenu = Menu::create();
    
    for (int i = 1; i <= 13; i++) {
        auto player = SimplePlayer::create(i);
        player->setMainColor({158, 158, 158});
        player->setSecondaryColor({255, 255, 255});
        
        auto btn = MenuItemSpriteExtra::create(player, [=](Node* btn) {
            this->_pSelectedCube = btn->getTag() - 1;
            GameManager::getInstance()->setVariable("player-cube", btn->getTag());
            this->_pSelectionFrame->setPosition(iconsMenu->convertToWorldSpace(btn->getPosition()));

            this->_pPreviewPlayer->removeFromParentAndCleanup(true);
            //this->_pPreviewPlayer->release(); // this shit crashes dont do this
            this->_pPreviewPlayer = SimplePlayer::create(btn->getTag());
            _pPreviewPlayer->setPosition({winSize.width / 2, floor->getPositionY() + floor->getContentSize().height / 2});
            _pPreviewPlayer->setAnchorPoint({0.5, 0});
            _pPreviewPlayer->setScale(1.6f);
            _pPreviewPlayer->setMainColor(GameToolbox::iconColors[GameManager::getInstance()->getVariable<int>("player-main-color")]);
            _pPreviewPlayer->setSecondaryColor(GameToolbox::iconColors[GameManager::getInstance()->getVariable<int>("player-secondary-color")]);
            this->addChild(_pPreviewPlayer);
        });
        
        btn->setTag(i);

        iconsMenu->addChild(btn);
    }

    iconsMenu->setPosition({winSize.width / 2, square->getPositionY() - square->getContentSize().height / 2});
    iconsMenu->alignItemsHorizontallyWithPadding(12);
    this->addChild(iconsMenu);


    auto colorsMenu = Menu::create();

    for (int i = 0; i < GameToolbox::iconColors.size(); i++) {
        auto colorSprite = Sprite::create("square.png");
        colorSprite->setColor(GameToolbox::iconColors[i]);
        auto btn = MenuItemSpriteExtra::create(colorSprite, [=](Node* button) {
            this->_pSelectionFrame2->setPosition({colorsMenu->getPositionX()+button->getPositionX(),colorsMenu->getPositionY()});
            // log_ << _pSelectionFrame2->getPositionX();
            
            _pPreviewPlayer->setMainColor(GameToolbox::iconColors[i]);
            
            GameManager::getInstance()->setVariable("player-main-color", button->getTag());
           });
        btn->setScale(1.7f);
        // btn->setNewScale(1.25f);
        btn->setTag(i);
        colorsMenu->addChild(btn);
    }

    colorsMenu->setPosition({ winSize.width / 2, square->getPositionY() - square->getContentSize().height / 2 - 125 });
    colorsMenu->alignItemsHorizontallyWithPadding(15);
    this->addChild(colorsMenu);


    auto secondaryColorsMenu = Menu::create();

    for (int i = 0; i < GameToolbox::iconColors.size(); i++) {
        auto colorSprite = Sprite::create("square.png");
        colorSprite->setColor(GameToolbox::iconColors[i]);
        auto btn = MenuItemSpriteExtra::create(colorSprite, [=](Node* button) {
            this->_pSelectionFrame3->setPosition({ secondaryColorsMenu->getPositionX() + button->getPositionX(),secondaryColorsMenu->getPositionY() });
            // log_ << _pSelectionFrame3->getPositionX();

            _pPreviewPlayer->setSecondaryColor(GameToolbox::iconColors[i]);
            GameManager::getInstance()->setVariable("player-secondary-color", button->getTag());

            });
        btn->setScale(1.7f);
        // btn->setNewScale(1.25f);
        btn->setTag(i);
        secondaryColorsMenu->addChild(btn);
    }

    secondaryColorsMenu->setPosition({ winSize.width / 2, square->getPositionY() - square->getContentSize().height / 2 - 190 });
    secondaryColorsMenu->alignItemsHorizontallyWithPadding(15);
    this->addChild(secondaryColorsMenu);


    this->_pSelectedCube = GameManager::getInstance()->getVariable<int>("player-cube") - 1;
    this->_pSelectedMC = GameManager::getInstance()->getVariable<int>("player-main-color");
    this->_pSelectedSC = GameManager::getInstance()->getVariable<int>("player-secondary-color");
    this->_pSelectionFrame = Sprite::createWithSpriteFrameName("GJ_select_001.png");
    this->_pSelectionFrame2 = Sprite::createWithSpriteFrameName("GJ_select_001.png");
    this->_pSelectionFrame3 = Sprite::createWithSpriteFrameName("GJ_select_001.png");

    if (this->_pSelectedMC < 0) {
        this->_pSelectedMC++;
    }
    if (this->_pSelectedCube < 0) {
        this->_pSelectedCube++;
    }
    if (this->_pSelectedSC < 0) {
        this->_pSelectedSC++;
    }

    this->addChild(this->_pSelectionFrame);
    this->addChild(this->_pSelectionFrame2);
    this->addChild(this->_pSelectionFrame3);

    this->_pSelectionFrame->setPosition(iconsMenu->convertToWorldSpace(iconsMenu->getChildren().at(_pSelectedCube)->getPosition()));

    this->_pSelectionFrame2->setPositionX(colorsMenu->getPositionX() + colorsMenu->getChildren().at(_pSelectedMC)->getPositionX());
    this->_pSelectionFrame3->setPositionX(secondaryColorsMenu->getPositionX() + secondaryColorsMenu->getChildren().at(_pSelectedSC)->getPositionX());

    this->_pSelectionFrame2->setScale(0.95f);
    this->_pSelectionFrame3->setScale(0.95f);

    this->_pSelectionFrame2->setPositionY(square->getPositionY() - square->getContentSize().height / 2 - 125);
    this->_pSelectionFrame3->setPositionY(square->getPositionY() - square->getContentSize().height / 2 - 190);

    auto backBtn = MenuItemSpriteExtra::create("GJ_arrow_03_001.png", [&](Node* btn) {
        GameManager::getInstance()->setVariable("player-username", this->_pUsernamefield->getString());
        Director::getInstance()->replaceScene(TransitionFade::create(0.5f, MenuLayer::scene()));
    });
    
    auto menu = Menu::create(backBtn, nullptr);
    this->addChild(menu);
    backBtn->setPosition(menu->convertToNodeSpace({48, winSize.height - 46}));
    //paletteBtn->setPosition(menu->convertToNodeSpace({48, winSize.height - 146}));

    return true;
}