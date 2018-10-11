//  This file is part of ktimetrace.

//  ktimetrace is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  ktimetrace is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with ktimetrace.  If not, see <https://www.gnu.org/licenses/>.

//  (C) 2001 by Frank Mori Hess <fmhess@uiuc.edu>
//  (C) 2018 by Helio Chissini de Castro <helio@kde.org>

#include <QDialog>
#include <QString>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "zoomdialog.h"

ZoomDialog::ZoomDialog(QWidget *parent) :
	QDialog(parent)
{
	const int hsep = 10, vsep = 10;

	horizontalZoom = 1.0;

	hZoomInput = new QLineEdit(this);
	hZoomInput->move(hsep, vsep);
	hZoomInput->adjustSize();
	hZoomInput->setWhatsThis(
		"This controls the horizontal zoom for plotting."
		"Valid input is a number between 0.1 and 10."
		);

	hZoomInputLabel = new QLabel("Horizontal Zoom (pixels per data point)", this);
	hZoomInputLabel->move(hZoomInput->geometry().right() + hsep, hZoomInput->y());
	hZoomInputLabel->adjustSize();

	okButton = new QPushButton("&Ok", this);
	okButton->adjustSize();
	okButton->move(hZoomInput->x(), hZoomInput->geometry().bottom() + 2 * vsep);

	cancelButton = new QPushButton("&Cancel", this);
	cancelButton->adjustSize();
	cancelButton->move(okButton->geometry().right() + hsep, okButton->y());

	adjustSize();

	// connect signals/slots
	connect(hZoomInput, SIGNAL(textChanged(const QString &)), SLOT(slotHZoomInput(const QString &)));
	connect(okButton, SIGNAL(clicked()), SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), SLOT(reject()));
}

float ZoomDialog::hZoom(float zoom)
{
	QString temp;
	if( zoom <= 0.0)
		return horizontalZoom;
	horizontalZoom = zoom;
	hZoomInput->setText(temp.setNum(horizontalZoom));
	return horizontalZoom;
}

void ZoomDialog::slotHZoomInput(const QString &zoomText)
{
	horizontalZoom = zoomText.toFloat();
}
