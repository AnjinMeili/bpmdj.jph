#include "ui-includes.h" // PCH include
#include "cluster-dialog.h"

#include <qvariant.h>
#include "metric-widget.h"
/*
 *  Constructs a ClusterDialog as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
ClusterDialog::ClusterDialog(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

}

/*
 *  Destroys the object and frees any allocated resources
 */
ClusterDialog::~ClusterDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ClusterDialog::languageChange()
{
    retranslateUi(this);
}
