#include "Promise.h"

namespace JSMdict {

    Promise::Promise(jsi::Runtime &runtime,
                     std::shared_ptr<react::CallInvoker> callInvoker,
                     jsi::Value resolver, jsi::Value rejecter)
            : runtime(runtime), callInvoker(std::move(callInvoker)), _resolver(std::move(resolver)),
              _rejecter(std::move(rejecter)) {}

    void Promise::resolve(jsi::Value result) {
        auto v = std::make_shared<jsi::Value>(std::move(result));
        callInvoker->invokeAsync([this, v]() {
            _resolver.asObject(runtime).asFunction(runtime).call(runtime, *v);
        });
    }

    void Promise::reject(const std::string message) {
        callInvoker->invokeAsync([this, message]() {
            _rejecter.asObject(runtime).asFunction(runtime).call(
                    runtime,
                    jsi::JSError(runtime, std::move(message)).value()
            );
        });
    }

} // namespace JSMdict