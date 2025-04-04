#include <Geode/Geode.hpp>
#include <Geode/modify/CCTextInputNode.hpp>
#include <alphalaneous.alphas_geode_utils/include/NodeModding.h>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
#include <string_view>
#include <Geode/modify/CCEGLView.hpp>

CCTextInputNode* g_selectedInput = nullptr;
#define ENABLE_BUTTONS Mod::get()->getSettingValue<bool>("enable-buttons")
#else
#define ENABLE_BUTTONS true
#endif

class $nodeModify(AlphaCCTextInputNode, CCTextInputNode) {
	void modify() {
		if (!ENABLE_BUTTONS) return;

		CCMenu* specialButtons = CCMenu::create();

		specialButtons->setContentSize({40, getContentSize().height});
		specialButtons->setScale(0.9f);

		AxisLayout* layout = AxisLayout::create();

		layout->setAxis(Axis::Column);
		layout->setAxisReverse(true);
		layout->setGap(2);

		specialButtons->setLayout(layout);

		CCSprite* copySpr = ButtonSprite::create("C", 40, true, "goldFont.fnt", "square02_small.png", 40, 1);
		CCSprite* pasteSpr = ButtonSprite::create("P", 40, true, "goldFont.fnt", "square02_small.png", 40, 1);
		
		copySpr->setScale(0.5f);
		pasteSpr->setScale(0.5f);

		CCScale9Sprite* copyBg = copySpr->getChildByType<CCScale9Sprite>(0);
		CCScale9Sprite* pasteBg = pasteSpr->getChildByType<CCScale9Sprite>(0);

		copyBg->setOpacity(64);
		pasteBg->setOpacity(64);

		CCMenuItemSpriteExtra* copyButton = CCMenuItemSpriteExtra::create(copySpr, this, menu_selector(AlphaCCTextInputNode::onCopy));
		CCMenuItemSpriteExtra* pasteButton = CCMenuItemSpriteExtra::create(pasteSpr, this, menu_selector(AlphaCCTextInputNode::onPaste));
	
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
	}
	void onPaste(CCObject* obj) {

		CCTextInputNode* node = reinterpret_cast<CCTextInputNode*>(this);

		std::string text = node->getString();
		text.append(clipboard::read());
	
		node->setString(text);
	}

	void onCopy(CCObject* obj) {
		CCTextInputNode* node = reinterpret_cast<CCTextInputNode*>(this);

		clipboard::write(node->getString());
	}
};

class $modify(MyCCTextInputNode, CCTextInputNode) {
#ifdef GEODE_IS_WINDOWS
	struct Fields {
		// currently selected string (from left to right, until the cursor)
		// or if cursor is at the end, the whole string
		std::string m_string;
		int m_pos;
	};
#endif

	bool onTextFieldAttachWithIME(cocos2d::CCTextFieldTTF* tField) {
#ifdef GEODE_IS_WINDOWS
		g_selectedInput = this;
#endif
		if (auto specialButtons = getChildByID("copy-paste-menu"_spr)) {
			specialButtons->setVisible(true);
			if (m_textField && m_textField->getAnchorPoint().x == 0) {
				specialButtons->setPosition({ getContentSize().width, 0 });
			}
		}

		return CCTextInputNode::onTextFieldAttachWithIME(tField);
	}

    bool onTextFieldDetachWithIME(cocos2d::CCTextFieldTTF* tField) {
#ifdef GEODE_IS_WINDOWS
		g_selectedInput = nullptr;
#endif
		if (auto specialButtons = getChildByID("copy-paste-menu"_spr)) {
			specialButtons->setVisible(false);
		}

		return CCTextInputNode::onTextFieldDetachWithIME(tField);
	}

#ifdef GEODE_IS_WINDOWS
	void textChanged()
	{
		CCTextInputNode::textChanged();

		m_fields->m_string = this->getString();
	}

	void updateBlinkLabelToChar(int pos)
	{
		CCTextInputNode::updateBlinkLabelToChar(pos);
		m_fields->m_pos = pos;

		// -1 is the end, 0 is the beginning
		if (pos == -1 || pos == 0) {
			m_fields->m_string = this->getString();
		}
		else {
			m_fields->m_string = std::string_view(this->getString()).substr(0, pos);
		}
	}
#endif
};

#ifdef GEODE_IS_WINDOWS

class $modify(CCEGLView) {
	void onGLFWKeyCallback(
		GLFWwindow* window,
		int key,
		int scancode,
		int action, // 0 released, 1 clicked, 2 held 
		int mods
	) {
		static int previousKey = '\0';
		static bool avoidCurrentAndNextInput = false;

		if (ENABLE_BUTTONS || !g_selectedInput) {
			return CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);
		}

		if (avoidCurrentAndNextInput) {
			avoidCurrentAndNextInput = false;
			return;
		}

		// avoid some very weird behavior where ctrl+v is executed but onGLFWKeyCallback isn't called
		if (action == 2 && key == 0x155) {
			avoidCurrentAndNextInput = true;
			return;
		}

		// 0x155 being Left Ctrl
		if (g_selectedInput && previousKey == 0x155) {
			if (key == 'A') {
				g_selectedInput->updateBlinkLabelToChar(-1);
			} else if (key == 'V') {
				CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);

				g_selectedInput->updateBlinkLabelToChar(
					static_cast<MyCCTextInputNode*>(g_selectedInput)->m_fields->m_pos
				);

				previousKey = key;
				return;
			} else if (key == 'C') {
				clipboard::write(
					static_cast<MyCCTextInputNode*>(g_selectedInput)->m_fields->m_string
				);

				previousKey = key;
				return;
			}
		}

		if (!avoidCurrentAndNextInput) {
			CCEGLView::onGLFWKeyCallback(window, key, scancode, action, mods);
		}

		previousKey = key;
	}
};

#endif
