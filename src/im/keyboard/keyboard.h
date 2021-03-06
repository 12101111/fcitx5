//
// Copyright (C) 2016~2016 by CSSlayer
// wengxt@gmail.com
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; see the file COPYING. If not,
// see <http://www.gnu.org/licenses/>.
//
#ifndef _FCITX_IM_KEYBOARD_KEYBOARD_H_
#define _FCITX_IM_KEYBOARD_KEYBOARD_H_

#include <xkbcommon/xkbcommon-compose.h>
#include <xkbcommon/xkbcommon.h>
#include "fcitx-config/configuration.h"
#include "fcitx-config/iniparser.h"
#include "fcitx-utils/i18n.h"
#include "fcitx-utils/inputbuffer.h"
#include "fcitx/addonfactory.h"
#include "fcitx/addonmanager.h"
#include "fcitx/inputcontextproperty.h"
#include "fcitx/inputmethodengine.h"
#include "fcitx/instance.h"
#include "isocodes.h"
#include "keyboard_public.h"
#include "xkbrules.h"

namespace fcitx {

class Instance;

FCITX_CONFIG_ENUM(ChooseModifier, None, Alt, Control, Super);
FCITX_CONFIG_ENUM_I18N_ANNOTATION(ChooseModifier, N_("None"), N_("Alt"),
                                  N_("Control"), N_("Super"));

FCITX_CONFIGURATION(
    KeyboardEngineConfig,
    Option<int, IntConstrain> pageSize{this, "PageSize", _("Page size"), 5,
                                       IntConstrain(3, 10)};
    KeyListOption prevCandidate{
        this,
        "PrevCandidate",
        _("Prev Candidate"),
        {Key("Shift+Tab")},
        KeyListConstrain(KeyConstrainFlag::AllowModifierLess)};
    KeyListOption nextCandidate{
        this,
        "NextCandidate",
        _("Next Candidate"),
        {Key("Tab")},
        KeyListConstrain(KeyConstrainFlag::AllowModifierLess)};
    Option<bool> enableEmoji{this, "EnableEmoji", _("Enable emoji in hint"),
                             true};
    OptionWithAnnotation<ChooseModifier, ChooseModifierI18NAnnotation>
        chooseModifier{this, "Choose Modifier", _("Choose key modifier"),
                       ChooseModifier::Alt};
    KeyListOption hintTrigger{this,
                              "Hint Trigger",
                              _("Trigger hint mode"),
                              {Key("Control+Alt+H")},
                              KeyListConstrain()};);

class KeyboardEngine;

struct KeyboardEngineState : public InputContextProperty {
    bool enableWordHint_ = false;
    InputBuffer buffer_;

    void reset() { buffer_.clear(); }
};

class KeyboardEnginePrivate;

class KeyboardEngine final : public InputMethodEngine {
public:
    KeyboardEngine(Instance *instance);
    ~KeyboardEngine();
    Instance *instance() { return instance_; }
    void keyEvent(const InputMethodEntry &entry, KeyEvent &keyEvent) override;
    std::vector<InputMethodEntry> listInputMethods() override;
    void reloadConfig() override;

    const Configuration *getConfig() const override { return &config_; }
    void setConfig(const RawConfig &config) override {
        config_.load(config, true);
        safeSaveAsIni(config_, "conf/keyboard.conf");
        reloadConfig();
    }

    void reset(const InputMethodEntry &entry,
               InputContextEvent &event) override;

    void resetState(InputContext *inputContext);

    FCITX_ADDON_DEPENDENCY_LOADER(spell, instance_->addonManager());
    FCITX_ADDON_DEPENDENCY_LOADER(notifications, instance_->addonManager());
    FCITX_ADDON_DEPENDENCY_LOADER(emoji, instance_->addonManager());

    void updateCandidate(const InputMethodEntry &entry,
                         InputContext *inputContext);

    auto state() { return &factory_; }

    bool
    foreachLayout(const std::function<bool(
                      const std::string &layout, const std::string &description,
                      const std::vector<std::string> &languages)> &callback);
    bool foreachVariant(
        const std::string &layout,
        const std::function<
            bool(const std::string &variant, const std::string &description,
                 const std::vector<std::string> &languages)> &callback);

private:
    FCITX_ADDON_EXPORT_FUNCTION(KeyboardEngine, foreachLayout);
    FCITX_ADDON_EXPORT_FUNCTION(KeyboardEngine, foreachVariant);

    std::string preeditString(InputContext *inputContext);
    void commitBuffer(InputContext *inputContext);
    void updateUI(InputContext *inputContext);

    Instance *instance_;
    AddonInstance *spell_ = nullptr;
    AddonInstance *notifications_ = nullptr;
    KeyboardEngineConfig config_;
    IsoCodes isoCodes_;
    XkbRules xkbRules_;
    std::string ruleName_;
    KeyList selectionKeys_;

    FactoryFor<KeyboardEngineState> factory_{
        [](InputContext &) { return new KeyboardEngineState; }};
};

class KeyboardEngineFactory : public AddonFactory {
public:
    AddonInstance *create(AddonManager *manager) override {
        return new KeyboardEngine(manager->instance());
    }
};
} // namespace fcitx

#endif // _FCITX_IM_KEYBOARD_KEYBOARD_H_
