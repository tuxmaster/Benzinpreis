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
#ifndef TANKSTELLE_H
#define TANKSTELLE_H

#include <QtCore>
#include <QGeoLocation>
#include <QGeoAddress>

#include "Öffnungszeiten.h"

class Tankstelle
{
	public:
		Tankstelle(){}
		Tankstelle(const QUuid &id, const QString &name, const QString &marke,const QGeoLocation &adresse,const float &entfernung,
				   const bool &offen,const QHash<QString,float> &preise)
		{
			K_ID=id;
			K_Name=name;
			K_Marke=marke;
			K_Adresse=adresse;
			K_Entferung=entfernung;
			K_Offen=offen;
			K_Preise=preise;
		}
		void						DatenErweitern(const bool &ganztags, const QString &bundesland,const QList<Oeffnungszeiten> &oeffnungszeiten,
												   const QStringList &ausnahmen)
		{
			K_24h=ganztags;
			if(!bundesland.isNull())
				K_Adresse.address().setState(bundesland);
			K_Oeffnungszeiten=oeffnungszeiten;
			K_Ausnahmen=ausnahmen;
		}
		const QUuid&					IDHolen()const{return K_ID;}
		const QString&					NameHolen()const{return K_Name;}
		const QString&					MarkeHolen()const{return K_Marke;}
		const QGeoLocation&				AdresseHolen()const{return K_Adresse;}
		const float&					EntfernungHolen()const{return K_Entferung;}
		const bool&						OffenHolen()const{return K_Offen;}
		const bool&						GanztagsHolen()const{return K_24h;}
		const QHash<QString,float>&		PreiseHolen()const{return K_Preise;}
		const QList<Oeffnungszeiten>&	OeffnungszeitenHolen()const{return K_Oeffnungszeiten;}
		const QStringList&				AusnahmenHolen()const{return K_Ausnahmen;}

	private:
		QUuid							K_ID;
		QString							K_Name;
		QString							K_Marke;
		QGeoLocation					K_Adresse;
		float							K_Entferung;
		bool							K_Offen;
		bool							K_24h;
		QHash<QString,float>			K_Preise;
		QList<Oeffnungszeiten>			K_Oeffnungszeiten;
		QStringList						K_Ausnahmen;
};

#endif // TANKSTELLE_H
