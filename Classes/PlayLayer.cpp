#include "PlayLayer.h"
#include "AudioEngine.h"
#include "EffectGameObject.h"
#include "GameToolbox.h"
#include "ImGui/ImGuiPresenter.h"
#include "ImGui/imgui/imgui.h"
#include "LevelSelectLayer.h"
#include "LevelTools.h"
#include "MenuItemSpriteExtra.h"
#include <fstream>
#include <LevelPage.h>
#include "constants.h"

USING_NS_AX;
USING_NS_AX_EXT;

bool showDn = false, noclip = false;

float gameSpeed = 1, fps = 0;

static PlayLayer* Instance = nullptr;

ax::Node* cameraFollow;

Scene* PlayLayer::scene(GJGameLevel* level)
{
	auto scene = Scene::create();
	scene->addChild(PlayLayer::create(level));
	return scene;
}

int PlayLayer::sectionForPos(float x)
{
	int section = x / 100;
	if (section < 0) section = 0;
	return section;
}

PlayLayer* PlayLayer::create(GJGameLevel* level)
{
	auto ret = new (std::nothrow) PlayLayer();
	if (ret && ret->init(level))
	{
		ret->autorelease();
		return ret;
	}

	AX_SAFE_DELETE(ret);
	return nullptr;
}

void PlayLayer::fillColorChannel(std::vector<std::string>& colorString, int id)
{
	for (size_t j = 0; j < colorString.size() - 1; j += 2)
	{
		switch (std::stoi(colorString[j]))
		{
		case 1:
			m_pColorChannels.insert({id, ax::Color3B(std::stof(colorString[j + 1]), 0, 0)});
			break;
		case 2:
			m_pColorChannels.at(id).g = std::stof(colorString[j + 1]);
			break;
		case 3:
			m_pColorChannels.at(id).b = std::stof(colorString[j + 1]);
			break;
		}
	}
}
void PlayLayer::loadLevel(std::string levelStr)
{
	std::string levelString = GJGameLevel::decompressLvlStr(levelStr);

	std::vector<std::string> objData = GameToolbox::splitByDelim(levelString, ';'), levelData;

	levelData = GameToolbox::splitByDelim(objData[0], ',');
	objData.erase(objData.begin());

	for (size_t i = 0; i < levelData.size() - 1; i += 2)
	{
		if (levelData[i] == "kS1")
		{
			m_pColorChannels.insert({1000, ax::Color3B(std::stof(levelData[i + 1]), 0, 0)});
		}
		else if (levelData[i] == "kS2")
		{
			m_pColorChannels.at(1000).g = std::stof(levelData[i + 1]);
		}
		else if (levelData[i] == "kS3")
		{
			m_pColorChannels.at(1000).b = std::stof(levelData[i + 1]);
		}
		else if (levelData[i] == "kS4")
		{
			m_pColorChannels.insert({1001, ax::Color3B(std::stof(levelData[i + 1]), 0, 0)});
		}
		else if (levelData[i] == "kS5")
		{
			m_pColorChannels.at(1001).g = std::stof(levelData[i + 1]);
		}
		else if (levelData[i] == "kS6")
		{
			m_pColorChannels.at(1001).b = std::stof(levelData[i + 1]);
		}
		else if (levelData[i] == "kS29")
		{
			auto colorString = GameToolbox::splitByDelim(levelData[i + 1], '_');
			fillColorChannel(colorString, 1000);
		}
		else if (levelData[i] == "kS30")
		{
			auto colorString = GameToolbox::splitByDelim(levelData[i + 1], '_');
			fillColorChannel(colorString, 1001);
		}
		else if (levelData[i] == "kS31")
		{
			auto colorString = GameToolbox::splitByDelim(levelData[i + 1], '_');
			fillColorChannel(colorString, 1002);
		}
		else if (levelData[i] == "kS32")
		{
			auto colorString = GameToolbox::splitByDelim(levelData[i + 1], '_');
			fillColorChannel(colorString, 1004);
		}
		else if (levelData[i] == "kS37")
		{
			auto colorString = GameToolbox::splitByDelim(levelData[i + 1], '_');
			fillColorChannel(colorString, 1003);
		}
		else if (levelData[i] == "kS38")
		{
			auto colorString = GameToolbox::splitByDelim(levelData[i + 1], '|');
			for (std::string colorData : colorString)
			{
				auto innerData = GameToolbox::splitByDelim(colorData, '_');
				int key;
				Color3B col;
				for (size_t j = 0; j < innerData.size() - 1; j += 2)
				{
					switch (std::stoi(innerData[j]))
					{
					case 1:
						col.r = std::stof(innerData[j + 1]);
						break;
					case 2:
						col.g = std::stof(innerData[j + 1]);
						break;
					case 3:
						col.b = std::stof(innerData[j + 1]);
						break;
					case 6:
						key = std::stoi(innerData[j + 1]);
					}
				}
				m_pColorChannels.insert({key, col});
			}
		}
		else if (levelData[i] == "kA6")
		{
			_bgID = std::stoi(levelData[i + 1]);
			if (!_bgID)
				_bgID = 1;
		}
		else if (levelData[i] == "kA7")
		{
			_groundID = std::stoi(levelData[i + 1]);
			if (!_groundID)
				_groundID = 1;
		}
	}

	m_pColorChannels[1005] = m_pPlayer->getMainColor();
	m_pColorChannels[1006] = m_pPlayer->getSecondaryColor();
	m_pColorChannels[1010] = Color3B::BLACK;

	_originalColors = std::map<int, Color3B>(m_pColorChannels);
	
	for (std::string data : objData)
	{
		auto d = GameToolbox::splitByDelim(data, ',');

		GameObject* obj = nullptr;

		Hitbox hb = {0, 0, 0, 0};

		//GameToolbox::log("obj:{}", data);

		for (size_t i = 0; i < d.size() - 1; i += 2)
		{
			int key = std::stoi(d[i]);

			if (key != 1 && obj == nullptr) break;

			switch (key)
			{
			case 1:
			{
				int id = std::stoi(d[i + 1]);

				auto block = GameObject::_pBlocks.at(id);
				std::string frame = static_cast<std::string>(block[0]);
				std::string glowFrame = "";
				if (block.size() > 1) glowFrame = static_cast<std::string>(block[1]);

				if (std::find(std::begin(GameObject::_pTriggers), std::end(GameObject::_pTriggers), id) !=
					std::end(GameObject::_pTriggers))
					obj = EffectGameObject::create(frame + ".png");
				else
					obj = GameObject::create(frame + ".png", glowFrame + ".png");

				if (obj == nullptr) break;

				AX_SAFE_RETAIN(obj);

				obj->setStretchEnabled(false);
				obj->setActive(true);
				obj->setID(id);

				obj->setupColors();

				obj->customSetup();

				if (GameObject::_pHitboxes.contains(id)) hb = GameObject::_pHitboxes.at(id);

				obj->_uniqueID = _pObjects.size();

				_pObjects.push_back(obj);
			}
			break;
			case 2:
				obj->setPositionX(std::stof(d[i + 1]));
				break;
			case 3:
				obj->setPositionY(std::stof(d[i + 1]) + 90.0f);
				break;
			case 4:
				obj->setFlippedX(std::stoi(d[i + 1]));
				break;
			case 5:
				obj->setFlippedY(std::stoi(d[i + 1]));
				break;
			case 6:
				obj->setRotation(std::stof(d[i + 1]));
				break;
			case 7:
				dynamic_cast<EffectGameObject*>(obj)->m_cColor.r = std::stof(d[i + 1]);
				break;
			case 8:
				dynamic_cast<EffectGameObject*>(obj)->m_cColor.g = std::stof(d[i + 1]);
				break;
			case 9:
				dynamic_cast<EffectGameObject*>(obj)->m_cColor.b = std::stof(d[i + 1]);
				break;
			case 10:
				dynamic_cast<EffectGameObject*>(obj)->m_fDuration = std::stof(d[i + 1]);
				break;
			case 21:
				obj->_mainColorChannel = std::stoi(d[i + 1]);
				break;
			case 22:
				obj->_secColorChannel = std::stoi(d[i + 1]);
				break;
			case 23:
				dynamic_cast<EffectGameObject*>(obj)->m_nTargetColorId = std::stof(d[i + 1]);
				break;
			case 25:
				obj->setGlobalZOrder(std::stoi(d[i + 1]));
				break;
			case 67: // dont enter
			case 64: // dont exit
				obj->setDontTransform(true);
				break;
			}
		}
		if (obj)
		{
			obj->updateObjectType();

			switch (obj->getGameObjectType())
			{
			case kGameObjectTypeYellowJumpRing:
			case kGameObjectTypeYellowJumpPad:
			case kGameObjectTypeNormalGravityPortal:
			case kGameObjectTypeInverseGravityPortal:
			case kGameObjectTypeCubePortal:
			case kGameObjectTypeShipPortal:
			case kGameObjectTypeSolid:
			case kGameObjectTypeSpecial:
			case kGameObjectTypeHazard:
				ax::Mat4 tr;

				tr.rotate(obj->getRotationQuat());

				tr.scale(
					obj->getScaleX() * (obj->isFlippedX() ? -1.f : 1.f), obj->getScaleY() * (obj->isFlippedY() ? -1.f : 1.f),
					1);

				ax::Rect rec = {hb.x, hb.y, hb.w, hb.h};
				rec = RectApplyTransform(rec, tr);

				obj->setOuterBounds(Rect(
					obj->getPosition() + Vec2(rec.origin.x, rec.origin.y) + Vec2(15, 15),
					{rec.size.width, rec.size.height}));
				break;
			}
			obj->setStartPosition(obj->getPosition());
			obj->setStartScaleX(obj->getScaleX());
			obj->setStartScaleY(obj->getScaleY());
		}
	}
}

bool PlayLayer::init(GJGameLevel* level)
{
	if (!Layer::init()) return false;
	setLevel(level);

	Instance = this;

	auto winSize = Director::getInstance()->getWinSize();

	dn = ax::DrawNode::create();
	dn->setPosition({-15, -15});
	addChild(dn, 99999);

	this->m_pPlayer = PlayerObject::create(GameToolbox::randomInt(1, 12), this);
	this->m_pPlayer->setPosition({-20, 105});
	this->addChild(this->m_pPlayer, 2);
	this->m_pPlayer->setAnchorPoint({0, 0});

	m_pPlayer->setMainColor({125, 255, 0});
	m_pPlayer->setSecondaryColor({0, 255, 255});

	_blendingBatchNode = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 150);
	this->addChild(_blendingBatchNode);
	_blendingBatchNode->setBlendFunc(GameToolbox::getBlending());

	_glowBatchNode = ax::SpriteBatchNode::create(GameToolbox::getTextureString("GJ_GameSheetGlow.png"), 150);
	this->addChild(_glowBatchNode);
	_glowBatchNode->setBlendFunc(GameToolbox::getBlending());

	_mainBatchNode = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_mainBatchNodeTexture), 150);
	this->addChild(_mainBatchNode);

	_main2BatchNode = ax::SpriteBatchNode::create(GameToolbox::getTextureString(_main2BatchNodeTexture), 150);
	this->addChild(_main2BatchNode);

	_particleBatchNode = ax::ParticleBatchNode::create("square.png", 30);
	addChild(_particleBatchNode);

	_mainBatchNodeTexture = _mainBatchNode->getTexture()->getPath();
	_main2BatchNodeTexture = _main2BatchNode->getTexture()->getPath();

	//std::string levelStr = FileUtils::getInstance()->getStringFromFile("level.txt");
	std::string levelStr = getLevel()->_LevelString.empty() ? GJGameLevel::getLevelStrFromID(getLevel()->_LevelID) : getLevel()->_LevelString;
	loadLevel(levelStr);

	this->_bottomGround = GroundLayer::create(_groundID);
	this->_ceiling = GroundLayer::create(_groundID);
	cameraFollow = ax::Node::create();
	cameraFollow->addChild(this->_bottomGround, 1);
	cameraFollow->addChild(this->_ceiling, 1);
	this->addChild(cameraFollow, 1);

	this->_ceiling->setScaleY(-1);
	_ceiling->setVisible(false);
	_bottomGround->setPositionY(-cameraFollow->getPositionY() + 12);
	_ceiling->setPositionY(winSize.height + _ceiling->_sprite->getTextureRect().size.height);

	this->m_pBG = Sprite::create(GameToolbox::getTextureString(fmt::format("game_bg_{:02}_001.png", _bgID)));
	m_pBG->setStretchEnabled(false);
	const Texture2D::TexParams texParams = {
		backend::SamplerFilter::LINEAR, backend::SamplerFilter::LINEAR, backend::SamplerAddressMode::REPEAT,
		backend::SamplerAddressMode::REPEAT };
	this->m_pBG->getTexture()->setTexParameters(texParams);
	this->m_pBG->setTextureRect(Rect(0, 0, 1024 * 5, 1024));
	this->m_pBG->setPosition(winSize.x / 2, winSize.y / 4);
	this->addChild(this->m_pBG, -100);

	if (this->m_pColorChannels.contains(1000)) this->m_pBG->setColor(this->m_pColorChannels.at(1000));
	this->_bottomGround->update(0);

	if (_pObjects.size() != 0)
	{
		this->m_lastObjXPos = std::numeric_limits<float>().min();

		for (GameObject* object : _pObjects)
			if (this->m_lastObjXPos < object->getPositionX()) this->m_lastObjXPos = object->getPositionX();

		if (this->m_lastObjXPos < 570.f) this->m_lastObjXPos = 570.f;

		for (size_t i = 0; i < sectionForPos(this->m_lastObjXPos); i++)
		{
			std::vector<GameObject*> vec;
			m_pSectionObjects.push_back(vec);
		}

		for (GameObject* object : _pObjects)
		{
			int section = sectionForPos(object->getPositionX());
			m_pSectionObjects[section - 1 < 0 ? 0 : section - 1].push_back(object);

			object->setCascadeOpacityEnabled(true);
			object->update();
		}
	}

	this->m_pBar = SimpleProgressBar::create();
	this->m_pBar->setPercentage(0.f);
	this->m_pBar->setPosition({winSize.width / 2, winSize.height - 20});
	this->addChild(m_pBar, 1000);

	m_pBar->setPosition(this->m_obCamPos + winSize / 2);
	m_pBar->setPositionY((this->m_obCamPos + winSize).height - 10);

	bool levelValid = LevelTools::verifyLevelIntegrity(this->getLevel()->_LevelString, this->getLevel()->_LevelID);

	if(!levelValid) {
		auto loadfailedstr = Label::createWithBMFont(GameToolbox::getTextureString("bigFont.fnt"), "Load Failed!");
		loadfailedstr->setPosition({
			winSize.width / 2,
			winSize.height / 2
		});
		addChild(loadfailedstr, 128);
	}
	updateVisibility();

	scheduleOnce(
		[=](float d) {
			if(levelValid)
			{
				AudioEngine::play2d(LevelTools::getAudioFilename(getLevel()->_MusicID), false, 0.1f);
				scheduleUpdate();
				m_pPlayer->setIsDead(false);
			} else 
			{
				exit();
			}
		},
		1.f, "k");

	return true;
}

double lastY = 0;

void PlayLayer::update(float dt)
{
	if (_pauseUpdate)
		return;

	if (m_freezePlayer)
	{
		AudioEngine::pauseAll();
		return;
	}
	else
	{
		AudioEngine::resumeAll();
	}

	float step = std::min(2.0f, dt * 60.0f);

	m_pPlayer->m_bIsPlatformer = m_platformerMode;
	m_pPlayer->noclip = noclip;

	auto winSize = Director::getInstance()->getWinSize();

	this->m_pColorChannels.at(1005) = this->m_pPlayer->getMainColor();
	this->m_pColorChannels.at(1006) = this->m_pPlayer->getSecondaryColor();

	if (this->m_pColorChannels.contains(1000)) this->m_pBG->setColor(this->m_pColorChannels.at(1000));

	if (!m_freezePlayer && !this->m_pPlayer->isDead())
	{
		step /= 4.0f;
		lastY = m_pPlayer->getYVel();
		for (int i = 0; i < 4; i++)
		{
			this->m_pPlayer->update(step);

			m_pPlayer->setOuterBounds(Rect(m_pPlayer->getPosition(), {30, 30}));
			m_pPlayer->setInnerBounds(Rect(m_pPlayer->getPosition() + Vec2(11.25f, 11.25f), {7.5, 7.5}));

			this->checkCollisions(step);
			if (this->m_pPlayer->isDead()) break;
		}
		step *= 4.0f;
	}

	m_pBar->setPercentage(m_pPlayer->getPositionX() / this->m_lastObjXPos * 100.f);

	this->updateVisibility();
	this->updateCamera(step);
	if (m_pPlayer->isShip()) m_pPlayer->updateShipRotation(step);

	m_pColorChannels[1005] = m_pPlayer->getMainColor();
	m_pColorChannels[1006] = m_pPlayer->getSecondaryColor();
}

void PlayLayer::destroyPlayer()
{
	if (m_pPlayer->isDead()) return;
	if (m_pPlayer->noclip) return;
	m_pPlayer->setIsDead(true);
	m_pPlayer->playDeathEffect();
	m_pPlayer->stopRotation();

	scheduleOnce([=](float d) { resetLevel(); }, 1.f, "restart");
}

void PlayLayer::updateCamera(float dt)
{
	auto winSize = Director::getInstance()->getWinSize();
	Vec2 cam = m_obCamPos;

	PlayerObject* player = m_pPlayer;
	Vec2 pPos = player->getPosition();

	if (player->isShip())
	{
		cam.y = (winSize.height * -0.5f) + m_fCameraYCenter;
		if (cam.y <= 0.0f) cam.y = 0.0f;
	}
	else
	{
		float unk2 = 90.0f;
		float unk3 = 120.0f;
		if (player->isGravityFlipped())
		{
			unk2 = 120.0f;
			unk3 = 90.0f;
		}
		if (pPos.y <= winSize.height + cam.y - unk2)
		{
			if (pPos.y < unk3 + cam.y) cam.y = pPos.y - unk3;
		}
		else
			cam.y = pPos.y - winSize.height + unk2;
		if (!player->isGravityFlipped())
		{
			Vec2 lastGroundPos = player->getLastGroundPos();

			if (lastGroundPos.y == 105.f)
				if (pPos.y <= cam.y + winSize.height - unk2) cam.y = 0.0f;
		}
	}

	cam.y = clampf(cam.y, 0.0f, 1140.f - winSize.height);

	if (pPos.x >= winSize.width / 2.5f && !player->isDead() && !player->m_bIsPlatformer) // wrong but works for now
	{
		this->m_pBG->setPositionX(
			this->m_pBG->getPositionX() - dt * player->getPlayerSpeed() * _bottomGround->getSpeed() * 0.1175f);
		_bottomGround->update(dt * player->getPlayerSpeed());
		_ceiling->update(dt * player->getPlayerSpeed());
		cam.x += dt * player->getPlayerSpeed() * 5.770002f;
	}
	else if (player->m_bIsPlatformer)
		cam.x = pPos.x - winSize.width / 2.f;

	if (this->m_pBG->getPosition().x <= cam.x - 1024.f) this->m_pBG->setPositionX(this->m_pBG->getPositionX() + 1024.f);

	this->m_pBG->setPositionX(this->m_pBG->getPositionX() + (cam.x - m_obCamPos.x));

	if (!this->m_bMoveCameraX) m_obCamPos.x = cam.x;

	if (!this->m_bMoveCameraY) m_obCamPos.y = GameToolbox::iLerp(m_obCamPos.y, cam.y, 0.1f, dt / 60.f);

	Camera::getDefaultCamera()->setPosition(this->m_obCamPos + winSize / 2);

	cameraFollow->setPosition(m_obCamPos);
	_ceiling->setVisible(m_pPlayer->isShip());
	if (!m_pPlayer->isShip()) _bottomGround->setPositionY(-cameraFollow->getPositionY() + 12);

	m_pBar->setPosition(this->m_obCamPos + winSize / 2);
	m_pBar->setPositionY((this->m_obCamPos + winSize).height - 10);
}

float PlayLayer::getRelativeMod(Vec2 pos, float v1, float v2, float v3)
{
	auto winSize = ax::Director::getInstance()->getWinSize();
	float camX = m_obCamPos.x;
	float centerX = winSize.width / 2.f;
	float camXCenter = camX + centerX;
	float posX = pos.x;

	float vv1;
	float vv2;
	float vv3;
	float result;

	if (posX <= camXCenter)
	{
		vv2 = v2;
		vv3 = (camXCenter - posX) - v3;
	}
	else
	{
		vv1 = ((posX - v3) - camX) - centerX;
		vv2 = v1;
		vv3 = vv1;
	}
	if (vv2 < 1.f)
		vv2 = 1.f;

	result = (centerX - vv3) / vv2;

	return result;
}
void PlayLayer::applyEnterEffect(GameObject* obj)
{
	if (obj->getEnterEffectID() != _enterEffectID)
		obj->setEnterEffectID(_enterEffectID);
	Vec2 objStartPos = obj->getStartPosition();
	Vec2 objStartScale = obj->getStartScale();
	float rModn = getRelativeMod(objStartPos, 60.f, 60.f, 0.f);
	float rMod = clampf(rModn, 0.f, 1.f);

	switch (obj->getEnterEffectID())
	{
	case 2:
		if (obj->getGameObjectType() != GameObjectType::kGameObjectTypeYellowJumpPad)
		{
			obj->setScaleX(rMod * objStartScale.x);
			obj->setScaleY(rMod * objStartScale.y);
		}
		break;
	case 3:
		if (obj->getGameObjectType() != GameObjectType::kGameObjectTypeYellowJumpPad)
		{
			obj->setScaleX((2.f - rMod) * objStartScale.x);
			obj->setScaleY((2.f - rMod) * objStartScale.y);
		}
		break;
	case 4:
		if (obj->getGameObjectType() != GameObjectType::kGameObjectTypeYellowJumpPad)
			obj->setPositionY((1.0 - rMod) * 100.f + objStartPos.y);
		break;
	case 5:
		if (obj->getGameObjectType() != GameObjectType::kGameObjectTypeYellowJumpPad)
			obj->setPositionY((1.0 - rMod) * -100.f + objStartPos.y);
		break;
	case 6:
		if (obj->getGameObjectType() != GameObjectType::kGameObjectTypeYellowJumpPad)
			obj->setPositionX((1.0 - rMod) * -100.f + objStartPos.x);
		break;
	case 7:
		if (obj->getGameObjectType() != GameObjectType::kGameObjectTypeYellowJumpPad)
			obj->setPositionX((1.0 - rMod) * 100.f + objStartPos.x);
		break;
	default:
		obj->setPosition(objStartPos);
		break;
	}
	obj->setEnterEffectID(0);
}
void PlayLayer::updateVisibility()
{
	auto winSize = ax::Director::getInstance()->getWinSize();

	float unk = 70.0f;

	int prevSection = floorf(this->m_obCamPos.x / 100) - 1.0f;
	int nextSection = ceilf((this->m_obCamPos.x + winSize.width) / 100) + 1.0f;

	for (int i = prevSection; i < nextSection; i++)
	{
		if (i >= 0)
		{
			if (i < m_pSectionObjects.size())
			{
				auto section = m_pSectionObjects[i];
				for (size_t j = 0; j < section.size(); j++)
				{
					GameObject* obj = section[j];
					if (!obj) continue;

					if (obj->getParent() == nullptr)
					{
						if (obj->_particle)
						{
							_particleBatchNode->addChild(obj->_particle);
							AX_SAFE_RELEASE(obj->_particle);
						}
						if (obj->_glowSprite)
						{
							_glowBatchNode->addChild(obj->_glowSprite);
							AX_SAFE_RELEASE(obj->_glowSprite);
						}

						if (obj->getBlendFunc() != GameToolbox::getBlending())
						{
							if(obj->_texturePath == _mainBatchNodeTexture)
								_mainBatchNode->addChild(obj);
							else if (obj->_texturePath == _main2BatchNodeTexture)
								_main2BatchNode->addChild(obj);
						}
						else
							_blendingBatchNode->addChild(obj);
						AX_SAFE_RELEASE(obj);
					}

					obj->setActive(true);
					obj->update();

					// if (obj->getType() == GameObjectType::kBallFrame || obj->getType() ==
					// GameObjectType::kYellowJumpRing)
					//	 obj->setScale(this->getAudioEffectsLayer()->getAudioScale())

					// auto pos = obj->getPosition();

					float unk2 = 0.0f;
					if (obj->getGameObjectType() == GameObjectType::kGameObjectTypeDecoration)
						unk2 = obj->getTextureRect().origin.x * obj->getScaleX() * 0.4f;

					unsigned char opacity = clampf(getRelativeMod(obj->getPosition(), 70.f, 70.f, unk2), 0.f, 1.f) * 255.0f;
					if (!obj->getDontTransform())
					{
						obj->setOpacity(opacity);
						this->applyEnterEffect(obj);
					}
				}
			}
		}
	}

	if (_prevSection - 1 >= 0 && m_pSectionObjects.size() != 0 && _prevSection <= m_pSectionObjects.size())
	{
		auto section = m_pSectionObjects[_prevSection - 1];
		for (size_t j = 0; j < section.size(); j++)
		{
			section[j]->setActive(false);
			if (section[j]->getParent() != nullptr)
			{
				AX_SAFE_RETAIN(section[j]);
				if(section[j]->_particle)
				{
					AX_SAFE_RETAIN(section[j]->_particle);
					_particleBatchNode->removeChild(section[j]->_particle, true);
				}
				if (section[j]->_glowSprite)
				{
					AX_SAFE_RETAIN(section[j]->_glowSprite);
					_glowBatchNode->removeChild(section[j]->_glowSprite, true);
				}
				//_mainBatchNode->removeChild(section[j], true);
				if (section[j]->getBlendFunc() != GameToolbox::getBlending())
				{
					if (section[j]->_texturePath == _mainBatchNodeTexture)
						_mainBatchNode->removeChild(section[j], true);
					else if (section[j]->_texturePath == _main2BatchNodeTexture)
						_main2BatchNode->removeChild(section[j], true);
				}
				else
					_blendingBatchNode->removeChild(section[j], true);
			}
		}
	}

	this->_prevSection = prevSection;
	this->_nextSection = nextSection;
}

void PlayLayer::changeGameMode(GameObject* obj, int gameMode)
{
	switch (gameMode)
	{
	case 0:
	{
		this->m_pPlayer->setIsShip(false);
	}
	break;
	case 1:
	{
		if (obj->getPositionY() < 270)
		{
			m_fCameraYCenter = 240.0f;
		}
		else
		{
			m_fCameraYCenter = (floorf(obj->getPositionY() / 30.0f) * 30.0f);
		}
		this->m_pPlayer->setIsShip(true);
		this->m_pPlayer->setRotation(0.f);
		tweenBottomGround(-68);
		tweenCeiling(388);
	}
	}
}

void PlayLayer::moveCameraToPos(Vec2 pos)
{
	auto moveX = [this](float a, float b, float c) -> void {
		this->stopActionByTag(0);
		auto tweenAction = ActionTween::create(b, "cTX", m_obCamPos.x, a);
		auto easeAction = EaseInOut::create(tweenAction, c);
		easeAction->setTag(0);
		this->runAction(easeAction);
	};
	auto moveY = [this](float a, float b, float c) -> void {
		this->stopActionByTag(1);
		auto tweenAction = ActionTween::create(b, "cTY", m_obCamPos.y, a);
		auto easeAction = EaseInOut::create(tweenAction, c);
		easeAction->setTag(1);
		this->runAction(easeAction);
	};
	moveX(pos.x, 1.2f, 1.8f);
	moveY(pos.y, 1.2f, 1.8f);
}

void PlayLayer::processTriggers()
{
	// int current_section = this->sectionForPos(m_pPlayer->getPositionX());
	// if (m_pSectionObjects.size() == 0) return;

	// std::vector<GameObject*> section =
	// 	m_pSectionObjects[sectionForPos(m_pPlayer->getPositionX()) <= 0 ? 0 : sectionForPos(m_pPlayer->getPositionX()) - 1];

	// int i = 0;
	// while (i < section.size())
	// {
	// 	i++;
	// }
}

void PlayLayer::checkCollisions(float dt)
{
	auto playerOuterBounds = this->m_pPlayer->getOuterBounds();
	if (this->m_pPlayer->getPositionY() < 105.0f && !this->m_pPlayer->isShip())
	{
		if (this->m_pPlayer->isGravityFlipped())
		{
			this->destroyPlayer();
			return;
		}

		this->m_pPlayer->setPositionY(105.0f);

		this->m_pPlayer->hitGround(false);
	}

	else if (this->m_pPlayer->getPositionY() > 1290.0f)
	{
		this->destroyPlayer();
		return;
	}

	if (this->m_pPlayer->isShip())
	{
		if (this->m_pPlayer->getPositionY() < _bottomGround->getPositionY() + cameraFollow->getPositionY() + 93.0f)
		{
			this->m_pPlayer->setPositionY(_bottomGround->getPositionY() + cameraFollow->getPositionY() + 93.0f);

			if(!this->m_pPlayer->isGravityFlipped())
				this->m_pPlayer->hitGround(false);

			m_pPlayer->setYVel(0.f);
		}
		if (m_pPlayer->getPositionY() > _ceiling->getPositionY() - 240.f + m_fCameraYCenter - 12.f)
		{
			this->m_pPlayer->setPositionY(_ceiling->getPositionY() - 240.f + m_fCameraYCenter - 12.f);

			if (m_pPlayer->isGravityFlipped()) 
				this->m_pPlayer->hitGround(true);

			m_pPlayer->setYVel(0.f);
		}
	}

	dn->setVisible(showDn);

	if (showDn)
	{
		dn->clear();
		renderRect(playerOuterBounds, ax::Color4B::RED);
		renderRect(m_pPlayer->getInnerBounds(), ax::Color4B::GREEN);
	}

	int current_section = this->sectionForPos(m_pPlayer->getPositionX());

	std::deque<GameObject*> m_pHazards;

	for (int i = current_section - 2; i < current_section + 1; i++)
	{
		if (i < m_pSectionObjects.size() && i >= 0)
		{
			std::vector<GameObject*> section = m_pSectionObjects[i];

			for (int j = 0; j < section.size(); j++)
			{
				GameObject* obj = section[j];

				auto objBounds = obj->getOuterBounds();

				if (obj->getGameObjectType() == kGameObjectTypeHazard)
				{
					if (objBounds.size.width <= 0 || objBounds.size.height <= 0) continue;
					m_pHazards.push_back(obj);
					if (showDn) renderRect(objBounds, ax::Color4B::RED);
				}
				else if (obj->isActive())
				{
					if (!obj->m_bHasBeenActivated)
					{
						if (auto trigger = dynamic_cast<EffectGameObject*>(obj); trigger)
						{
							if (trigger->getPositionX() <= m_pPlayer->getPositionX()) trigger->triggerActivated(dt);
						}
					}
					if (objBounds.size.width <= 0 || objBounds.size.height <= 0) continue;
					if (showDn) renderRect(objBounds, ax::Color4B::BLUE);
					if (playerOuterBounds.intersectsRect(objBounds))
					{
						// GameToolbox::log("game object type 2: {}", obj->getGameObjectType());
						switch (obj->getGameObjectType())
						{
						case GameObjectType::kGameObjectTypeInverseGravityPortal:
							// if (!m_pPlayer->isGravityFlipped())
							//	 this->playGravityEffect(true);

							m_pPlayer->setPortalP(obj->getPosition());
							m_pPlayer->setPortalObject(obj);

							m_pPlayer->flipGravity(true);
							break;

						case GameObjectType::kGameObjectTypeNormalGravityPortal:
							// if (m_pPlayer->isGravityFlipped())
							//	 this->playGravityEffect(false);

							m_pPlayer->setPortalP(obj->getPosition());
							m_pPlayer->setPortalObject(obj);

							m_pPlayer->flipGravity(false);
							break;

						case GameObjectType::kGameObjectTypeShipPortal:
							m_pPlayer->setPortalP(obj->getPosition());
							m_pPlayer->setPortalObject(obj);

							this->changeGameMode(obj, 1);
							break;

						case GameObjectType::kGameObjectTypeCubePortal:
							m_pPlayer->setPortalP(obj->getPosition());
							m_pPlayer->setPortalObject(obj);

							this->changeGameMode(obj, 0);
							break;

						case GameObjectType::kGameObjectTypeYellowJumpPad:
							if (!obj->m_bHasBeenActivated)
							{
								m_pPlayer->setPortalP(obj->getPosition());
								m_pPlayer->setPortalObject(obj);

								obj->triggerActivated();
								m_pPlayer->propellPlayer();
							}
							break;

						case GameObjectType::kGameObjectTypeYellowJumpRing:
							if (!obj->m_bHasBeenActivated)
							{
								m_pPlayer->setPortalP(obj->getPosition());
								m_pPlayer->setPortalObject(obj);

								m_pPlayer->setTouchedRing(obj);

								m_pPlayer->ringJump();
							}
							break;
						case GameObjectType::kGameObjectTypeSpecial:
							break;
						default:
							m_pPlayer->collidedWithObject(dt, obj);
							break;
						}
					}
				}
			}
		}
	}
	for (unsigned int i = 0; i < m_pHazards.size(); ++i)
	{
		GameObject* hazard = m_pHazards[i];

		if (playerOuterBounds.intersectsRect(hazard->getOuterBounds()))
		{
			destroyPlayer();
			return;
		}
	}
	m_pHazards.clear();
}

void PlayLayer::onDrawImGui()
{
	extern bool _showDebugImgui;
	if (!_showDebugImgui) return;
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	ImGui::SetNextWindowPos({1000.0f, 200.0f}, ImGuiCond_FirstUseEver);

	ImGui::Begin("PlayLayer Debug");

	ImGui::Checkbox("Freeze Player", &m_freezePlayer);
	ImGui::Checkbox("Platformer Mode (Basic)", &m_platformerMode);

	if (ImGui::Button("Exit"))
	{
		this->exit();
	}

	ImGui::Text(
		"Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

	ImGui::Text("yVel %.3f", m_pPlayer->getYVel());

	ImGui::Checkbox("Show Hitboxes", &showDn);
	ImGui::Checkbox("Gain the power of invincibility", &noclip);

	if (ImGui::InputFloat("Speed", &gameSpeed)) Director::getInstance()->getScheduler()->setTimeScale(gameSpeed);

	if (ImGui::InputFloat("FPS", &fps)) Director::getInstance()->setAnimationInterval(1.0f / fps);

	ImGui::Text("Sections: %i", m_pSectionObjects.size());
	if (m_pSectionObjects.size() > 0 && sectionForPos(m_pPlayer->getPositionX()) - 1 < m_pSectionObjects.size())
		ImGui::Text(
			"Current Section Size: %i",
			m_pSectionObjects
				[sectionForPos(m_pPlayer->getPositionX()) <= 0 ? 0 : sectionForPos(m_pPlayer->getPositionX()) - 1]
					.size());

	if (ImGui::Button("Reset"))
	{
		this->resetLevel();
	}
	ImGui::End();
}

void PlayLayer::resetLevel()
{
	m_pPlayer->setPosition({ 2, 105 });
	m_pPlayer->setRotation(0);
	m_obCamPos.x = 0;
	m_obCamPos.y = 0;
	_bottomGround->setPositionX(0);
	_ceiling->setPositionX(0);
	m_pPlayer->reset();
	m_pBG->setPositionX(0);
	_enterEffectID = 0;

	for (auto obj : this->_pObjects)
	{
		if (!obj) continue;
		obj->m_bHasBeenActivated = false;
		obj->setActive(false);
		if (obj->getParent() != nullptr)
		{
			if(obj->_particle)
			{
				AX_SAFE_RETAIN(obj->_particle);
				_particleBatchNode->removeChild(obj->_particle, true);
			}
			if (obj->_glowSprite)
			{
				AX_SAFE_RETAIN(obj->_glowSprite);
				_glowBatchNode->removeChild(obj->_glowSprite, true);
			}
			AX_SAFE_RETAIN(obj);
			//_mainBatchNode->removeChild(obj, true);
			if (obj->getBlendFunc() != GameToolbox::getBlending())
			{
				if (obj->_texturePath == _mainBatchNodeTexture)
					_mainBatchNode->removeChild(obj, true);
				else if (obj->_texturePath == _main2BatchNodeTexture)
					_main2BatchNode->removeChild(obj, true);
			}
			else
				_blendingBatchNode->removeChild(obj, true);
		}
	}

	ax::Director::getInstance()->getActionManager()->removeAllActions();

	m_pColorChannels = std::map<int, Color3B>(_originalColors);

	_prevSection = -1;
	_nextSection = -1;

	if (this->m_pColorChannels.contains(1000)) this->m_pBG->setColor(this->m_pColorChannels.at(1000));
	this->_bottomGround->update(0);
	this->_ceiling->update(0);

	AudioEngine::stopAll();
	AudioEngine::play2d(LevelTools::getAudioFilename(getLevel()->_MusicID), false, 0.1f);
	scheduleUpdate();
}

void PlayLayer::renderRect(ax::Rect rect, ax::Color4B col)
{
	dn->drawRect({rect.getMinX(), rect.getMinY()}, {rect.getMaxX(), rect.getMaxY()}, col);
	dn->drawSolidRect({rect.getMinX(), rect.getMinY()}, {rect.getMaxX(), rect.getMaxY()}, Color4B(col.r, col.g, col.b, 100));
}

void PlayLayer::onEnter()
{
	Layer::onEnter();

	auto listener = EventListenerKeyboard::create();
	auto dir = Director::getInstance();

	listener->onKeyPressed = AX_CALLBACK_2(PlayLayer::onKeyPressed, this);
	listener->onKeyReleased = AX_CALLBACK_2(PlayLayer::onKeyReleased, this);
	dir->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);

	auto current = dir->getRunningScene();
#if SHOW_IMGUI == true
	ImGuiPresenter::getInstance()->addRenderLoop("#playlayer", AX_CALLBACK_0(PlayLayer::onDrawImGui, this), current);
#endif
}

void PlayLayer::onExit()
{
#if SHOW_IMGUI == true
	Director::getInstance()->getEventDispatcher()->removeEventListenersForTarget(this);
	ImGuiPresenter::getInstance()->removeRenderLoop("#playlayer");
#endif
	LevelPage::replacingScene = false;
	Instance = nullptr;
	Layer::onExit();
}
void PlayLayer::exit()
{
	m_pPlayer->deactivateStreak();
	//removeChild(m_pPlayer->motionStreak);

	_pauseUpdate = true;
	unscheduleUpdate();
	m_pPlayer->unscheduleUpdate();
	//_mainBatchNode->removeAllChildrenWithCleanup(true);

	int size = _pObjects.size();
	for (int i = 0; i < size; i++)
	{
		GameObject* obj = _pObjects.at(i);
		if(obj && !obj->getParent()) AX_SAFE_RELEASE(obj);
	}

	_particleBatchNode->removeAllChildrenWithCleanup(true);
	_glowBatchNode->removeAllChildrenWithCleanup(true);
	
	//for (auto particle : _particleBatchNode->getChildren())
	//{
	//	particle->autorelease();
	//}
	AudioEngine::stopAll();
	AudioEngine::play2d("quitSound_01.ogg", false, 0.1f);
	AudioEngine::play2d("menuLoop.mp3", true, 0.2f);
	// MenuLayer::music = false;
	Director::getInstance()->replaceScene(TransitionFade::create(0.5f, LevelSelectLayer::scene()));

}
void PlayLayer::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	GameToolbox::log("Key with keycode {} pressed", static_cast<int>(keyCode));
	switch (keyCode)
		{
		case EventKeyboard::KeyCode::KEY_R:
		{
			resetLevel();
		}
		break;
		case EventKeyboard::KeyCode::KEY_F:
		{
			extern bool _showDebugImgui;
			_showDebugImgui = !_showDebugImgui;
		}
		break;
		case EventKeyboard::KeyCode::KEY_SPACE:
		{
			m_pPlayer->onTouchBegan(nullptr, nullptr);
		}
		break;
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		{
			m_pPlayer->onTouchBegan(nullptr, nullptr);
		}
		break;
		case EventKeyboard::KeyCode::KEY_BACK:
		{
			m_pPlayer->onTouchEnded(nullptr, nullptr);
			this->exit();
		}
	}
	if (keyCode == EventKeyboard::KeyCode::KEY_A && m_pPlayer->m_bIsPlatformer)
		m_pPlayer->direction = -1.f;
	else if (keyCode == EventKeyboard::KeyCode::KEY_D && m_pPlayer->m_bIsPlatformer)
		m_pPlayer->direction = 1.f;
}

void PlayLayer::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	GameToolbox::log("Key with keycode {} released", static_cast<int>(keyCode));
	if ((keyCode == EventKeyboard::KeyCode::KEY_A && m_pPlayer->direction == -1.f) ||
		(keyCode == EventKeyboard::KeyCode::KEY_D && m_pPlayer->direction == 1.f) && m_pPlayer->m_bIsPlatformer)
		m_pPlayer->direction = 0.f;
	switch (keyCode)
	{
	case EventKeyboard::KeyCode::KEY_SPACE:
	{
		m_pPlayer->onTouchEnded(nullptr, nullptr);
	}
	break;
	case EventKeyboard::KeyCode::KEY_UP_ARROW:
	{
		m_pPlayer->onTouchEnded(nullptr, nullptr);
	}
	}
}

void PlayLayer::tweenBottomGround(float y)
{
	_bottomGround->runAction(EaseInOut::create(ActionTween::create(0.1f, "y", _bottomGround->getPositionY(), y), 2.f));
	//_bottomGround->setPositionY(y);
}

void PlayLayer::tweenCeiling(float y)
{
	_ceiling->runAction(EaseInOut::create(ActionTween::create(0.1f, "y", _ceiling->getPositionY(), y), 2.f));
	//_ceiling->setPositionY(y);
}

PlayLayer* PlayLayer::getInstance() { return Instance; }