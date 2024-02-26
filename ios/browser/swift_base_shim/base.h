#import <Foundation/Foundation.h>
#include <string>
#include <swift/bridging>

namespace swift_base {

std::string SysNSStringToUTF8(NSString* ref);
NSDate* GetTime() __attribute__ ((visibility ("default")));

class SwiftFun {
public:
  virtual void DoFunStuff();
  static SwiftFun *Create();
};
}
