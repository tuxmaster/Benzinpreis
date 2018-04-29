/*
	Copyright (C) 2018 Frank Büttner frank@familie-büttner.de

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include <QApplication>
#include <QtCore>

#include "DlgHauptfenster.h"
#include "Vorgaben.h"

int main(int argumente_anzahl, char *argumente[])
{
	QApplication qt(argumente_anzahl,argumente);
	qt.setApplicationName(APP_NAME);
	qt.setOrganizationName(APP_ORGANISATION);

	QString Uebersetzungspfad=QLibraryInfo::location(QLibraryInfo::TranslationsPath);
	QTranslator QtUebersetzung;
	QtUebersetzung.load(QString("qt_%1").arg(QLocale::system().name()),Uebersetzungspfad);
	qt.installTranslator(&QtUebersetzung);
	DlgHauptfenster hauptfenster;
	hauptfenster.show();
	return qt.exec();
}
