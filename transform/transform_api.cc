#include "chrome/browser/extensions/api/transform/transform_api.h"

#include "extensions/browser/extension_function_registry.h"
#include "extensions/common/extension.h"

namespace extensions {
namespace api {

TransformOpenDialogFunction::TransformOpenDialogFunction() {}
TransformOpenDialogFunction::~TransformOpenDialogFunction() {}

bool TransformOpenDialogFunction::RunAsync() {
  std::unique_ptr<transform::OpenDialog::Params> params(transform::OpenDialog::Params::Create(*args_));
  return true;
}

} // namespace api
} // namespace extensions
