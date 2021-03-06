//=========================================================
//  MusE
//  Linux Music Editor
//  $Id: ./muse/widgets/shortcutconfig.h $
//
//  Copyright (C) 1999-2011 by Werner Schweer and others
// Author: Mathias Lundgren <lunar_shuttle@users.sourceforge.net>, (C) 2003
//
// Copyright: Mathias Lundgren (lunar_shuttle@users.sourceforge.net) (C) 2003
//
//
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; version 2 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//=========================================================
//
// C++ Interface: shortcutconfig
//
// Description:
// Dialog for configuring keyboard shortcuts
//

#ifndef __SHORTCUTCONFIG_H
#define __SHORTCUTCONFIG_H

class QCloseEvent;

#include "ui_shortcutconfigbase.h"

#define SHRT_CATEGORY_COL 0

namespace MusEGui {

enum
      {
      SHRT_SHRTCUT_COL = 0,
      SHRT_DESCR_COL,
      };

class SCListViewItem : public QTreeWidgetItem {
      private:
      int index;

      public:
      SCListViewItem(QTreeWidget* parent, int i)
            : QTreeWidgetItem(parent), index(i) { }
      int getIndex() { return index; }

};


class ShortcutConfig : public QDialog, public Ui::ShortcutConfigBase {
      Q_OBJECT
      private:
      int current_category;

      void updateSCListView(int category, QString filterText);

      void updateSCListView() { updateSCListView(current_category, textFilter->text()); }

      void closeEvent(QCloseEvent *e);
      void closing();

      private slots:
      void categorySelChanged(QTreeWidgetItem*, int);
      void shortcutSelChanged(QTreeWidgetItem*, int);
      void assignShortcut();
      void clearShortcut();
      void applyAll();
      void okClicked();
      void resetAllClicked();
      void textFileClicked();
      void filterChanged(QString);

      protected:
      void reject();  
      signals:
      void saveConfig();
        
      public:
      ShortcutConfig(QWidget* parent = 0);
      bool _config_changed;

};

} // namespace MusEGui

#endif
