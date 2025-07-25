#include "JSMdict.h"
#include "mdict_extern.h"
#include <thread>

namespace JSMdict {
    using namespace facebook::jsi;
    using namespace facebook::react;

    JSMdict::JSMdict(std::string file, std::shared_ptr<CallInvoker> _jsInvoker)
            : filePath(file),
              mdict(new mdict::Mdict(file)),
              jsInvoker(std::weak_ptr<CallInvoker>(_jsInvoker)) {
    }

    std::vector<PropNameID> JSMdict::getPropertyNames(Runtime &rt) {
        std::vector<PropNameID> vec;
        vec.reserve(1);
        vec.push_back(PropNameID::forAscii(rt, "search"));
        return vec;
    }

    Value JSMdict::get(Runtime &runtime, const PropNameID &name) {
        if (name.utf8(runtime) == "search") {
            return Function::createFromHostFunction(
                    runtime,
                    PropNameID::forAscii(runtime, "search"),
                    1,
                    [this](Runtime &runtime, const Value &thisValue, const Value *args, size_t count) -> Value {
                        if (!args[0].isString()) {
                            throw JSError(runtime, "Invalid argument of index 0");
                        }
                        return search(runtime, args[0].getString(runtime).utf8(runtime));
                    });
        }
        return Value::undefined();
    }

    Value JSMdict::search(Runtime &runtime, std::string value) {
        try {
            mdict->init();
        } catch (std::exception &e) {
//            return Promise::createPromise(runtime,
//                                   jsInvoker.lock(),
//                                   [this, &runtime](std::shared_ptr<Promise> promise) {
//
//                                   });
        }
        auto s = mdict->lookup(value);
        if (s.empty()) return Value::undefined();
        return Value(runtime, String::createFromUtf8(runtime, s));
    }

}

