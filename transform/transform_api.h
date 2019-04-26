#ifndef CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_

#include "chrome/common/extensions/api/transform.h"
#include "chrome/browser/extensions/chrome_extension_function.h"

namespace extensions {
namespace api {

class TransformOpenDialogFunction : public ChromeAsyncExtensionFunction {
  public:
    TransformOpenDialogFunction();

  protected:
    ~TransformOpenDialogFunction() override;
    virtual bool RunAsync() override;

  private:
    DECLARE_EXTENSION_FUNCTION("transform.openDialog", TRANSFORM_OPENDIALOG)
};

}  // namespace api
}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_
