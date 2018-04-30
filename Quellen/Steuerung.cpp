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
#include "Steuerung.h"
#include "PLZ_Datenbank.h"
#include "Tankstellen.h"
#include "Vorgaben.h"
#include "Preissuche.h"

#include <QtPositioning>

Steuerung::Steuerung(QObject *eltern) : QObject(eltern)
{
	K_Einstellungen=new QSettings(this);
	K_Tankstellen=new Tankstellen(this);
	EinstellungenLaden();

	K_PositionsQuelle=QGeoPositionInfoSource::createDefaultSource(this);
	K_PLZ_DB=new PLZ_Datenbank(K_Datenbankdatei,this);


	connect(K_PLZ_DB, &PLZ_Datenbank::KeineDatenbank,this,&Steuerung::KeinePLZ_DB);
	connect(K_PLZ_DB, &PLZ_Datenbank::Fehler,this,&Steuerung::Fehler);
	connect(K_PLZ_DB, &PLZ_Datenbank::Meldung,this,&Steuerung::Meldung);
	connect(K_PLZ_DB, &PLZ_Datenbank::DatenbankVorhanden,this,&Steuerung::PLZ_DB_Bereit);

	connect(K_Tankstellen,&Tankstellen::Warnung,this,&Steuerung::Warnung);

	if (K_PositionsQuelle)
		connect(K_PositionsQuelle,&QGeoPositionInfoSource::positionUpdated,this,&Steuerung::NeuePosition);
}
Steuerung::~Steuerung()
{
	for(Preissuche* Eintrag : K_Suchen.values())
		delete Eintrag;
}
const QStringList Steuerung::GPS(const uint &plz)
{
	return K_PLZ_DB->GPS(plz);
}

void Steuerung::NeuePosition(const QGeoPositionInfo &postion)
{
	if(postion.isValid())
		Q_EMIT Position(QStringList()<<QString::number(postion.coordinate().latitude())
						<<QString::number(postion.coordinate().longitude()));
}
void Steuerung::EinstellungenSpeichern()
{
	K_Einstellungen->setValue(PARAM_API_KEY,K_API_Key);
	K_Einstellungen->setValue(PARAM_AKTUALISIERUNG,K_Akualisierung);
	K_Einstellungen->setValue(PARAM_PLZ_DB,K_Datenbankdatei);
	if(K_LetztePosition.isValid())
		K_Einstellungen->setValue(PARAM_LETZTE_POSITION,QStringList()<<QString::number(K_LetztePosition.latitude())
																	  <<QString::number(K_LetztePosition.longitude()));
	for(QString Name : K_Suchen.keys())
	{
		K_Einstellungen->setValue(QString("%1/%2").arg(Name).arg(PARAM_PREIS_POSITION),QStringList()<<QString::number(K_Suchen[Name]->PositionHolen().latitude())
																									<<QString::number(K_Suchen[Name]->PositionHolen().longitude()));
		K_Einstellungen->setValue(QString("%1/%2").arg(Name).arg(PARAM_PREIS_UMKREIS),K_Suchen[Name]->UmkreisHolen());
		K_Einstellungen->setValue(QString("%1/%2").arg(Name).arg(PARAM_PREIS_SORTE),K_Suchen[Name]->SortenHolen());
	}
}
void Steuerung::EinstellungenLaden()
{
	K_API_Key=K_Einstellungen->value(PARAM_API_KEY,PARAM_API_KEY_DEMO).toString();
	K_Tankstellen->API_Key_Setzen(K_API_Key);
	K_Akualisierung=K_Einstellungen->value(PARAM_AKTUALISIERUNG,PARAM_AKTUALISSIERUG_VORAGBE).toUInt();
	K_Datenbankdatei=K_Einstellungen->value(PARAM_PLZ_DB,PARAM_PLZ_DB_WERT).toString();
	if(K_Einstellungen->value(PARAM_LETZTE_POSITION).toStringList().size() == 2)
		K_LetztePosition=QGeoCoordinate(K_Einstellungen->value(PARAM_LETZTE_POSITION).toStringList()[0].toDouble(),
										K_Einstellungen->value(PARAM_LETZTE_POSITION).toStringList()[1].toDouble());
	else
		K_LetztePosition=QGeoCoordinate();
	for(QString Name : K_Einstellungen->childGroups())
	{
		QStringList wert=K_Einstellungen->value(QString("%1/%2").arg(Name).arg(PARAM_PREIS_POSITION)).toStringList();
		if (wert.size()==2)
		{
			uint Umkreis=K_Einstellungen->value(QString("%1/%2").arg(Name).arg(PARAM_PREIS_UMKREIS),1).toUInt();
			QString Sorte=K_Einstellungen->value(QString("%1/%2").arg(Name).arg(PARAM_PREIS_SORTE),"all").toString();
			QGeoCoordinate Pos(wert[0].toDouble(),wert[1].toDouble());
			if(Pos.isValid())
				K_Suchen.insert(Name,new Preissuche(Name,Pos,Umkreis,Sorte));
		}
	}
	if(!K_Suchen.isEmpty())
		K_Tankstellen->AufgabenUebernehmen(K_Suchen.values());
}
void Steuerung::NeuePreissuche(Preissuche* suche)
{
	K_Suchen.insert(suche->NameHolen(),suche);
	K_Tankstellen->AufgabenUebernehmen(K_Suchen.values());
}
void Steuerung::PreissucheLoeschen(const QString &welche)
{
	delete K_Suchen[welche];
	K_Suchen.remove(welche);
	K_Einstellungen->remove(welche);
	K_Tankstellen->AufgabenUebernehmen(K_Suchen.values());
}
void Steuerung::AktualisierungSetzen(const uint&zeit)
{
	K_Tankstellen->AktualisieungsintervallSetzen(zeit);
}
void Steuerung::API_KeySetzen(const QString &key)
{
	K_API_Key=key;
	K_Tankstellen->API_Key_Setzen(key);
}
