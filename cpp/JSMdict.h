#pragma once

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
#include <mdict.h>
#include "Promise.h"
#include "CancellableThreadPool.h"

namespace JSMdict {
    using namespace facebook;

    class JSMdict : public jsi::HostObject {
    public:
        std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime &rt) override;

        jsi::Value get(jsi::Runtime &runtime, const jsi::PropNameID &name) override;

        JSMdict(std::string file, std::shared_ptr<react::CallInvoker> jsInvoker);

        ~JSMdict();

        jsi::Value init(jsi::Runtime &runtime);

        jsi::Value keyList(jsi::Runtime &runtime, std::string query);

        jsi::Value lookup(jsi::Runtime &runtime, std::string value);

    private:
        mdict::Mdict *mdict;
        std::weak_ptr<react::CallInvoker> jsInvoker;
        bool initialized = false;
        CancellableThreadPool threadPool;
        CancellableThreadPool::TaskID currentSearchTaskId = 0;

        inline void checkInitialized(jsi::Runtime &runtime) {
            if (!initialized) throw jsi::JSError(runtime, "Mdict not initialized");
        };

    };
}