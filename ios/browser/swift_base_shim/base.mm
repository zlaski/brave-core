#include "brave/ios/browser/swift_base_shim/base.h"

#include "base/strings/sys_string_conversions.h"
#include "base/time/time.h"
#include <cstdio>

namespace swift_base {

std::string SysNSStringToUTF8(NSString* ref) {
  return base::SysNSStringToUTF8(ref);
}

NSDate* GetTime() {
  return base::Time::Now().ToNSDate();
}

class SwiftFunImpl : public SwiftFun {
public:
  void DoFunStuff() final { printf("Testing!"); }
  ~SwiftFunImpl() { printf("SwiftFun died"); }
};

SwiftFun *SwiftFun::Create() { return new SwiftFunImpl(); }
}
