#pragma once

#include <memory>
#include <string>

#include <ReactCommon/CallInvoker.h>
#include <jsi/jsi.h>

namespace JSMdict {

    using namespace facebook;

    class Promise;

    template<typename T>
    concept PromiseRunFn = std::invocable<T, std::shared_ptr<Promise>> &&
                           std::same_as<std::invoke_result_t<T, std::shared_ptr<Promise>>, void>;

    class Promise {
    public:
        Promise(jsi::Runtime &runtime,
                std::shared_ptr<react::CallInvoker> callInvoker,
                jsi::Value resolver,
                jsi::Value rejecter);

        Promise(const Promise &) = delete;

        Promise &operator=(const Promise &) = delete;

        void resolve(jsi::Value result);

        void reject(const std::string error);

        static jsi::Value resolve(jsi::Runtime &runtime, jsi::Value value) {
            auto promiseResolve = runtime.global()
                    .getPropertyAsFunction(runtime, "Promise")
                    .getPropertyAsFunction(runtime, "resolve");
            return promiseResolve.call(runtime, value);
        };

        static jsi::Value reject(jsi::Runtime &runtime, jsi::Value value) {
            auto promiseReject = runtime.global()
                    .getPropertyAsFunction(runtime, "Promise")
                    .getPropertyAsFunction(runtime, "reject");
            return promiseReject.call(runtime, value);
        }

        /**
          Creates a new promise and runs the supplied "run" function that takes this
          promise. We use a template for the function type to not use std::function
          and be able to bind a lambda.
        */
        template<PromiseRunFn Fn>
        static jsi::Value
        createPromise(jsi::Runtime &runtime, std::shared_ptr<react::CallInvoker> callInvoker, Fn &&run) {
            // Get Promise ctor from global
            auto promiseCtor =
                    runtime.global().getPropertyAsFunction(runtime, "Promise");

            auto promiseCallback = jsi::Function::createFromHostFunction(
                    runtime,
                    jsi::PropNameID::forUtf8(runtime, "anonymous"),
                    2,
                    [run, callInvoker](jsi::Runtime &runtime, const jsi::Value &thisValue, const jsi::Value *arguments,
                                       size_t count) {
                        // Call function
                        auto promise = std::make_shared<Promise>(
                                runtime, callInvoker, arguments[0].asObject(runtime), arguments[1].asObject(runtime)
                        );
                        run(promise);

                        return jsi::Value::undefined();
                    }
            );

            return promiseCtor.callAsConstructor(runtime, promiseCallback);
        }

    private:
        jsi::Runtime &runtime;
        std::shared_ptr<react::CallInvoker> callInvoker;
        jsi::Value _resolver;
        jsi::Value _rejecter;
    };

} // namespace JSMdict