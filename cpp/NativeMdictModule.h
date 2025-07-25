#pragma once

#if __has_include(<React-Codegen/MdictSpecJSI.h>)
// CocoaPods include (iOS)
#include <React-Codegen/MdictSpecJSI.h>
#elif __has_include(<MdictSpecJSI.h>)
// CMake include on Android
#include <MdictSpecJSI.h>

#else
#error Cannot find react-native-litehtml spec! Try cleaning your cache and re-running CodeGen!
#endif
#include "JSMdict.h"

namespace facebook::react {

    class NativeMdictModule : public NativeMdictCxxSpec<NativeMdictModule> {
    public:
        NativeMdictModule(std::shared_ptr<CallInvoker> jsInvoker);

        ~NativeMdictModule() override = default;

        jsi::Object load(jsi::Runtime &rt, jsi::String file);

    private:
        std::shared_ptr<CallInvoker> jsInvoker;
    };

} // namespace facebook::react