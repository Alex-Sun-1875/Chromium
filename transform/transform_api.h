#ifndef CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_
#define CHROME_BROWSER_EXTENSIONS_API_BROWSER_TRANSFORM_API_H_

namespace extensions {
namespace api {

class TransfromFunction : public ChromeUIThreadExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("transform.openTransformDialog", TRANSFORM_OPENTRANSFORMDIALOG)

 protected:
  ~TransfromFunction() override;

  ResponseAction Run() override;
};

}  // namespace api
}  // namespace extensions

#endif
