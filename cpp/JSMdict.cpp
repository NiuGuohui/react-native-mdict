#include "JSMdict.h"
#include <string>
#include <thread>
#include <utility>

namespace JSMdict {
    JSMdict::JSMdict(std::string file, std::shared_ptr<react::CallInvoker> jsInvoker)
            : mdict(new mdict::Mdict(std::move(file))),
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
        return Promise::createPromise(runtime, jsInvoker.lock(), [this](const std::shared_ptr<Promise> &promise) {
            threadPool.enqueue([this, promise]() {
                try {
                    mdict->init();
                    initialized = true;
                    promise->resolve(JSVariant(nullptr));
                } catch (std::exception &e) {
                    promise->reject(e.what());
                }
            });
        });
    }

    jsi::Value JSMdict::lookup(jsi::Runtime &runtime, std::string value) {
        checkInitialized(runtime);
        return Promise::createPromise(
                runtime,
                jsInvoker.lock(),
                [this, value](const std::shared_ptr<Promise> &promise) {
                    threadPool.enqueue([this, value, promise]() {
                        auto s = mdict->lookup(value);
                        if (s.empty()) promise->resolve(JSVariant(nullptr));
                        else promise->resolve(JSVariant(s));
                    });
                });
    }

    jsi::Value JSMdict::keyList(jsi::Runtime &runtime, std::string query) {
        checkInitialized(runtime);
        return Promise::createPromise(
                runtime,
                jsInvoker.lock(),
                [this, query](const std::shared_ptr<Promise> &promise) {
                    threadPool.cancel(currentSearchTaskId);
                    currentSearchTaskId = threadPool.enqueue([this, promise, query]() {
                        auto list = std::vector<JSVariant>();
                        auto keys = mdict->keyList();
                        for (auto &key: keys) {
                            if (key->key_word.find(query) != std::string::npos) {
                                list.insert(list.end(), JSVariant(key->key_word));
                            }
                        }
                        promise->resolve(JSVariant(list));
                    });
                });
    }

}

