#include "about.h"

#include <QCoreApplication>
#include <QString>

#include "revision_utils.hpp"

#include "ui_about.h"

CAboutDlg::CAboutDlg(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::CAboutDlg)
{
  ui->setupUi(this);
  //avt 8/14/22
  ui->labelTxt->setText ("<h2>" + QString {"WSJT-X v"
                         + QCoreApplication::applicationVersion () + " " + revision ()}.simplified () + " modified by WM8Q</h2><br />"
                         "This is a derivative work based on WSJT-X at <br /><a href=\"https://sourceforge.net/projects/wsjt\">"
                         "https://sourceforge.net/projects/wsjt</a><br /><br />"

                         "Source code for modifications is at <br /><a href=\"https://sourceforge.net/u/k9avt/wsjt\">"
                         "https://sourceforge.net/u/k9avt/wsjt</a><br />"
                         "See 'Help|Copyright notice' for more information.<br /><br />"

                         "WSJT-X implements a number of digital modes designed for <br />"
                         "weak-signal Amateur Radio communication.  <br /><br />"

                         "&copy; 2001-2018 by Joe Taylor, K1JT, with grateful <br />"
                         "acknowledgment for contributions from AC6SL, AE4JY, <br />"
                         "DJ0OT, G3WDG, G4KLA, G4WJS, IV3NWV, IW3RAB, K3WYC, K9AN, <br />"
                         "KA6MAL, KA9Q, KB1ZMX, KD6EKQ, KI7MT, KK1D, ND0B, PY2SDR, <br />"
                         "VE1SKY, VK3ACF, VK4BDJ, VK7MO, W4TI, W4TV, and W9MDB.<br /><br />"

                         "WSJT-X modifications by WM8Q are licensed under the terms <br />"
                         "of Version 3 of the GNU General Public License (GPL) <br />"
                         "<a href=" WSJTX_STRINGIZE (PROJECT_HOMEPAGE) ">"
                         "<img src=\":/icon_128x128.png\" /></a>"
                         "<a href=\"https://www.gnu.org/licenses/gpl-3.0.txt\">"
                         "<img src=\":/gpl-v3-logo.svg\" height=\"80\" /><br />"
                         "https://www.gnu.org/licenses/gpl-3.0.txt</a>");
}

CAboutDlg::~CAboutDlg()
{
}
