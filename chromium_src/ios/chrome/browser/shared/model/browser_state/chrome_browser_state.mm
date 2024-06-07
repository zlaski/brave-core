
// URLRequestJobFactory::CreateJob checks the protocol_handler_map_
// to see what requests can be handled
// The FactoryForMain contains the ProtocolHandlerMap
// This is initialized via ChromeBrowserStateIOData::Init(ProtocolHandlerMap* protocol_handlers)
// Which is called via ChromeBrowserState::GetRequestContext

#include "src/ios/chrome/browser/shared/model/browser_state/chrome_browser_state.h"
#include "ios/components/webui/web_ui_url_constants.h"
#include "ios/web/webui/url_data_manager_ios_backend.h"

// The below const_cast would work, but doesn't seem like a good idea :S
// const_cast<T>(browser_state->GetRequestContext()->GetURLRequestContext()->job_factory())->SetProtocolHandler("chrome-untrusted", web::URLDataManagerIOSBackend::CreateProtocolHandler(browser_state))

// Add the chrome-untrusted scheme
auto CreateRequestContext_Brave(ChromeBrowserState* browser_state) {
  return [browser_state](ProtocolHandlerMap* protocol_handlers) {
    protocol_handlers->insert({"chrome-untrusted", web::URLDataManagerIOSBackend::CreateProtocolHandler(browser_state)});
    return browser_state->CreateRequestContext(protocol_handlers);
  };
}

#define CreateRequestContext CreateRequestContext_Brave(this)

#include "src/ios/chrome/browser/shared/model/browser_state/chrome_browser_state.mm"

#undef CreateRequestContext
