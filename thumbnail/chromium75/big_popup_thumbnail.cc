// Copyright 2019 The WCT(Wisdom City Times) Authors. All rights reserved.

#include "chrome/browser/ui/views/tabs/big_popup_thumbnail.h"

#include "cc/paint/paint_flags.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/browser/ui/views/tabs/tab.h"
#include "chrome/grit/generated_resources.h"
#include "chrome/grit/theme_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/gfx/canvas.h"
#include "ui/views/controls/label.h"
#include "ui/views/controls/image_view.h"
#include "ui/views/widget/widget.h"

using views::Widget;

namespace {

const int kThumbnailWidth = 460;
const int kThumbnailHeight = 305;

const gfx::Insets kTitleInsets(3, 5);

// Shadow and border.
const gfx::Insets kShadowBorderInsets(3, 5, 7, 5);

Widget* create_widget(views::WidgetDelegate* view_delegate) {
  Widget* widget = new Widget();

  Widget::InitParams init_params(views::Widget::InitParams::TYPE_POPUP);
  init_params.delegate = view_delegate;
  init_params.opacity = Widget::InitParams::TRANSLUCENT_WINDOW;
  init_params.accept_events = false;
  init_params.keep_on_top = true;
  init_params.activatable = Widget::InitParams::ACTIVATABLE_NO;
  init_params.shadow_type = Widget::InitParams::SHADOW_TYPE_NONE;
  init_params.force_show_in_taskbar = true;

  widget->Init(init_params);
  widget->SetVisibilityChangedAnimationsEnabled(false);

  return widget;
}

int get_image_height_for_width(const gfx::Image& image, int width) {
  if (image.Width() < 1)
    return 0;
  return static_cast<float>(image.Height()) / image.Width() * width;
}

}

/********************* BigPopupThumbnail::TabState *********************/

class BigPopupThumbnail::TabState : public gfx::AnimationDelegate {
 public:
  TabState(BigPopupThumbnail* owner, const Tab* tab);
  ~TabState() override;

  void Show();
  void Hide();
  void Close();

  bool IsShowing();
  bool IsFullyHidden();
  bool CanShow();
  void PaintTabThumbnail(gfx::Canvas* canvas, const gfx::Rect& bounds);

  const Tab* tab() const { return tab_; }
  gfx::Image thumbnail() const { return thumbnail_; }

  void SetThumbnail(const gfx::Image& thumbnail) {
    thumbnail_ = thumbnail;
  }
  void SetTitle(const base::string16& title) {
    if (title_ == title)
      return;
    title_ = title;
  }

  // Overriden from gfx::AnimationDelegate
  void AnimationEnded(const gfx::Animation* animation) override;
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationCanceled(const gfx::Animation* animation) override;

 private:
  void UpdatePaintedBitmap();
  SkBitmap GeneratePaintedBitmap(const gfx::Image& thumbnail,
                                 const base::string16& title,
                                 const gfx::Size& available_size);

  BigPopupThumbnail* owner_;
  const Tab* const tab_;

  gfx::Image thumbnail_;
  base::string16 title_;

  gfx::SlideAnimation animation_;

  DISALLOW_COPY_AND_ASSIGN(TabState);
};

BigPopupThumbnail::TabState::TabState(BigPopupThumbnail* owner, const Tab* tab)
    : owner_(owner), tab_(tab), animation_(this) {
  animation_.SetSlideDuration(250);
}

BigPopupThumbnail::TabState::~TabState() {}

void BigPopupThumbnail::TabState::Show() {
  animation_.Show();
}

void BigPopupThumbnail::TabState::Hide() {
  animation_.Hide();
}

void BigPopupThumbnail::TabState::Close() {
  animation_.Stop();
}

bool BigPopupThumbnail::TabState::IsShowing() {
  return animation_.IsShowing();
}

bool BigPopupThumbnail::TabState::IsFullyHidden() {
  return !animation_.IsShowing() && !animation_.IsClosing();
}

bool BigPopupThumbnail::TabState::CanShow() {
  return !tab_->IsActive();
}

void BigPopupThumbnail::TabState::PaintTabThumbnail(gfx::Canvas* canvas,
                                                    const gfx::Rect& bounds) {
  if (IsFullyHidden() || bounds.IsEmpty())
    return;
  if (!thumbnail_.IsEmpty()) {
    cc::PaintFlags flags;
    flags.setAntiAlias(false);
    flags.setStyle(cc::PaintFlags::kFill_Style);
    flags.setColor(SkColorSetARGB(animation_.GetCurrentValue() * 255, 0, 0, 0));
    canvas->DrawImageInt(thumbnail_.AsImageSkia(), 0, 0, thumbnail_.Width(),
                         thumbnail_.Height(),bounds.x(), bounds.y(), bounds.width(),
                         bounds.height(), false, flags);
    return;
  }

  int alpha = static_cast<int>(animation_.GetCurrentValue() * 150);
  canvas->FillRect(bounds, SkColorSetARGB(alpha, 0, 0, 0));

  const int tb_width = kThumbnailWidth;
  int font_delta = static_cast<int>(bounds.width() - tb_width) / 100 * 3;
  gfx::FontList font_list(ui::ResourceBundle::GetSharedInstance().GetFontListWithDelta(font_delta));
  gfx::Rect loading_bounds = bounds;
  loading_bounds.set_y(bounds.y() + 5);
  loading_bounds.Inset(gfx::Insets(20));
  loading_bounds.set_height(font_list.GetHeight());
  canvas->DrawStringRectWithFlags(
      l10n_util::GetStringUTF16(IDS_TAB_LOADING_TITLE),
      font_list, SK_ColorWHITE, loading_bounds,
      gfx::Canvas::TEXT_ALIGN_CENTER | gfx::Canvas::NO_SUBPIXEL_RENDERING);
                        
  gfx::Rect title_bounds = bounds;
  title_bounds.set_y(loading_bounds.bottom() + 10);
  title_bounds.Inset(gfx::Insets(20));
  title_bounds.set_height(std::max(0, bounds.bottom() - title_bounds.y()));
  canvas->DrawStringRectWithFlags(
      title_, font_list, SK_ColorWHITE, title_bounds,
      gfx::Canvas::TEXT_ALIGN_CENTER | gfx::Canvas::NO_SUBPIXEL_RENDERING);
}

// Overriden from gfx::AnimationDelegate.
void BigPopupThumbnail::TabState::AnimationEnded(
    const gfx::Animation* animation) {
  if (animation == &animation_) {
    owner_->SchedulePaintForTab();
    owner_->HideIfNothingToShow();
  }
}

void BigPopupThumbnail::TabState::AnimationProgressed(
    const gfx::Animation* animation) {
  if (animation == &animation_) {
    owner_->SchedulePaintForTab();
  }
}

void BigPopupThumbnail::TabState::AnimationCanceled(
    const gfx::Animation* animation) {
  if (animation == &animation_) {
    owner_->SchedulePaintForTab();
    owner_->HideIfNothingToShow();
  }
}

/**************************** BigPopupThumbnail ***************************/

BigPopupThumbnail::BigPopupThumbnail(Browser* browser)
    : PopupThumbnail(PopupThumbnail::PT_BIG, browser),
      animation_(this),
      observing_(false) {
  set_can_activate(false);
}

BigPopupThumbnail::~BigPopupThumbnail() {
  if (observing_ && GetWidget()) {
    GetWidget()->RemoveObserver(this);
    observing_ = false;
  }
}

// Overriden from PopupThumbnail.
bool BigPopupThumbnail::IsShowing() {
  return GetWidget() && GetWidget()->IsVisible();
}