//
// Copyright (C) 2020~2020 by CSSlayer
// wengxt@gmail.com
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2 of the
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
#include "testfrontend.h"
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontextmanager.h>
#include <fcitx/inputpanel.h>

namespace fcitx {

class TestInputContext : public InputContext {
public:
    TestInputContext(InputContextManager &inputContextManager,
                     const std::string &program)
        : InputContext(inputContextManager, program) {
        created();
    }
    ~TestInputContext() { destroy(); }

    const char *frontend() const override { return "testfrontend"; }

    void commitStringImpl(const std::string &text) override {
        FCITX_INFO() << "Commit: " << text;
    }
    void forwardKeyImpl(const ForwardKeyEvent &key) override {
        FCITX_INFO() << "ForwardKey: " << key.key();
    }
    void deleteSurroundingTextImpl(int offset, unsigned int size) override {
        FCITX_INFO() << "DeleteSurrounding: " << offset << " " << size;
    }

    void updatePreeditImpl() override {
        FCITX_INFO() << "Update preedit: "
                     << inputPanel().clientPreedit().toString();
    }
};

TestFrontend::TestFrontend(Instance *instance) : instance_(instance) {}

TestFrontend::~TestFrontend() {}

ICUUID
TestFrontend::createInputContext(const std::string &program) {
    auto ic = new TestInputContext(instance_->inputContextManager(), program);
    return ic->uuid();
}

void TestFrontend::destroyInputContext(ICUUID uuid) {
    auto ic = instance_->inputContextManager().findByUUID(uuid);
    delete ic;
}
void TestFrontend::keyEvent(ICUUID uuid, const Key &key, bool isRelease) {
    auto ic = instance_->inputContextManager().findByUUID(uuid);
    if (!ic) {
        return;
    }
    KeyEvent keyEvent(ic, key, isRelease);
    ic->keyEvent(keyEvent);
    FCITX_INFO() << "KeyEvent key: " << key.toString()
                 << " isRelease: " << isRelease
                 << " accepted: " << keyEvent.accepted();
}

class TestFrontendFactory : public AddonFactory {
public:
    AddonInstance *create(AddonManager *manager) override {
        return new TestFrontend(manager->instance());
    }
};

} // namespace fcitx

FCITX_ADDON_FACTORY(fcitx::TestFrontendFactory);
