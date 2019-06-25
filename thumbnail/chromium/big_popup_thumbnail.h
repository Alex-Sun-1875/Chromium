// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_BIG_POPUP_THUMBNAIL_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_BIG_POPUP_THUMBNAIL_H_

#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "chrome/browser/ui/views/tabs/popup_thumbnail.h"
#include "ui/gfx/animation/animation_delegate.h"
#include "ui/gfx/animation/slide_animation.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/image.h"
#include "ui/views/widget/widget_delegate.h"
#include "ui/views/widget/widget_observer.h"

namespace views {
class ImageView;
class Label;
}

class BigPopupThumbnail
    : public PopupThumbnail,
      public views::WidgetDelegateView,
      public gfx::AnimationDelegate,
      public views::WidgetObserver,
      public base::SupportsWeakPtr<BigPopupThumbnail> {
 public:
  BigPopupThumbnail(Browser* browser);
  ~BigPopupThumbnail() override;

  // Overriden from PopupThumbnail:
  bool IsShowing() override;
  void Show(const Tab* tab,
            const gfx::Image& thumbnail,
            const base::string16& title) override;
  void UpdataThumbnail(const Tab* tab, const gfx::Image& thumbnail) override;
  void UpdataTitle(const Tab* tab, const base::string16& title) override;
  void TabRemoved(const Tab* tab) override;
  void Hide() override;
  void Close() override;

  // Overridden from views::View:
  void OnPaint(gfx::Canvas* canvas) override;

  // Overridden from views::WidgetDelegate:
  views::View* GetContentsView() override;

  // Overridden from gfx::AnimationDelegate:
  void AnimationEnded(const gfx::Animation* animation) override;
  void AnimationProgressed(const gfx::Animation* animation) override;
  void AnimationCanceled(const gfx::Animation* animation) override;

  // Overridden from views::WidgetObserver:
  void OnWidgetClosing(views::Widget* widget) override;
  void OnWidgetVisibilityChanged(views::Widget* widget, bool visible) override;

 private:
  class TabState;

  // If |adjust| is true, the coresponding |TabState| will be created if not
  // exists and stored in the last of |tabs_showing_order_|.
  TabState* GetTabState(const Tab* tab, bool adjust);
  void RemoveTabState(const Tab* tab);

  gfx::Rect CalculateWidgetBounds();

  gfx::Rect CalculateTabThumbnailBounds();

  // Convinient methods for TabState.
  void SchedulePaintForTab();
  void HideIfNothingToShow();

  gfx::SlideAnimation animation_;

  bool observing_;

  std::map<const Tab*, std::unique_ptr<TabState>> tab_states_;
  std::vector<const Tab*> tabs_showing_order_;

  DISALLOW_COPY_AND_ASSIGN(BigPopupThumbnail);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_BIG_POPUP_THUMBNAIL_H_
