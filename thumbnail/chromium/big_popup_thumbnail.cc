// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved.

#include "chrome/browser/ui/views/tabs/big_popup_thumbnail.h"

/// gmbrowser {
/// yfj
#include "cc/paint/paint_flags.h"
///}
#include "chrome/browser/thumbnails/thumbnail_service.h"
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

const int kThumbnailWidth = 220;
const int kMaxThumbnailHeight = 250;

const gfx::Insets kTitleInsets(3, 5);
// shadow and border
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
  init_params.force_software_compositing = true;

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

/////////////////// BigPopupThumbnail::TabState ////////////////////////////

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

  // Overriden from gfx::AnimationDelegate:
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

BigPopupThumbnail::TabState::TabState(BigPopupThumbnail* owner,
                                         const Tab* tab)
    : owner_(owner),
      tab_(tab),
      animation_(this) {
  animation_.SetSlideDuration(250);
}
BigPopupThumbnail::TabState::~TabState() {
}

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
    /// gmbrowser {
    /// yfj erro
    cc::PaintFlags flags;
    flags.setAntiAlias(false);
    flags.setStyle(cc::PaintFlags::kFill_Style);
    flags.setColor(SkColorSetARGB(animation_.GetCurrentValue() * 255, 0, 0, 0));
    canvas->DrawImageInt(thumbnail_.AsImageSkia(),
      0, 0, thumbnail_.Width(), thumbnail_.Height(),
      bounds.x(), bounds.y(), bounds.width(), bounds.height(), false, flags);
    ///}
    return;
  }

  int alpha = static_cast<int>(animation_.GetCurrentValue() * 150);
  canvas->FillRect(bounds, SkColorSetARGB(alpha, 0, 0, 0));

  const int tb_width = thumbnails::ThumbnailService::kThumbnailWidth;
  int font_delta = static_cast<double>(bounds.width() - tb_width) / 100 * 3;
  font_delta = std::min(std::max(font_delta, 2), 15);
  gfx::FontList font_list(ui::ResourceBundle::GetSharedInstance()
                              .GetFontListWithDelta(font_delta));
  gfx::Rect loading_bounds = bounds;
  loading_bounds.set_y(bounds.y() + 5);
  loading_bounds.Inset(gfx::Insets(20));
  loading_bounds.set_height(font_list.GetHeight());
  canvas->DrawStringRectWithFlags(
      l10n_util::GetStringUTF16(IDS_TAB_LOADING_TITLE),
      font_list, SK_ColorWHITE, loading_bounds,
      gfx::Canvas::TEXT_ALIGN_CENTER | gfx::Canvas::NO_SUBPIXEL_RENDERING);

  gfx::Rect title_bounds = bounds;
  title_bounds.set_y(loading_bounds.bottom() + 10 );
  title_bounds.Inset(gfx::Insets(20));
  title_bounds.set_height(std::max(0, bounds.bottom() - title_bounds.y()));
  canvas->DrawStringRectWithFlags(
      title_, font_list, SK_ColorWHITE, title_bounds,
      gfx::Canvas::MULTI_LINE | gfx::Canvas::NO_SUBPIXEL_RENDERING);
}

// Overriden from gfx::AnimationDelegate:
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

//////////////////////// BigPopupThumbnail /////////////////////////////////

BigPopupThumbnail::BigPopupThumbnail(Browser* browser)
    : PopupThumbnail(PopupThumbnail::PT_MOVING, browser),
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

// Overriden from PopupThumbnail:
bool BigPopupThumbnail::IsShowing() {
  return GetWidget() && GetWidget()->IsVisible();
}
void BigPopupThumbnail::Show(const Tab* tab,
                             const gfx::Image& thumbnail,
                             const base::string16& title) {
  if (tab->IsActive()) {
    Hide();
    return;
  }

  TabState* tab_state = GetTabState(tab, true);
  tab_state->SetThumbnail(thumbnail);
  tab_state->SetTitle(title);

  if (!tab_state->CanShow()) {
    Hide();
    return;
  }

  gfx::Rect bounds = CalculateWidgetBounds();
  Widget* widget = GetWidget();
  if (!widget) {
    widget = create_widget(this);
    widget->AddObserver(this);
    observing_ = true;
  }

  widget->SetBounds(bounds);
  widget->ShowInactive();
  widget->StackAbove(browser()->window()->GetNativeWindow());
  animation_.Show();

  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    if (it->second.get() == tab_state) {
      tab_state->Show();
      continue;
    }

    auto item = it->second.get();
    if (item->IsShowing())
      item->Hide();
  }
}
void BigPopupThumbnail::UpdataThumbnail(const Tab* tab,
                                        const gfx::Image& thumbnail) {
  TabState* tab_state = GetTabState(tab, false);
  if (!tab_state)
    return;

  tab_state->SetThumbnail(thumbnail);
  SchedulePaintForTab();
}
void BigPopupThumbnail::UpdataTitle(const Tab* tab,
                                    const base::string16& title) {
  TabState* tab_state = GetTabState(tab, false);
  if (!tab_state)
    return;
  if (!tab_state->thumbnail().IsEmpty())
    return;

  tab_state->SetTitle(title);
  SchedulePaintForTab();
}
void BigPopupThumbnail::TabRemoved(const Tab* tab) {
  TabState* tab_state = GetTabState(tab, false);
  if (!tab_state)
    return;

  if (!tab_state->IsFullyHidden())
    SchedulePaintForTab();

  RemoveTabState(tab);
}

void BigPopupThumbnail::Hide() {
  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    if (it->second->IsShowing())
      it->second->Hide();
  }

  HideIfNothingToShow();
}
void BigPopupThumbnail::Close() {
  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    it->second->Close();
  }

  Widget* widget = GetWidget();
  if (widget) {
    widget->Close();
  }
}

// Overridden from views::View:
void BigPopupThumbnail::OnPaint(gfx::Canvas* canvas) {
  int alpha = static_cast<int>(animation_.GetCurrentValue() * 40);
  /// gmbrowser {
  /// yfj
  canvas->DrawColor(SkColorSetARGB(alpha, 0, 0, 0),
    SkBlendMode::kSrc);
  ///}
  if (tabs_showing_order_.empty())
   return;

  gfx::Rect thumbnail_bounds = CalculateTabThumbnailBounds();

  for (const Tab* tab : tabs_showing_order_) {
   auto it = tab_states_.find(tab);
   if (it == tab_states_.end()) {
     NOTREACHED();
     continue;
   }
   it->second->PaintTabThumbnail(canvas, thumbnail_bounds);
  }
}

// Overridden from views::WidgetDelegate:
views::View* BigPopupThumbnail::GetContentsView() {
  return this;
}

// Overridden from
void BigPopupThumbnail::AnimationEnded(const gfx::Animation* animation) {
  DCHECK(animation == &animation_);
  SchedulePaint();
  if (animation_.GetCurrentValue() != 1) {
    if (GetWidget())
      GetWidget()->Hide();
  }
}
void BigPopupThumbnail::AnimationProgressed(const gfx::Animation* animation) {
  DCHECK(animation == &animation_);
  SchedulePaint();
}
void BigPopupThumbnail::AnimationCanceled(const gfx::Animation* animation) {
  DCHECK(animation == &animation_);
  if (GetWidget())
    GetWidget()->Hide();
}

// Overridden from views::WidgetObserver:
void BigPopupThumbnail::OnWidgetClosing(views::Widget* widget) {
  DCHECK(widget == GetWidget());
  if (observing_) {
    widget->RemoveObserver(this);
    observing_ = false;
  }
}
void BigPopupThumbnail::OnWidgetVisibilityChanged(views::Widget* widget,
                                                  bool visible) {
  DCHECK(widget == GetWidget());
  if (visible) {
    widget->StackAbove(browser()->window()->GetNativeWindow());
  }
}

BigPopupThumbnail::TabState* BigPopupThumbnail::GetTabState(
    const Tab* tab, bool adjust) {
  auto it = std::find(tabs_showing_order_.begin(),
                      tabs_showing_order_.end(),
                      tab);
  if (!adjust) {
    return it != tabs_showing_order_.end() ? tab_states_[tab].get() : nullptr;
  }

  if (it == tabs_showing_order_.end()) {
    tabs_showing_order_.push_back(tab);
    tab_states_[tab] = base::MakeUnique<TabState>(this, tab);
    return tab_states_[tab].get();
  }

  tabs_showing_order_.erase(it);
  tabs_showing_order_.push_back(tab);
  return tab_states_[tab].get();
}

void BigPopupThumbnail::RemoveTabState(const Tab* tab) {
  auto it = std::find(tabs_showing_order_.begin(),
                      tabs_showing_order_.end(),
                      tab);
  if (it == tabs_showing_order_.end())
    return;
  tabs_showing_order_.erase(it);

  auto state_it = tab_states_.find(tab);
  if (state_it == tab_states_.end()) {
    NOTREACHED();
    return;
  }
  tab_states_.erase(state_it);

  HideIfNothingToShow();
}

gfx::Rect BigPopupThumbnail::CalculateWidgetBounds() {
  /// gmbrowser {
  ///yfj erro
  return browser()->window()->GetRenderViewBoundsInScreen();
  //return gfx::Rect(0, 0, 100, 100);
  ///}
}

gfx::Rect BigPopupThumbnail::CalculateTabThumbnailBounds() {
  gfx::Rect view_bounds = GetWidget() && GetWidget()->IsVisible()
                              ? GetBoundsInScreen()
                              : CalculateWidgetBounds();
  if (view_bounds.IsEmpty()) {
    NOTREACHED();
    return gfx::Rect();
  }
  const int kMaxScale = 2;
  const int tb_width = thumbnails::ThumbnailService::kThumbnailWidth;
  const int tb_height = thumbnails::ThumbnailService::kThumbnailHeight;
  double tb_h_by_w = static_cast<double>(tb_height) / tb_width;
  double vb_h_by_w =
      static_cast<double>(view_bounds.height()) / view_bounds.width();
  gfx::Size tb_size;
  if (tb_h_by_w < vb_h_by_w) {
    tb_size.set_width(std::min(static_cast<int>(0.8 * view_bounds.width()),
                               kMaxScale * tb_width));
    tb_size.set_height(tb_size.width() * tb_h_by_w);
  } else {
    tb_size.set_height(std::min(static_cast<int>(0.8 * view_bounds.height()),
                                kMaxScale * tb_height));
    tb_size.set_width(tb_size.height() / tb_h_by_w);
  }
  return gfx::Rect((view_bounds.width() - tb_size.width()) / 2,
                   (view_bounds.height() - tb_size.height()) / 3,
                    tb_size.width(), tb_size.height());
}

void BigPopupThumbnail::SchedulePaintForTab() {
  SchedulePaintInRect(CalculateTabThumbnailBounds());
}

void BigPopupThumbnail::HideIfNothingToShow() {
  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    if (!it->second->IsFullyHidden())
      return;
  }

  Widget* widget = GetWidget();
  if (widget) {
    widget->Hide();
  }
}