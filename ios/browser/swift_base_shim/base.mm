#include "brave/ios/browser/swift_base_shim/base.h"

#include "base/time/time.h"
#include "base/strings/sys_string_conversions.h"

namespace swift_base {

std::string SysNSStringToUTF8(NSString* ref) {
  return base::SysNSStringToUTF8(ref);
}

NSDate* GetTime() {
  return base::Time::Now().ToNSDate();
}

}
