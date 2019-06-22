// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved.

#include "chrome/browser/ui/views/tabs/moving_popup_thumbnail.h"

#include <stdint.h>

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

const int kThumbnailWidth = 230;
const int kMaxThumbnailHeight = 270;

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

bool is_float_equal(float f1, float f2) {
  return static_cast<int64_t>(f1 * 10000) == static_cast<int64_t>(f2 * 10000);
}

}

/////////////////// MovingPopupThumbnail::TabState ////////////////////////////

class MovingPopupThumbnail::TabState : public gfx::AnimationDelegate {
 public:
  TabState(MovingPopupThumbnail* owner, const Tab* tab);
  ~TabState() override;

  void Show();
  void Hide();
  void Close();

  bool IsShowing();

  bool IsFullyHidden();

  void PaintTabThumbnail(gfx::Canvas* canvas);

  const Tab* tab() const { return tab_; }

  void set_bounds(const gfx::Rect& bounds) { bounds_ = bounds; }

  void SetThumbnail(const gfx::Image& thumbnail) {
    thumbnail_ = thumbnail;
    need_update_ = true;
  }

  void SetTitle(const base::string16& title) {
    if (title_ == title)
      return;
    title_ = title;
    need_update_ = true;
  }

  // Overriden from gfx::AnimationDelegate:
  void AnimationEnded(const gfx::Animation* animation) override;
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationCanceled(const gfx::Animation* animation) override;

 private:
  void UpdatePaintedImage(float scale);
  gfx::ImageSkia GeneratePaintedImage(const gfx::Image& thumbnail,
                                      const base::string16& title,
                                      const gfx::Size& available_size,
                                      float scale);

  MovingPopupThumbnail* owner_;
  const Tab* const tab_;

  gfx::Image thumbnail_;
  base::string16 title_;

  bool need_update_;
  float last_painted_scale_;

  gfx::Rect bounds_;

  gfx::ImageSkia title_image_;
  gfx::ImageSkia thumbnail_title_image_;

  gfx::SlideAnimation animation_;

  DISALLOW_COPY_AND_ASSIGN(TabState);
};

MovingPopupThumbnail::TabState::TabState(MovingPopupThumbnail* owner,
                                         const Tab* tab)
    : owner_(owner),
      tab_(tab),
      need_update_(false),
      last_painted_scale_(-1.f),
      animation_(this) {
  animation_.SetSlideDuration(250);
}
MovingPopupThumbnail::TabState::~TabState() {
}

void MovingPopupThumbnail::TabState::Show() {
  animation_.Show();
}
void MovingPopupThumbnail::TabState::Hide() {
  animation_.Hide();
}
void MovingPopupThumbnail::TabState::Close() {
  animation_.Stop();
}
bool MovingPopupThumbnail::TabState::IsShowing() {
  return animation_.IsShowing();
}

bool MovingPopupThumbnail::TabState::IsFullyHidden() {
  return !animation_.IsShowing() && !animation_.IsClosing();
}

void MovingPopupThumbnail::TabState::PaintTabThumbnail(gfx::Canvas* canvas) {
  if (IsFullyHidden())
    return;

  float scale = 1.f;
  if (owner_->GetWidget() && owner_->GetWidget()->GetCompositor())
    scale = owner_->GetWidget()->GetCompositor()->device_scale_factor();
  if (scale <= 0.1f)
    scale = 1.f;

  UpdatePaintedImage(scale);

  gfx::ImageSkia* image =
      (!tab_->IsActive() && !thumbnail_title_image_.isNull())
          ? &thumbnail_title_image_
          : &title_image_;
  if (!image || image->isNull()) {
    NOTREACHED();
    return;
  }

  // Modified by sunlh:
  // For tabstrip location selction.
  // When tabstrip in bottom the thumnail should show up.
  int image_height = image->height() / scale;
  double y = tab_->IsTabstripOnBottom()
             ? kMaxThumbnailHeight - image_height * animation_.GetCurrentValue()
             : image_height * (animation_.GetCurrentValue() - 1);

  cc::PaintFlags flags;
  flags.setAntiAlias(false);
  flags.setStyle(cc::PaintFlags::kFill_Style);
  flags.setColor(SkColorSetARGB(animation_.GetCurrentValue() * 255, 0, 0, 0));
  canvas->DrawImageInt(*image, 0, 0, image->width(), image->height(),
                       bounds_.x(), y,
                       image->width() / scale, image->height() / scale,
                       false);
}

// Overriden from gfx::AnimationDelegate:
void MovingPopupThumbnail::TabState::AnimationEnded(
    const gfx::Animation* animation) {
  if (animation == &animation_) {
    owner_->SchedulePaintForTab(tab_);

    owner_->HideIfNothingToShow();
  }
}
void MovingPopupThumbnail::TabState::AnimationProgressed(
    const gfx::Animation* animation) {
  if (animation == &animation_) {
    owner_->SchedulePaintForTab(tab_);
  }
}
void MovingPopupThumbnail::TabState::AnimationCanceled(
    const gfx::Animation* animation) {
  if (animation == &animation_) {
    owner_->SchedulePaintForTab(tab_);

    owner_->HideIfNothingToShow();
  }
}

void MovingPopupThumbnail::TabState::UpdatePaintedImage(float scale) {
  if (!need_update_ && is_float_equal(last_painted_scale_, scale))
    return;

  const gfx::Size scaled_size(bounds_.width() * scale, bounds_.height() * scale);

  title_image_ = GeneratePaintedImage(
      gfx::Image(),
      !title_.empty() ? title_
                      : l10n_util::GetStringUTF16(IDS_TAB_LOADING_TITLE),
      scaled_size,
      scale);
  if (thumbnail_.IsEmpty()) {
    if (!thumbnail_title_image_.isNull())
      thumbnail_title_image_ = gfx::ImageSkia();
  } else {
    thumbnail_title_image_ = GeneratePaintedImage(thumbnail_, title_,
                                                  scaled_size,
                                                  scale);
  }
  need_update_ = false;
  last_painted_scale_ = scale;
}
gfx::ImageSkia MovingPopupThumbnail::TabState::GeneratePaintedImage(
    const gfx::Image& thumbnail,
    const base::string16& title,
    const gfx::Size& available_size,
    float scale) {
  gfx::Rect painted_bitmap_size(available_size);
  gfx::Rect content_bounds(painted_bitmap_size);
  content_bounds.Inset(kShadowBorderInsets);

  gfx::Rect image_bounds = content_bounds;
  image_bounds.set_height(
      get_image_height_for_width(thumbnail, image_bounds.width()));

  gfx::FontList font_list = owner_->font_list();
  if (!is_float_equal(scale, 1.f))
    font_list = font_list.DeriveWithSizeDelta(
                    font_list.GetFontSize() * (scale - 1.f));

  gfx::Rect title_bounds = content_bounds;
  title_bounds.set_y(image_bounds.bottom());
  title_bounds.Inset(kTitleInsets);
  title_bounds.set_height(std::min(
      static_cast<int>(thumbnail.IsEmpty() ? 170 * scale : 93 * scale),
      owner_->GetTiTleHeightForWidth(font_list, title_, title_bounds.width())));

  painted_bitmap_size.set_height(
      title_bounds.bottom() + kShadowBorderInsets.bottom() +
      kTitleInsets.bottom());

  SkBitmap bitmap;
  bitmap.allocPixels(SkImageInfo::MakeN32Premul(
      painted_bitmap_size.width(), painted_bitmap_size.height()));
  bitmap.eraseColor(SkColorSetARGB(0, 0, 0, 0));

  /// gmbrowser {
  /// yfj
  cc::SkiaPaintCanvas pcanvas(new SkCanvas(bitmap));
  gfx::Canvas canvas(&pcanvas, 1);
  ///}
  // Paint shadow.
  auto& rb = ui::ResourceBundle::GetSharedInstance();
  gfx::ImageSkia* pts_top_left     = rb.GetImageSkiaNamed(IDR_PTS_TOP_LEFT);
  gfx::ImageSkia* pts_top          = rb.GetImageSkiaNamed(IDR_PTS_TOP);
  gfx::ImageSkia* pts_top_right    = rb.GetImageSkiaNamed(IDR_PTS_TOP_RIGHT);
  gfx::ImageSkia* pts_right        = rb.GetImageSkiaNamed(IDR_PTS_RIGHT);
  gfx::ImageSkia* pts_bottom_right = rb.GetImageSkiaNamed(IDR_PTS_BOTTOM_RIGHT);
  gfx::ImageSkia* pts_bottom       = rb.GetImageSkiaNamed(IDR_PTS_BOTTOM);
  gfx::ImageSkia* pts_bottom_left  = rb.GetImageSkiaNamed(IDR_PTS_BOTTOM_LEFT);
  gfx::ImageSkia* pts_left         = rb.GetImageSkiaNamed(IDR_PTS_LEFT);

  canvas.DrawImageInt(*pts_top_left, 0, 0);
  int top_width = painted_bitmap_size.width() - pts_top_left->width() -
                      pts_top_right->width();
  canvas.DrawImageInt(*pts_top, 0, 0, pts_top->width(), pts_top->height(),
      pts_top_left->width(), 0, top_width, pts_top->height(), true);
  canvas.DrawImageInt(*pts_top_right,
      painted_bitmap_size.width() - pts_top_right->width(), 0);
  int right_height = painted_bitmap_size.height() - pts_top_right->height() -
                         pts_bottom_right->height();
  canvas.DrawImageInt(*pts_right, 0, 0, pts_right->width(), pts_right->height(),
      painted_bitmap_size.width() - pts_right->width(), pts_top_right->height(),
      pts_right->width(), right_height, true);
  canvas.DrawImageInt(*pts_bottom_right,
      painted_bitmap_size.width() - pts_bottom_right->width(),
      painted_bitmap_size.height() - pts_bottom_right->height());
  int bottom_width = painted_bitmap_size.width() - pts_bottom_left->width() -
                         pts_bottom_right->width();
  canvas.DrawImageInt(*pts_bottom,
      0, 0, pts_bottom->width(), pts_bottom->height(), pts_bottom_left->width(),
      painted_bitmap_size.height() - pts_bottom->height(), bottom_width,
      pts_bottom->height(), true);
  canvas.DrawImageInt(*pts_bottom_left,
                      0, painted_bitmap_size.height() - pts_bottom->height());
  int left_height = painted_bitmap_size.height() - pts_top_left->height() -
                        pts_bottom_left->height();
  canvas.DrawImageInt(*pts_left, 0, 0, pts_left->width(), pts_left->height(),
      0, pts_top_left->height(), pts_left->width(), left_height, true);

  if (!thumbnail.IsEmpty()) {
    canvas.DrawImageInt(thumbnail.AsImageSkia(),
        0, 0, thumbnail.Width(), thumbnail.Height(),
        image_bounds.x(), image_bounds.y(), image_bounds.width(),
        image_bounds.height(), true);
  }
  gfx::Rect wrap_title_bounds = title_bounds;
  wrap_title_bounds.Inset(-kTitleInsets);
  canvas.FillRect(wrap_title_bounds, SK_ColorWHITE);
  canvas.DrawStringRectWithFlags(title_, font_list, SK_ColorBLACK,
                                 title_bounds, gfx::Canvas::MULTI_LINE);

  return gfx::ImageSkia::CreateFrom1xBitmap(bitmap);
}

//////////////////////// MovingPopupThumbnail /////////////////////////////////

MovingPopupThumbnail::MovingPopupThumbnail(Browser* browser)
    : PopupThumbnail(PopupThumbnail::PT_MOVING, browser),
      font_list_(
          ui::ResourceBundle::GetSharedInstance().GetFontListWithDelta(2)) {
  set_can_activate(false);

  label_ = new views::Label();
  label_->SetHorizontalAlignment(gfx::ALIGN_LEFT);
  label_->SetLineHeight(20);
  label_->SetMultiLine(true);
}
MovingPopupThumbnail::~MovingPopupThumbnail() {
}

// Overriden from PopupThumbnail:
bool MovingPopupThumbnail::IsShowing() {
  return GetWidget() && GetWidget()->IsVisible();
}
void MovingPopupThumbnail::Show(const Tab* tab,
                                const gfx::Image& thumbnail,
                                const base::string16& title) {
  DCHECK(tab);
  TabState* tab_state = GetTabState(tab, true);
  tab_state->SetThumbnail(thumbnail);
  tab_state->SetTitle(title);
  tab_state->set_bounds(CalculateTabThumbnailBounds(tab));

  gfx::Rect bounds = CalculateWidgetBounds(tab);
  Widget* widget = GetWidget();
  if (!widget)
    widget = create_widget(this);

  widget->SetBounds(bounds);
  widget->ShowInactive();
  widget->StackAbove(browser()->window()->GetNativeWindow());
  SchedulePaint();

  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    if (it->second.get() == tab_state) {
      tab_state->Show();
      continue;
    }

    auto* item = it->second.get();
    if (item->IsShowing())
      item->Hide();
  }
}
void MovingPopupThumbnail::UpdataThumbnail(const Tab* tab,
                                           const gfx::Image& thumbnail) {
  TabState* tab_state = GetTabState(tab, false);
  if (!tab_state)
    return;
  tab_state->SetThumbnail(thumbnail);
  SchedulePaintForTab(tab);
}
void MovingPopupThumbnail::UpdataTitle(const Tab* tab,
                                       const base::string16& title) {
  TabState* tab_state = GetTabState(tab, false);
  if (!tab_state)
    return;
  tab_state->SetTitle(title);
  SchedulePaintForTab(tab);
}
void MovingPopupThumbnail::TabRemoved(const Tab* tab) {
  TabState* tab_state = GetTabState(tab, false);
  if (!tab_state)
    return;

  if (!tab_state->IsFullyHidden())
    SchedulePaintForTab(tab);

  RemoveTabState(tab);
}

void MovingPopupThumbnail::Hide() {
  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    if (it->second->IsShowing())
      it->second->Hide();
  }

  HideIfNothingToShow();
}
void MovingPopupThumbnail::Close() {
  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    it->second->Close();
  }

  Widget* widget = GetWidget();
  if (widget) {
    widget->Close();
  }
}

// Overridden from views::View:
void MovingPopupThumbnail::OnPaint(gfx::Canvas* canvas) {
  /// gmbrowser {
  /// yfj
  canvas->DrawColor(SK_ColorTRANSPARENT, SkBlendMode::kSrc);
  ///}
  if (tabs_showing_order_.empty())
   return;

  for (const Tab* tab : tabs_showing_order_) {
   auto it = tab_states_.find(tab);
   if (it == tab_states_.end()) {
     NOTREACHED();
     continue;
   }
   it->second->PaintTabThumbnail(canvas);
  }
}

// Overridden from views::WidgetDelegate:
views::View* MovingPopupThumbnail::GetContentsView() {
  return this;
}

MovingPopupThumbnail::TabState* MovingPopupThumbnail::GetTabState(
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

void MovingPopupThumbnail::RemoveTabState(const Tab* tab) {
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

gfx::Rect MovingPopupThumbnail::CalculateWidgetBounds(const Tab* tab) {
  gfx::Rect browser_bounds = browser()->window()->GetBounds();
  if (tab->IsTabstripOnBottom()) {
    return gfx::Rect(browser_bounds.x(),
                     tab->GetBoundsInScreen().y() - kMaxThumbnailHeight,
                     browser_bounds.width(), kMaxThumbnailHeight);
  }
  return gfx::Rect(browser_bounds.x(), tab->GetBoundsInScreen().bottom() + 3,
                   browser_bounds.width(), kMaxThumbnailHeight);
}

gfx::Rect MovingPopupThumbnail::CalculateTabThumbnailBounds(const Tab* tab) {
  gfx::Rect tab_bounds = tab->GetBoundsInScreen();

  const int x = tab_bounds.x() - (kThumbnailWidth - tab_bounds.width()) / 2;
  gfx::Rect bounds(x, 0, kThumbnailWidth, height());
  bounds.Inset(-kShadowBorderInsets.left(), 0,
               -kShadowBorderInsets.right(), 0);

  // Convert to this view's coordinate.
  gfx::Rect this_bounds = GetWidget() && GetWidget()->IsVisible()
                              ? GetBoundsInScreen()
                              : CalculateWidgetBounds(tab);
  bounds.set_x(bounds.x() - this_bounds.x());

  if (bounds.x() < 0)
    bounds.set_x(0);
  if (bounds.right() > this_bounds.right())
    bounds.set_x(this_bounds.right() - bounds.width());

  bounds.set_y(0);
  bounds.set_height(this_bounds.height());
  return bounds;
}

int MovingPopupThumbnail::GetTiTleHeightForWidth(const gfx::FontList& font_list,
                                                 const base::string16& title,
                                                 int width) {
  label_->SetFontList(font_list);
  if (label_->text() != title)
    label_->SetText(title);
  return label_->GetHeightForWidth(width);
}

void MovingPopupThumbnail::SchedulePaintForTab(const Tab* tab) {
  SchedulePaintInRect(CalculateTabThumbnailBounds(tab));
}

void MovingPopupThumbnail::HideIfNothingToShow() {
  for (auto it = tab_states_.begin(); it != tab_states_.end(); ++it) {
    if (!it->second->IsFullyHidden())
      return;
  }

  Widget* widget = GetWidget();
  if (widget) {
    widget->Hide();
  }
}