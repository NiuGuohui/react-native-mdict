#include "Promise.h"

namespace JSMdict {

    Promise::Promise(jsi::Runtime &rt, std::shared_ptr<react::CallInvoker> callInvoker, jsi::Function resolver,
                     jsi::Function rejecter)
            : runtime(rt), callInvoker(std::move(callInvoker)),
              _resolver(std::make_shared<jsi::Function>(std::move(resolver))),
              _rejecter(std::make_shared<jsi::Function>(std::move(rejecter))) {}

    void Promise::resolve(JSVariant result) {
        auto self = shared_from_this();
        callInvoker->invokeAsync([self, r = std::move(result)]() mutable {
            self->_resolver->call(self->runtime, std::move(to_jsi(self->runtime, r)));
        });
    }

    void Promise::reject(std::string message) {
        auto self = shared_from_this();
        callInvoker->invokeAsync([self, message]() {
            self->_rejecter->call(self->runtime, jsi::JSError(self->runtime, message).value());
        });
    }

} // namespace JSMdict