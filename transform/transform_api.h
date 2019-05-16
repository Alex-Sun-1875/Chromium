#ifndef CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_

#include "chrome/common/extensions/api/transform.h"
#include "chrome/browser/extensions/chrome_extension_function.h"
#include "extensions/common/extension.h"
#include "extensions/browser/event_router.h"
// #include "extensions/browser/extension_host.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"

namespace extensions {
namespace api {

class TransformOpenFileInCurrentTabFunction : public ChromeAsyncExtensionFunction {
  public:
    TransformOpenFileInCurrentTabFunction();

  protected:
    ~TransformOpenFileInCurrentTabFunction() override;
    bool RunAsync() override;

  private:
    DECLARE_EXTENSION_FUNCTION("transform.openFileInCurrentTab", TRANSFORM_OPENFILEINCURRENTTAB)
};

class TransformOpenFileInNewTabFunction : public ChromeAsyncExtensionFunction {
  public:
    TransformOpenFileInNewTabFunction();

  protected:
    ~TransformOpenFileInNewTabFunction() override;
    bool RunAsync() override;

  private:
    DECLARE_EXTENSION_FUNCTION("transform.openFileInNewTab", TRANSFORM_OPENFILEINNEWTAB);
};

class TransformShowFileInFloderFunction : public ChromeAsyncExtensionFunction {
public:
  TransformShowFileInFloderFunction();

protected:
  ~TransformShowFileInFloderFunction() override;
  bool RunAsync() override;

private:
  DECLARE_EXTENSION_FUNCTION("transform.showFileInFloder", TRANSFORM_SHOWFILEINFLODER);
};

class ExtensionTransformEventRouter
  : public extensions::EventRouter::Observer,
    public extensions::ExtensionRegistryObserver {
  public:
    ExtensionTransformEventRouter(Profile* profile,
                                  scoped_refptr<const Extension> extension);

    ~ExtensionTransformEventRouter() override;

    void SendStartTransform();

  private:
    Profile* profile_;
    scoped_refptr<const Extension> extension_;

    DISALLOW_COPY_AND_ASSIGN(ExtensionTransformEventRouter);
};

}  // namespace api
}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_
