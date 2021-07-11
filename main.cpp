// generated through GDMake https://github.com/HJfod/GDMake

// include GDMake & submodules
#include <GDMake.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>

using namespace gd;
using namespace gdmake;
using namespace gdmake::extra;
using namespace cocos2d;
using namespace cocos2d::extension;

static constexpr const int VOLUME_MUSIC_TAG = 345893;
static constexpr const int VOLUME_SFX_TAG = 345894;

int (__stdcall * ChannelControl_setVolume)(void *, float);

#define HIDE_SPRITE(spr, opacity)           \
    spr->setOpacity(opacity);               \
    spr->stopAllActions();                  \
    spr->runAction(CCSequence::create(      \
        CCDelayTime::create(1.0f),          \
        CCFadeTo::create(1.0f, 0),          \
        nullptr                             \
    ));                                     \

class VolumeControlOverlay : public CCNode {
    protected:
        CCScale9Sprite* m_pBG;
        Slider* m_pSlider;
        int m_nTag;

        bool init(const int tag) {
            if (!CCNode::init())
                return false;

            auto winSize = CCDirector::sharedDirector()->getWinSize();

            m_pBG = cocos2d::extension::CCScale9Sprite::create(
                "square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
            );

            m_pBG->setScale(.5f);
            m_pBG->setColor({ 0, 0, 0 });
            m_pBG->setContentSize({ 360.0f, 60.0f });
            m_pBG->setOpacity(0);

            this->addChild(m_pBG);

            m_pSlider = Slider::create(this, nullptr, .8f);
            m_pSlider->m_pTouchLogic->m_pThumb->setVisible(false);
            m_pSlider->setValue(
                tag == VOLUME_MUSIC_TAG ?
                FMODAudioEngine::sharedEngine()->getMusicVolume() :
                FMODAudioEngine::sharedEngine()->getSFXVolume()
            );
            m_pSlider->m_pSliderBar->setOpacity(0);
            m_pSlider->m_pGroove->setOpacity(0);

            this->addChild(m_pSlider);

            this->setPosition({
                winSize.width - 120.0f,
                40.0f + (tag == VOLUME_SFX_TAG ? 40.0f : 0.0f)
            });
            this->setTag(tag);

            m_nTag = tag;

            return true;
        }

    public:
        static VolumeControlOverlay* create(const int tag) {
            auto pRet = new VolumeControlOverlay();

            if (pRet && pRet->init(tag)) {
                pRet->autorelease();
                return pRet;
            }

            CC_SAFE_DELETE(pRet);
            return nullptr;
        }

        void show() {
            HIDE_SPRITE(m_pSlider->m_pSliderBar, 255)
            HIDE_SPRITE(m_pSlider->m_pGroove, 255)
            HIDE_SPRITE(m_pBG, 185)
        }

        void updateValue(float offset) {
            auto fe = FMODAudioEngine::sharedEngine();

            auto ovol = 
                m_nTag == VOLUME_MUSIC_TAG ?
                fe->getMusicVolume() :
                fe->getSFXVolume();
            auto vol = ovol + offset / 240.f;

            if (vol < 0.0f) vol = 0.0f;
            if (vol > 1.0f) vol = 1.0f;
            m_pSlider->setValue(vol);
            m_pSlider->updateBar();

            if (m_nTag == VOLUME_MUSIC_TAG) {
                fe->m_fBackgroundMusicVolume = vol;
                ChannelControl_setVolume(fe->m_pGlobalChannel, vol);

                if (!GameManager::sharedState()->getEditorLayer() && !GameManager::sharedState()->getPlayLayer())
                    if (ovol == 0.0f && vol > 0.0f)
                        fe->playBackgroundMusic("menuLoop.mp3", true, false);
            } else {
                fe->m_fEffectsVolume = vol;
                ChannelControl_setVolume(fe->m_pCurrentSoundChannel, vol);
            }
        }
};

bool (__thiscall* dispatchScrollMSG)(CCMouseDelegate*, float, float);
bool __fastcall dispatchScrollMSGHook(CCMouseDelegate* self, edx_t, float deltaY, float param_2) {
    auto kb = CCDirector::sharedDirector()->getKeyboardDispatcher();

    if (kb->getAltKeyPressed()) {

        auto scene = CCDirector::sharedDirector()->getRunningScene();

        auto tag = VOLUME_MUSIC_TAG;
        if (kb->getShiftKeyPressed())
            tag = VOLUME_SFX_TAG;

        auto overlay = as<VolumeControlOverlay*>(scene->getChildByTag(tag));
        if (!overlay) {
            overlay = VolumeControlOverlay::create(tag);
            scene->addChild(overlay, 9999);
        }

        overlay->updateValue(-deltaY);
        overlay->show();

        return true;

    } else
        return dispatchScrollMSG(self, deltaY, param_2);
}

GDMAKE_MAIN {
    // main entrypoint for your mod.
    // this is where you do things like
    // create hooks, load settings, etc.

    // you don't have to enable hooks, as
    // they are automatically enabled after
    // this method.

    // return true if load was succesful,
    // false if an error occurred.
    
    ChannelControl_setVolume = reinterpret_cast<decltype(ChannelControl_setVolume)>(
        GetProcAddress(GetModuleHandleA("fmod.dll"), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z")
    );

    MH_CreateHook(
        GetProcAddress(GetModuleHandleA("libcocos2d.dll"), "?dispatchScrollMSG@CCMouseDispatcher@cocos2d@@QAE_NMM@Z"),
        dispatchScrollMSGHook,
        reinterpret_cast<LPVOID*>(&dispatchScrollMSG)
    );
    
    return true;
}

GDMAKE_UNLOAD {
    // if you need to do some extra cleanup
    // for your mod, write it here.
    
    // all default submodules are automatically
    // dealt with afterwards.
}
