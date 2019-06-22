// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved.

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_TAB_PREVIEW_CONTROLLER_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_TAB_PREVIEW_CONTROLLER_H_

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/timer/timer.h"
#include "chrome/browser/thumbnails/thumbnail_service.h"
#include "chrome/browser/ui/views/tabs/tab_preview_controller.h"
#include "chrome/browser/ui/views/tabs/tab_strip_observer.h"
#include "components/prefs/pref_member.h"
#include "ui/gfx/geometry/rect.h"

class Browser;
class PopupThumbnail;
class Tab;
class TabStrip;

class TabPreviewController
    : public thumbnails::ThumbnailService::Observer {
 public:
  TabPreviewController(Browser* browser, TabStrip* tab_strip);
  ~TabPreviewController() override;

  bool ShouldPreview() const;

  // |tab| can be nullptr meaning preview should hide.
  void UpdatePreviewForTab(Tab* tab);
  void TabRemoving(Tab* tab);
  void Close();
  bool ShoudTabShowToolTip(const Tab* tab);
  void UpdateTabTitle(const Tab* tab);

  Browser* browser() const { return browser_; }

  void set_tab_strip(TabStrip* tab_strip) { tab_strip_ = tab_strip; }
  TabStrip* tab_strip() const { return tab_strip_; }

 private:
  Browser* browser_;
  TabStrip* tab_strip_;

  base::WeakPtr<PopupThumbnail> CreatePopupThumbnail();
  void OnPopupThumbnailTypeChange();

  const content::WebContents* GetShowingWebContents();

  void UpdatePreviewForTabInternal(Tab* tab);

  // Overriden from thumbnails::ThumbnailService::Observer:
  void OnThumbnailUpdate(
      thumbnails::ThumbnailService* thumbail_service,
      const content::WebContents* web_contents,
      const gfx::Image& thumbnail) override;
  void ThumbnailServiceBeingDeleted(
      thumbnails::ThumbnailService* thumbail_service) override;

  // |showing_tab_| is null if not showing.
  Tab* showing_tab_;
  base::WeakPtr<PopupThumbnail> popup_thumbnail_;

  scoped_refptr<thumbnails::ThumbnailService> thumbail_service_;

  IntegerPrefMember popup_thumbnail_type_;

  base::Timer start_show_timer_;

  DISALLOW_COPY_AND_ASSIGN(TabPreviewController);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_TAB_PREVIEW_CONTROLLER_H_
