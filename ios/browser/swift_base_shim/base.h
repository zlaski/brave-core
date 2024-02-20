#include <string>
#import <Foundation/Foundation.h>

namespace swift_base {

std::string SysNSStringToUTF8(NSString* ref);
NSDate* GetTime() __attribute__ ((visibility ("default")));

}
