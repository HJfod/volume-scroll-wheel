#include <Geode/binding/Slider.hpp>
#include <Geode/modify/CCMouseDispatcher.hpp>

using namespace geode::prelude;

class VolumeControlOverlay : public CCNode {
protected:
	CCScale9Sprite* m_bg;
	CCLabelBMFont* m_musicTitle;
	Slider* m_musicSlider;
	CCLabelBMFont* m_musicPercent;
	CCLabelBMFont* m_sfxTitle;
	Slider* m_sfxSlider;
	CCLabelBMFont* m_sfxPercent;
	CCLabelBMFont* m_sfxShiftInfo;

	bool init() {
		if (!CCNode::init())
			return false;
		
		this->setAnchorPoint({ 1, 0 });
		this->setContentSize({ 400, 75 });
		this->setScale(.5f);

		m_bg = CCScale9Sprite::create(
			"square02b_001.png", { 0.0f, 0.0f, 80.0f, 80.0f }
		);
		m_bg->setContentSize(m_obContentSize);
		m_bg->setColor({ 0, 0, 0 });
		m_bg->setOpacity(205);
		this->addChildAtPosition(m_bg, Anchor::Center);

		m_musicTitle = CCLabelBMFont::create("Music", "goldFont.fnt");
		m_musicTitle->setAnchorPoint({ .0f, .5f });
		this->addChildAtPosition(m_musicTitle, Anchor::TopLeft, ccp(20, -20));

		m_musicSlider = Slider::create(nullptr, nullptr);
		m_musicSlider->setAnchorPoint({ .0f, .5f });
		m_musicSlider->m_touchLogic->m_thumb->setVisible(false);
		this->addChildAtPosition(m_musicSlider, Anchor::Top, ccp(0, -20));

		m_musicPercent = CCLabelBMFont::create("", "bigFont.fnt");
		m_musicPercent->setAnchorPoint({ 1.f, .5f });
		this->addChildAtPosition(m_musicPercent, Anchor::TopRight, ccp(-20, -20));

		m_sfxTitle = CCLabelBMFont::create("SFX", "goldFont.fnt");
		m_sfxTitle->setAnchorPoint({ .0f, .5f });
		this->addChildAtPosition(m_sfxTitle, Anchor::BottomLeft, ccp(20, 28));

		m_sfxSlider = Slider::create(nullptr, nullptr);
		m_sfxSlider->setAnchorPoint({ .0f, .5f });
		m_sfxSlider->m_touchLogic->m_thumb->setVisible(false);
		this->addChildAtPosition(m_sfxSlider, Anchor::Bottom, ccp(0, 20));

		m_sfxPercent = CCLabelBMFont::create("", "bigFont.fnt");
		m_sfxPercent->setAnchorPoint({ 1.f, .5f });
		this->addChildAtPosition(m_sfxPercent, Anchor::BottomRight, ccp(-20, 20));

		m_sfxShiftInfo = CCLabelBMFont::create("(Shift)", "bigFont.fnt");
		m_sfxShiftInfo->setAnchorPoint({ 0.f, .5f });
		m_sfxShiftInfo->setScale(.35f);
		this->addChildAtPosition(m_sfxShiftInfo, Anchor::BottomLeft, ccp(20, 10));

		// Update both sliders
		this->updateValue(0, false, false);
		this->updateValue(0, false, true);

		return true;
	}

	static VolumeControlOverlay* create() {
		auto ret = new VolumeControlOverlay();
		if (ret && ret->init()) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	template <class T>
	void show(T* child, GLubyte opacity = 255) {
		child->stopAllActions();
		child->setOpacity(opacity);
		child->runAction(CCSequence::create(
			CCDelayTime::create(.75f),
			CCFadeTo::create(.5f, 0),
			nullptr
		));
	}

public:
	static VolumeControlOverlay* get() {
		if (auto existing = CCScene::get()->getChildByID("overlay"_spr)) {
			return static_cast<VolumeControlOverlay*>(existing);
		}
		auto create = VolumeControlOverlay::create();
		create->setID("overlay"_spr);
		CCScene::get()->addChildAtPosition(create, Anchor::BottomRight, ccp(-10, 10), false);
		return create;
	}

	void updateValue(float add, bool slow, bool sfx) {
		auto fe = FMODAudioEngine::sharedEngine();
		auto value = clamp((sfx ? fe->m_sfxVolume : fe->m_musicVolume) + add / 240 * (slow ? .15f : 1), 0, 1);
		auto str = std::to_string(static_cast<int>(value * 100)) + "%";
		(sfx ? m_sfxSlider : m_musicSlider)->setValue(value);
		(sfx ? m_sfxSlider : m_musicSlider)->updateBar();
		(sfx ? fe->m_globalChannel : fe->m_backgroundMusicChannel)->setVolume(value);
		(sfx ? fe->m_sfxVolume : fe->m_musicVolume) = value;
		(sfx ? m_sfxPercent : m_musicPercent)->setString(str.c_str());
		(sfx ? m_sfxPercent : m_musicPercent)->limitLabelWidth(60.f, .8f, .1f);
		(sfx ? m_sfxTitle : m_musicTitle)->limitLabelWidth(60.f, .9f, .1f);

		(sfx ? m_sfxTitle : m_musicTitle)->setColor({ 255, 255, 255 });
		(sfx ? m_musicTitle : m_sfxTitle)->setColor({ 100, 100, 100 });

		(sfx ? m_sfxSlider : m_musicSlider)->m_sliderBar->setColor(slow ? ccColor3B { 0, 155, 255 } : ccWHITE);
		(sfx ? m_musicSlider : m_sfxSlider)->m_sliderBar->setColor(ccWHITE);

		this->show(m_bg, 205);
		this->show(m_musicTitle);
		this->show(m_musicSlider->m_sliderBar);
		this->show(m_musicSlider->m_groove);
		this->show(m_musicPercent);
		this->show(m_sfxTitle);
		this->show(m_sfxSlider->m_sliderBar);
		this->show(m_sfxSlider->m_groove);
		this->show(m_sfxPercent);
		this->show(m_sfxShiftInfo);
	}
};

class $modify(CCMouseDispatcher) {
	bool dispatchScrollMSG(float y, float x) {
		auto kb = CCKeyboardDispatcher::get();
		if (kb->getAltKeyPressed()) {
        	VolumeControlOverlay::get()->updateValue(-y, kb->getControlKeyPressed(), kb->getShiftKeyPressed());
			return true;
		}
		return CCMouseDispatcher::dispatchScrollMSG(y, x);
	}
};
