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
#include <QGeoCoordinate>

#include "Öffnungszeiten.h"

class Tankstelle
{
	public:
		Tankstelle(){}
		Tankstelle(const QUuid &id)
		{
			K_ID=id;
			K_24h=false;
			K_Offen=false;
		}
		Tankstelle(const QUuid &id, const QString &name, const QString &marke,const QGeoLocation &adresse,const double &entfernung,
				   const bool &offen,const QHash<QString,double> &preise)
		{
			K_ID=id;
			K_Name=name;
			K_Marke=marke;
			K_Adresse=adresse;
			K_Entferung=entfernung;
			K_Offen=offen;
			K_24h=false;
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
		void							NameSetzen(const QString &name){K_Name=name;}
		void							MarkeSetzen(const QString &marke){K_Marke=marke;}
		void							AdresseSetzen(const QGeoLocation &adresse){K_Adresse=adresse;}
		void							EntfernungSetzen(const double &entfernung){K_Entferung=entfernung;}
		void							OffenSetzen(const bool &offen){K_Offen=offen;}
		void							PreiseSetzen(const QHash<QString,double> &preise){K_Preise=preise;}
		void							AusnahmenSetzen(const QStringList &ausnahmen){K_Ausnahmen=ausnahmen;}
		void							GanztagsSetzen(const bool &gt){K_24h=gt;}
		void							OeffnungszeitenSetzen(const QList<Oeffnungszeiten> &offen){K_Oeffnungszeiten=offen;}
		void							PreisEinfuegen(const QString &sorte, const double &preis)
		{
			K_Preise[sorte]=preis;
		}
		const QUuid&					IDHolen()const{return K_ID;}
		const QString&					NameHolen()const{return K_Name;}
		const QString&					MarkeHolen()const{return K_Marke;}
		const QGeoLocation&				AdresseHolen()const{return K_Adresse;}
		const double&					EntfernungHolen()const{return K_Entferung;}
		const bool&						OffenHolen()const{return K_Offen;}
		const bool&						GanztagsHolen()const{return K_24h;}
		const QHash<QString,double>&	PreiseHolen()const{return K_Preise;}
		const QList<Oeffnungszeiten>&	OeffnungszeitenHolen()const{return K_Oeffnungszeiten;}
		const QStringList&				AusnahmenHolen()const{return K_Ausnahmen;}

	private:
		QUuid							K_ID;
		QString							K_Name;
		QString							K_Marke;
		QGeoLocation					K_Adresse;
		double							K_Entferung;
		bool							K_Offen;
		bool							K_24h;
		QHash<QString,double>			K_Preise;
		QList<Oeffnungszeiten>			K_Oeffnungszeiten;
		QStringList						K_Ausnahmen;
};
//Damit man in das Objekt reinschauen kann.
QDebug operator <<(QDebug debug, const Tankstelle &t)
{
	QDebugStateSaver Sicherung(debug);
	debug<<"Tankstelle("<<"ID:"<<t.IDHolen().toString()<<"Name:"<<t.NameHolen()<<"Make:"<<t.MarkeHolen();
	QGeoAddress Adresse=t.AdresseHolen().address();
	QGeoCoordinate Geo=t.AdresseHolen().coordinate();
	debug<<"Adresse: {"<<"Straße:"<<Adresse.street()<<"PLZ:"<<Adresse.postalCode()<<"Ort:"<<Adresse.city();
	if(!Adresse.state().isNull())
		debug<<"Bundesland:"<<Adresse.state();
	debug<<"Geoposition:"<<'{'<<"Entfernung:"<<t.EntfernungHolen()<<"Breite:"<<Geo.latitude()<<"Länge:"<<Geo.longitude()<<'}';
	debug<<'}';
	debug<<"Geöffnet:"<<t.OffenHolen()<<"Ganztägig offen:"<<t.GanztagsHolen();
	if(!t.OeffnungszeitenHolen().isEmpty())
		debug<<"Öffnungszeiten:"<<t.OeffnungszeitenHolen();
	if(!t.AusnahmenHolen().isEmpty())
		debug<<"Ausnahmen:"<<t.AusnahmenHolen();
	debug<<"Preise:"<<t.PreiseHolen();
	debug<<')';
	return debug;
}
#endif // TANKSTELLE_H
