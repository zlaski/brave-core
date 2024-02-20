import Foundation
import brave_ios_browser_swift_test_one_swift_test_one
@_implementationOnly import swift_base_shim

public struct FooUser {
  let foo = Foo()

  public init() {
    let a = swift_base.SysNSStringToUTF8("")
  }

  public func getTime() -> Date {
    return swift_base.GetTime()
  }
}