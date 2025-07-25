#include "NativeMdictModule.h"

namespace facebook::react {
    NativeMdictModule::NativeMdictModule(std::shared_ptr<CallInvoker> jsInvoker) : NativeMdictCxxSpec(
            jsInvoker) {
        this->jsInvoker = jsInvoker;
    }

    jsi::Object
    NativeMdictModule::load(jsi::Runtime &rt, jsi::String file) {
        return jsi::Object::createFromHostObject(rt, std::make_shared<JSMdict::JSMdict>(file.utf8(rt), jsInvoker));
    }
}