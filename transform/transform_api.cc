#include "chrome/browser/extensions/api/transform/transform_api.h"

#include <string>

#include "chrome/browser/extensions/extension_tab_util.h"
#include "extensions/browser/extension_function_registry.h"
#include "extensions/browser/extension_function.h"
#include "extensions/common/extension.h"

#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/app/chrome_command_ids.h"

#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/strings/utf_string_conversions.h"
#include "chrome/browser/platform_util.h"

namespace extensions {
namespace api {

TransformOpenDialogFunction::TransformOpenDialogFunction() {}
TransformOpenDialogFunction::~TransformOpenDialogFunction() {}

bool TransformOpenDialogFunction::RunAsync() {
  std::unique_ptr<transform::OpenDialog::Params> params(
      transform::OpenDialog::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  Browser* browser = chrome::FindLastActive();

  if (browser) {
    chrome::ExecuteCommand(browser, IDC_SHOW_DOWNLOADS);
  }

  return true;
}

TransformOpenFileFunction::TransformOpenFileFunction() {}
TransformOpenFileFunction::~TransformOpenFileFunction() {}

bool TransformOpenFileFunction::RunAsync() {
  return true;
}

TransformShowFileInFloderFunction::TransformShowFileInFloderFunction() {}
TransformShowFileInFloderFunction::~TransformShowFileInFloderFunction() {}

bool TransformShowFileInFloderFunction::RunAsync() {
  std::unique_ptr<transform::ShowFileInFloder::Params> params(
      transform::ShowFileInFloder::Params::Create(*args_));
  EXTENSION_FUNCTION_VALIDATE(params.get());

  base::FilePath platform_path(base::UTF8ToWide(params->filename));

  DCHECK(!platform_path.empty());
  Browser* browser = chrome::FindLastActive();
  platform_util::ShowItemInFolder(browser->profile(), platform_path);
  return true;
}

ExtensionTransformEventRouter::ExtensionTransformEventRouter(
    Profile* profile,
    scoped_refptr<const Extension> extension)
    : profile_(profile),
      extension_(extension) {
}

ExtensionTransformEventRouter::~ExtensionTransformEventRouter() {}

void ExtensionTransformEventRouter::SendStartTransform() {
  if (!EventRouter::Get((content::BrowserContext*)profile_))
    return;

  auto event = std::make_unique<extensions::Event>(
      events::START_TRANSFORM,
      transform::OnStartTransform::kEventName,
      transform::OnStartTransform::Create(),
      (content::BrowserContext*)profile_);

  EventRouter::Get((content::BrowserContext*)profile_)->DispatchEventToExtension(extension_->id(), std::move(event));
}

} // namespace api
} // namespace extensions
