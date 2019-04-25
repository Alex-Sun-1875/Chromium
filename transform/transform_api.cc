#include "chrome/browser/extensions/api/transform_api.h"
#include "chrome/common/extensions/api/transform.h"

namespace extensions {
namespace api {

TransfromOpenDialogFunction::~TransfromOpenDialogFunction() {}

ExtensionFunction::ResponseAction TransfromOpenDialogFunction::Run() {
  std::unique_ptr<transform::OpenDialog::Params> params(
      transform::OpenDialog::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  std::string error("hello error message");
  std::string data = std::string("hello ") + params->url;
  std::unique_ptr<base::ListValue> result(transform::OpenDialog::Results::Create(data));
  if (!result)
    return RespondNow(Error(error));

  return RespondNow(ArgumentList(std::move(result)));
}

}  // namespace api
}  // namespace extensions
