#pragma once

#include <memory>
#include <string>

#include <ReactCommon/CallInvoker.h>
#include <jsi/jsi.h>

namespace JSMdict {

    using namespace facebook;

    struct ArrayBuffer {
        std::shared_ptr<uint8_t> data;
        size_t size;
    };

    struct JSVariant {
        std::variant<nullptr_t, bool, int, double, long, long long, std::string, ArrayBuffer, std::vector<JSVariant>> value;
    };

    class Promise;

    template<typename T>
    concept PromiseRunFn = std::invocable<T, std::shared_ptr<Promise>> &&
                           std::same_as<std::invoke_result_t<T, std::shared_ptr<Promise>>, void>;

    class Promise: public std::enable_shared_from_this<Promise> {
    public:
        Promise(jsi::Runtime &rt, std::shared_ptr<react::CallInvoker> callInvoker,
                jsi::Function resolver, jsi::Function rejecter);

        Promise(const Promise &) = delete;

        Promise &operator=(const Promise &) = delete;

        void resolve(JSVariant result);

        void reject(std::string error);

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
                                runtime, callInvoker, arguments[0].asObject(runtime).asFunction(runtime),
                                arguments[1].asObject(runtime).asFunction(runtime)
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
        std::shared_ptr<jsi::Function> _resolver;
        std::shared_ptr<jsi::Function> _rejecter;
    };

    inline jsi::Value to_jsi(jsi::Runtime &rt, const JSVariant &value) {
        if (std::holds_alternative<bool>(value.value)) return std::get<bool>(value.value);
        else if (std::holds_alternative<int>(value.value)) return {std::get<int>(value.value)};
        else if (std::holds_alternative<double>(value.value)) return std::get<double>(value.value);
        else if (std::holds_alternative<long long>(value.value)) {
            return static_cast<double>(std::get<long long>(value.value));
        } else if (std::holds_alternative<std::string>(value.value)) {
            return jsi::String::createFromUtf8(rt, std::get<std::string>(value.value));
        } else if (std::holds_alternative<ArrayBuffer>(value.value)) {
            auto jsBuffer = std::get<ArrayBuffer>(value.value);
            jsi::Function array_buffer_ctor = rt.global().getPropertyAsFunction(rt, "ArrayBuffer");
            jsi::Object o = array_buffer_ctor.callAsConstructor(rt, (int) jsBuffer.size).getObject(rt);
            jsi::ArrayBuffer buf = o.getArrayBuffer(rt);
            memcpy(buf.data(rt), jsBuffer.data.get(), jsBuffer.size);
            return o;
        } else if (std::holds_alternative<std::vector<JSVariant>>(value.value)) {
            auto vec = std::get<std::vector<JSVariant>>(value.value);
            auto array_ctor = rt.global().getPropertyAsFunction(rt, "Array");
            auto o = array_ctor.callAsConstructor(rt, (int) vec.size()).getObject(rt).asArray(rt);
            for (int i = 0; i < vec.size(); ++i) {
                o.setValueAtIndex(rt, i, to_jsi(rt, vec[i]));
            }
            return o;
        }

        return jsi::Value::undefined();
    }

    inline JSVariant to_variant(jsi::Runtime &rt, const jsi::Value &value) {
        if (value.isNull() || value.isUndefined()) return JSVariant(nullptr);
        else if (value.isBool()) return JSVariant(value.getBool());
        else if (value.isString()) return JSVariant(value.asString(rt).utf8(rt));
        else if (value.isNumber()) {
            double doubleVal = value.asNumber();
            int intVal = (int) doubleVal;
            long long longVal = (long) doubleVal;
            if (intVal == doubleVal) return JSVariant(intVal);
            else if (longVal == doubleVal) return JSVariant(longVal);
            return JSVariant(doubleVal);
        } else if (value.isObject()) {
            auto obj = value.asObject(rt);
            if (obj.isArrayBuffer(rt)) {
                auto buffer = obj.getArrayBuffer(rt);
                auto *data = new uint8_t[buffer.size(rt)];
                memcpy(data, buffer.data(rt), buffer.size(rt));

                return JSVariant(
                        ArrayBuffer{.data = std::shared_ptr<uint8_t>{data}, .size = buffer.size(rt)}
                );
            } else if (obj.isArray(rt)) {
                auto arr = obj.asArray(rt);
                std::vector<JSVariant> vec;
                for (int i = 0; i < arr.size(rt); i++) {
                    vec.push_back(to_variant(rt, arr.getValueAtIndex(rt, i)));
                }
                return JSVariant(vec);
            }
        }

        throw std::runtime_error("Invalid Value: Cannot convert JS value to C++ Variant value");
    }
} // namespace JSMdict