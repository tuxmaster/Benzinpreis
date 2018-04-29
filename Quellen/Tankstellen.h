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
#ifndef TANKSTELLEN_H
#define TANKSTELLEN_H

#include <QtCore>

class Tankstellen : public QObject
{
		Q_OBJECT
	public:
		explicit Tankstellen(uint &aktualisierung_preis,QObject *eltern = Q_NULLPTR);

	private Q_SLOTS:
		void	PreisAktualisieren();

	private:
		uint	K_Aktualisierungsintervall_Preis;
		QTimer*	K_Preiswecker;
};

#endif // TANKSTELLEN_H
