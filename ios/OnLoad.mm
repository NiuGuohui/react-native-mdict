#import <Foundation/Foundation.h>
#import <ReactCommon/CxxTurboModuleUtils.h>
#import "NativeMdictModule.h"


@interface OnLoad : NSObject
@end

@implementation OnLoad

using namespace facebook::react;

+ (void) load {
  registerCxxModuleToGlobalModuleMap(
    std::string(NativeMdictModule::kModuleName),
    [](std::shared_ptr<CallInvoker> jsInvoker) {
      return std::make_shared<NativeMdictModule>(jsInvoker);
    }
  );
}

@end
