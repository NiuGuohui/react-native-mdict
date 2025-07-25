#pragma once

#include <jsi/jsi.h>
#include <ReactCommon/CallInvoker.h>
#include <mdict.h>
#include "Promise.h"

namespace JSMdict {
    using namespace facebook::jsi;
    using namespace facebook::react;

    class JSMdict : public HostObject {
    public:
        std::vector<PropNameID> getPropertyNames(Runtime &rt) override;

        Value get(Runtime &runtime, const PropNameID &name) override;

        JSMdict(std::string file, std::shared_ptr<CallInvoker> jsInvoker);

        Value search(Runtime &runtime, std::string value);

    private:
        std::string filePath;
        mdict::Mdict *mdict;
        std::weak_ptr<CallInvoker> jsInvoker;

    };
}