#pragma once

#include <axmol.h>

#include <core/ui/CocosGUI.h>

#include "SimplePlayer.h"

class GarageLayer : public ax::Layer {
private:
    ax::Sprite* _pSelectionFrame;
    ax::Sprite* _pSelectionFrame2;
    ax::Sprite* _pSelectionFrame3;

    int _pSelectedCube;
    int _pSelectedMC;
    int _pSelectedSC;

    SimplePlayer* _pPreviewPlayer;
    ax::ui::TextField* _pUsernamefield;
public:
    static ax::Scene* scene();
    bool init();
    CREATE_FUNC(GarageLayer);
};