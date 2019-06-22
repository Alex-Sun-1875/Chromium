// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved.

#include "chrome/browser/ui/views/tabs/tab_preview_controller.h"

#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/thumbnails/thumbnail_service.h"
#include "chrome/browser/thumbnails/thumbnail_service_factory.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/tabs/tab_strip_model.h"
#include "chrome/browser/ui/views/tabs/big_popup_thumbnail.h"
#include "chrome/browser/ui/views/tabs/moving_popup_thumbnail.h"
#include "chrome/browser/ui/views/tabs/popup_thumbnail.h"
#include "chrome/browser/ui/views/tabs/tab.h"
#include "chrome/browser/ui/views/tabs/tab_strip.h"
#include "chrome/common/pref_names.h"

/// gmbrowser {
#include "base/strings/utf_string_conversions.h"
/// }

using thumbnails::ThumbnailService;

TabPreviewController::TabPreviewController(Browser* browser,
                                           TabStrip* tab_strip)
    : browser_(browser),
      tab_strip_(tab_strip),
      showing_tab_(nullptr),
      popup_thumbnail_(nullptr),
      thumbail_service_(
          ThumbnailServiceFactory::GetForProfile(browser->profile())),
      start_show_timer_(false, false) {

  if (thumbail_service_)
    thumbail_service_->AddObserver(this);

  popup_thumbnail_type_.Init(
      prefs::kPopupThumbnailType,
      browser->profile()->GetPrefs(),
      base::Bind(&TabPreviewController::OnPopupThumbnailTypeChange,
                 base::Unretained(this)));
}

TabPreviewController::~TabPreviewController() {
  if (thumbail_service_) {
    thumbail_service_->RemoveObserver(this);
    thumbail_service_ = nullptr;
  }

  if (popup_thumbnail_)
    popup_thumbnail_->Close();

}

bool TabPreviewController::ShouldPreview() const {
  if (!thumbail_service_)
    return false;

  PopupThumbnail::Type type =
      static_cast<PopupThumbnail::Type>(popup_thumbnail_type_.GetValue());

  if (type <= 0 || type >= PopupThumbnail::PT_TYPE_COUNT)
    return false;

  return true;
}

void TabPreviewController::UpdatePreviewForTab(Tab* tab) {
  DCHECK(!showing_tab_ || (showing_tab_ && ShouldPreview()));

  if (showing_tab_ == tab)
    return;

  showing_tab_ = tab;
  if (start_show_timer_.IsRunning())
    start_show_timer_.Stop();

  if (!tab || (popup_thumbnail_ && popup_thumbnail_->IsShowing())) {
    UpdatePreviewForTabInternal(tab);
    return;
  }

  start_show_timer_.Start(FROM_HERE, base::TimeDelta::FromMilliseconds(300),
      base::Bind(&TabPreviewController::UpdatePreviewForTabInternal,
                 base::Unretained(this), tab));

}

void TabPreviewController::TabRemoving(Tab* tab) {
  if (popup_thumbnail_) {
    popup_thumbnail_->TabRemoved(tab);
  }
}

void TabPreviewController::Close() {
  if (popup_thumbnail_) {
    popup_thumbnail_->Close();
  }
}

bool TabPreviewController::ShoudTabShowToolTip(const Tab* tab) {
  PopupThumbnail::Type type =
      static_cast<PopupThumbnail::Type>(popup_thumbnail_type_.GetValue());
  switch (type) {
  case PopupThumbnail::PT_MOVING:
    return false;
  }

  return true;
}

void TabPreviewController::UpdateTabTitle(const Tab* tab) {
  if (popup_thumbnail_)
    popup_thumbnail_->UpdataTitle(tab, tab->data().title);
}

base::WeakPtr<PopupThumbnail> TabPreviewController::CreatePopupThumbnail() {
  /// gmbrowser jixunfen check error
  /// gmbrowser {
  /// yfj
  PopupThumbnail::Type type =
      static_cast<PopupThumbnail::Type>(popup_thumbnail_type_.GetValue());
  switch (type) {
  case PopupThumbnail::PT_MOVING:
    return (new MovingPopupThumbnail(browser()))->AsWeakPtr();
  case PopupThumbnail::PT_BIG:
    return (new BigPopupThumbnail(browser()))->AsWeakPtr();
  default:
    return nullptr;
  }
  ///}
}

void TabPreviewController::OnPopupThumbnailTypeChange() {
  if (popup_thumbnail_)
    popup_thumbnail_->Close();
}

const content::WebContents* TabPreviewController::GetShowingWebContents() {
  return browser()->tab_strip_model()->GetWebContentsAt(
      tab_strip()->GetModelIndexOfTab(showing_tab_));
}

void TabPreviewController::UpdatePreviewForTabInternal(Tab* tab) {
  DCHECK(showing_tab_ == tab);

  if (!showing_tab_) {
    if (popup_thumbnail_)
      popup_thumbnail_->Hide();
    return;
  }

  if (!ShouldPreview()) {
    NOTREACHED();
    return;
  }

  if (!popup_thumbnail_)
    popup_thumbnail_ = CreatePopupThumbnail();

  if (!popup_thumbnail_) {
    NOTREACHED();
    return;
  }

  DCHECK(thumbail_service_);
  DCHECK(showing_tab_->visible());
  const content::WebContents* web_contents = GetShowingWebContents();
  if (!web_contents) {
    NOTREACHED();
    popup_thumbnail_->Hide();
    return;
  }

  gfx::Image thumbnail;
  if (!web_contents->IsIERenderer())
    thumbail_service_->GetPageThumbnailImage(web_contents, &thumbnail);

  base::string16 title = web_contents->GetTitle();
  if (title.empty())
    title = base::UTF8ToUTF16(web_contents->GetURL().spec());
  popup_thumbnail_->Show(showing_tab_, thumbnail, title);
}

// Overriden from thumbnails::ThumbnailService::Observer:
void TabPreviewController::OnThumbnailUpdate(
    thumbnails::ThumbnailService* thumbail_service,
    const content::WebContents* web_contents,
    const gfx::Image& thumbnail) {
  DCHECK_EQ(thumbail_service_.get(), thumbail_service);
  if (web_contents != GetShowingWebContents())
    return;

  if (popup_thumbnail_)
    popup_thumbnail_->UpdataThumbnail(showing_tab_, thumbnail);
}
void TabPreviewController::ThumbnailServiceBeingDeleted(
    thumbnails::ThumbnailService* thumbail_service) {
  DCHECK_EQ(thumbail_service_.get(), thumbail_service);

  thumbail_service->RemoveObserver(this);
  thumbail_service_ = nullptr;
}
