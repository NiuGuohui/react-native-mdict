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

    std::string toLower(std::string_view str) {
        std::string result = std::string(str);
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    int levenshteinDistance(const std::string &s1, const std::string &s2) {
        size_t len1 = s1.size(), len2 = s2.size();
        std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1));

        for (size_t i = 0; i <= len1; ++i) dp[i][0] = i;
        for (size_t j = 0; j <= len2; ++j) dp[0][j] = j;

        for (size_t i = 1; i <= len1; ++i) {
            for (size_t j = 1; j <= len2; ++j) {
                dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + (s1[i - 1] != s2[j - 1])});
            }
        }

        return dp[len1][len2];
    }

    int computeScore(const std::string &candidateOriginal, const std::string &queryOriginal) {
        std::string candidate = toLower(candidateOriginal);
        std::string query = toLower(queryOriginal);

        if (candidate == query) return 100;
        if (candidate.starts_with(query)) return 80;
        if (candidate.find(query) != std::string::npos) return 50;
        return std::max(0, 100 - levenshteinDistance(candidate, query) * 10);
    }

    std::vector<JSVariant>
    sortKeyListByKeywordRelevance(std::vector<mdict::key_list_item *> &items, const std::string &query) {
        auto compare = [&query](const mdict::key_list_item *a, const mdict::key_list_item *b) {
            int scoreA = computeScore(a->key_word, query);
            int scoreB = computeScore(b->key_word, query);
            return scoreA > scoreB || (scoreA == scoreB && a->record_start < b->record_start);
        };
        if (items.size() >= 10) {
            std::partial_sort(items.begin(), items.begin() + 10, items.end(), compare);
            items.resize(10);
        } else {
            std::sort(items.begin(), items.begin(), compare);
        }
        auto list = std::vector<JSVariant>();
        for (auto item: items) list.push_back(JSVariant(item->key_word));
        return list;
    }


    jsi::Value JSMdict::keyList(jsi::Runtime &runtime, std::string query) {
        checkInitialized(runtime);
        return Promise::createPromise(
                runtime,
                jsInvoker.lock(),
                [this, query](const std::shared_ptr<Promise> &promise) {
                    threadPool.cancel(currentSearchTaskId);
                    currentSearchTaskId = threadPool.enqueue([this, promise, query]() {
                        auto tmp = std::vector<mdict::key_list_item *>();
                        auto keys = mdict->keyList();
                        std::copy_if(keys.begin(), keys.end(), std::back_inserter(tmp), [&query](auto key) {
                            return key->key_word.find(query) != std::string::npos;
                        });
                        keys.clear();
                        promise->resolve(JSVariant(sortKeyListByKeywordRelevance(tmp, query)));
                    });
                });
    }
}

