#include <Geode/Geode.hpp>
#include <Geode/modify/CCTextInputNode.hpp>

using namespace geode::prelude;

class $modify(MyCCTextInputNode, CCTextInputNode) {
	bool init(float p0, float p1, char const* p2, char const* p3, int p4, char const* p5) {
		if (!CCTextInputNode::init(p0, p1, p2, p3, p4, p5)) {
			return false;
		}

		CCMenu* specialButtons = CCMenu::create();

		specialButtons->setContentSize({40, getContentSize().height});
		specialButtons->setScale(0.9f);

		AxisLayout* layout = AxisLayout::create();

		layout->setAxis(Axis::Column);
		layout->setAxisReverse(true);
		layout->setGap(2);

		specialButtons->setLayout(layout);

		CCSprite* copySpr = ButtonSprite::create("C", 10, true, "goldFont.fnt", "square02_small.png", 20, 1);
		CCSprite* pasteSpr = ButtonSprite::create("P", 10, true, "goldFont.fnt", "square02_small.png", 20, 1);
		
		CCScale9Sprite* copyBg = getChildOfType<CCScale9Sprite>(copySpr, 0);
		CCScale9Sprite* pasteBg = getChildOfType<CCScale9Sprite>(pasteSpr, 0);

		copyBg->setOpacity(64);
		pasteBg->setOpacity(64);

		CCMenuItemSpriteExtra* copyButton = CCMenuItemSpriteExtra::create(copySpr, this, menu_selector(MyCCTextInputNode::onCopy));
		CCMenuItemSpriteExtra* pasteButton = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(MyCCTextInputNode::onPaste));
	
		specialButtons->addChild(copyButton);
		specialButtons->addChild(pasteButton);
		specialButtons->setPosition({getContentSize().width/2, 0});

		//hacky way to ensure scaling correctly

		specialButtons->updateLayout();
		layout->setAutoScale(false);
		specialButtons->updateLayout();
		specialButtons->setContentSize({copyButton->getScaledContentSize().width, copyButton->getScaledContentSize().height*2});
		specialButtons->updateLayout();
		layout->setAutoScale(true);
		specialButtons->updateLayout();

		specialButtons->setVisible(false);
		specialButtons->setID("copy-paste-menu"_spr);

		specialButtons->setTouchPriority(-512);

		addChild(specialButtons);

		return true;
	}

	void onPaste(CCObject* obj) {
		
		std::string text = getString();
		text.append(clipboard::read());
	
		setString(text);
	}

	void onCopy(CCObject* obj) {
		clipboard::write(getString());
	}


	bool onTextFieldAttachWithIME(cocos2d::CCTextFieldTTF* tField) {
		
		if(auto specialButtons = getChildByID("copy-paste-menu"_spr)) {
			specialButtons->setVisible(true);
			if(m_textField && m_textField->getAnchorPoint().x == 0) {
				specialButtons->setPosition({getContentSize().width, 0});
			}
		}

		return CCTextInputNode::onTextFieldAttachWithIME(tField);
	}

    bool onTextFieldDetachWithIME(cocos2d::CCTextFieldTTF* tField) {
		if(auto specialButtons = getChildByID("copy-paste-menu"_spr)) {
			specialButtons->setVisible(false);
		}

		return CCTextInputNode::onTextFieldDetachWithIME(tField);
	}
};
