#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <BraveCore/BraveCore.h>

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (nonatomic) UIWindow *window;
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey, id> *)launchOptions {
  self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
  self.window.rootViewController = [[UIViewController alloc] init];

  BraveCoreMain* _main = [[BraveCoreMain alloc] initWithUserAgent:@""];
  [_main scheduleLowPriorityStartupTasks];

  return YES;
}

@end

int main(int argc, char* argv[]) {
  @autoreleasepool {
    return UIApplicationMain(argc, argv, nil, @"AppDelegate");
  }
}
