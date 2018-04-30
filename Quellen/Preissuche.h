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
#ifndef PREISSUCHE_H
#define PREISSUCHE_H

#include <QtCore>
#include <QGeoCoordinate>

class Preissuche
{
	public:
		Preissuche(){}
		Preissuche(const QString &name){K_Name=name;}
		Preissuche(const QString &name,const QGeoCoordinate &pos,const uint umkreis,const QString &sorte)
		{
			K_Name=name;
			K_Position=pos;
			K_Umkreis=umkreis;
			K_Sorte=sorte;
		}

		void					PositionSetzen(const QGeoCoordinate &pos){K_Position=pos;}
		const QGeoCoordinate&	PositionHolen()const{return K_Position;}
		const QString&			NameHolen()const{return K_Name;}
		const uint&				UmkreisHolen()const{return K_Umkreis;}
		const QString&			SortenHolen()const{return  K_Sorte;}

	private:
		QGeoCoordinate			K_Position;
		QString					K_Name;
		QString					K_Sorte;
		uint					K_Umkreis;
};

#endif // PREISSUCHE_H
