// Copyright 2017 The WCT(Wisdom City Times) Authors. All rights reserved

#ifndef CHROME_BROWSER_UI_VIEWS_TABS_POPUP_THUMBNAIL_H_
#define CHROME_BROWSER_UI_VIEWS_TABS_POPUP_THUMBNAIL_H_

#include "base/macros.h"
#include "base/strings/string16.h"

class Browser;
class Tab;

namespace gfx {
class Image;
class Rect;
}  // namespace gfx

class PopupThumbnail {
 public:
  enum Type {
    PT_NONE = 0,
    PT_MOVE,
    PT_BIG,

    PT_TYPE_COUNT,
  };

  virtual bool IsShowing() = 0;
  virtual bool Show(const Tab* tab,
                    const gfx::Image& thumbnail,
                    const base::string16& title) = 0;
  virtual void UpdateThumbnail(const Tab* tab, const gfx::Image& thumbnail) = 0;
  virtual void UpdateTitle(const Tab* tab, const base::string16& title) = 0;
  virtual void TabRemoved(const Tab* tab) = 0;
  virtual void Hide() = 0;
  virtual void Close() = 0;

  Type type() const { return type_; }
  Browser* browser() const { return browser_; }

 protected:
  PopupThumbnail(Type type, Browser* browser);
  virtual ~PopupThumbnail();

 private:
  Type type_;
  Browser* browser_;

  DISALLOW_COPY_AND_ASSIGN(PopupThumbnail);
};

#endif  // CHROME_BROWSER_UI_VIEWS_TABS_POPUP_THUMBNAIL_H_
