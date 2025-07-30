#include "JSMdict.h"
#include <string>

namespace JSMdict {
    JSMdict::JSMdict(std::string file, std::shared_ptr<react::CallInvoker> jsInvoker)
            : mdict(new mdict::Mdict(file)),
              jsInvoker(std::weak_ptr<react::CallInvoker>(jsInvoker)),
              threadPool(CancellableThreadPool(std::thread::hardware_concurrency())) {
    }

    JSMdict::~JSMdict() {
        initialized = false;
        jsInvoker.reset();
        threadPool.cancel(currentSearchTaskId);
        threadPool.stop();
        delete mdict;
    }

    std::vector<jsi::PropNameID> JSMdict::getPropertyNames(jsi::Runtime &rt) {
        std::vector<jsi::PropNameID> vec;
        vec.reserve(1);
        vec.push_back(jsi::PropNameID::forAscii(rt, "search"));
        return vec;
    }

    jsi::Value JSMdict::get(jsi::Runtime &runtime, const jsi::PropNameID &name) {
        if (name.utf8(runtime) == "init") {
            return jsi::Function::createFromHostFunction(
                    runtime,
                    jsi::PropNameID::forAscii(runtime, "init"),
                    0,
                    [this](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *args, size_t count) {
                        return init(runtime);
                    }
            );
        } else if (name.utf8(runtime) == "lookup") {
            return jsi::Function::createFromHostFunction(
                    runtime,
                    jsi::PropNameID::forAscii(runtime, "lookup"),
                    1,
                    [this](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *args, size_t count) {
                        if (!args[0].isString()) {
                            throw jsi::JSError(runtime, "Invalid argument of index 0");
                        }
                        return lookup(runtime, args[0].getString(runtime).utf8(runtime));
                    }
            );
        } else if (name.utf8(runtime) == "keyList") {
            return jsi::Function::createFromHostFunction(
                    runtime,
                    jsi::PropNameID::forAscii(runtime, "keyList"),
                    1,
                    [this](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *args, size_t count) {
                        if (!args[0].isString()) throw jsi::JSError(runtime, "Invalid argument of index 0");
                        return keyList(runtime, args[0].getString(runtime).utf8(runtime));
                    }
            );
        }
        return jsi::Value::undefined();
    }

    jsi::Value JSMdict::init(jsi::Runtime &runtime) {
        return Promise::createPromise(runtime, jsInvoker.lock(), [&](const std::shared_ptr<Promise> &promise) {
            threadPool.enqueue([&, promise]() {
                try {
                    mdict->init();
                    initialized = true;
                    promise->resolve(jsi::Value::undefined());
                } catch (std::exception &e) {
                    promise->reject(e.what());
                }
            });
        });
    }

    jsi::Value JSMdict::lookup(jsi::Runtime &runtime, std::string value) {
        checkInitialized(runtime);
        return Promise::createPromise(runtime, jsInvoker.lock(), [&, value](const std::shared_ptr<Promise> &promise) {
            threadPool.enqueue([&, promise]() {
                auto s = mdict->lookup(value);
                if (s.empty()) promise->resolve(jsi::Value::undefined());
                else promise->resolve(jsi::Value(runtime, jsi::String::createFromUtf8(runtime, s)));
            });
        });
    }

    jsi::Value JSMdict::keyList(jsi::Runtime &runtime, std::string query) {
        checkInitialized(runtime);
        return Promise::createPromise(runtime, jsInvoker.lock(), [&](const std::shared_ptr<Promise> &promise) {
            threadPool.cancel(currentSearchTaskId);
            currentSearchTaskId = threadPool.enqueue([&, promise, query]() {
                auto list = std::vector<std::string>();
                auto keys = mdict->keyList();
                for (int i = 0; i < keys.size(); i++) {
                    if (keys[i]->key_word.find(query) != std::string::npos) {
                        list.insert(list.end(), keys[i]->key_word);
                    }
                }
                jsi::Array arr(runtime, list.size());
                for (int i = 0; i < list.size(); i++) {
                    arr.setValueAtIndex(runtime, i, jsi::String::createFromAscii(runtime, list[i]));
                }
                promise->resolve(jsi::Value(runtime, arr));
            });
        });
    }

}

