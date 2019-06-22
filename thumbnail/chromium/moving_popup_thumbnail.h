// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_MOVING_POPUP_THUMBNAIL_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_MOVING_POPUP_THUMBNAIL_H_

#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "chrome/browser/ui/views/tabs/popup_thumbnail.h"
#include "ui/gfx/font_list.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/image/image.h"
#include "ui/views/widget/widget_delegate.h"

namespace views {
class ImageView;
class Label;
}

class MovingPopupThumbnail
    : public PopupThumbnail,
      public views::WidgetDelegateView,
      public base::SupportsWeakPtr<MovingPopupThumbnail> {
 public:
  MovingPopupThumbnail(Browser* browser);
  ~MovingPopupThumbnail() override;

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

 private:
  class TabState;

  // If |adjust| is true, the coresponding |TabState| will be created if not
  // exists and stored in the last of |tabs_showing_order_|.
  TabState* GetTabState(const Tab* tab, bool adjust);
  void RemoveTabState(const Tab* tab);

  gfx::Rect CalculateWidgetBounds(const Tab* tab);

  gfx::Rect CalculateTabThumbnailBounds(const Tab* tab);

  // Convinient methods for TabState.
  gfx::FontList& font_list() { return font_list_; }
  int GetTiTleHeightForWidth(const gfx::FontList& font_list,
                             const base::string16& title,
                             int width);
  void SchedulePaintForTab(const Tab* tab);
  void HideIfNothingToShow();

  gfx::FontList font_list_;

  // Used to measure the size of title.
  views::Label* label_;

  std::map<const Tab*, std::unique_ptr<TabState>> tab_states_;
  std::vector<const Tab*> tabs_showing_order_;

  DISALLOW_COPY_AND_ASSIGN(MovingPopupThumbnail);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_MOVING_POPUP_THUMBNAIL_H_
