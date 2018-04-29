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

Steuerung::Steuerung(QObject *eltern) : QObject(eltern)
{
	K_Datenbankdatei="/tmp/PLZdb";
	K_PLZ_DB=new PLZ_Datenbank(K_Datenbankdatei,this);
	connect(K_PLZ_DB, &PLZ_Datenbank::KeineDatenbank,this,&Steuerung::KeinePLZ_DB);
	connect(K_PLZ_DB, &PLZ_Datenbank::Fehler,this,&Steuerung::Fehler);
	connect(K_PLZ_DB, &PLZ_Datenbank::Meldung,this,&Steuerung::Meldung);
	connect(K_PLZ_DB, &PLZ_Datenbank::DatenbankVorhanden,this,&Steuerung::PLZ_DB_Bereit);
}
const QStringList Steuerung::GPS(const uint &plz)
{
	return K_PLZ_DB->GPS(plz);
}
