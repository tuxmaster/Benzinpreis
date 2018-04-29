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
#include "PLZ_Datenbank.h"
#include "Vorgaben.h"

#include <QtNetwork>
#include <QtSql>

PLZ_Datenbank::PLZ_Datenbank(const QString &datenbank, QObject *eltern) : QObject(eltern)
{
	K_Datei=datenbank;
	K_Datenbank=false;
	QTimer::singleShot(0,this,SLOT(los()));
}
void PLZ_Datenbank::los()
{
	if (! QSqlDatabase::isDriverAvailable("QSQLITE"))
	{
		Q_EMIT Fehler(trUtf8("Das SQLite Modul für Qt fehlt."));
		return;
	}
	if(! QFile::exists(K_Datei))
	{
		Q_EMIT KeineDatenbank();
		DatenbankErstellen();
		return;
	}
	K_Datenbank=true;
	Q_EMIT DatenbankVorhanden();
}
void PLZ_Datenbank::DatenbankErstellen()
{
	K_NAM=new QNetworkAccessManager(this);
	connect(K_NAM,&QNetworkAccessManager::finished,this,&PLZ_Datenbank::DownloadFertig);
	QNetworkRequest Anforderung(QUrl(PLZ_DB_URL));
	Anforderung.setAttribute(QNetworkRequest::FollowRedirectsAttribute,true);
	K_NAM->get(Anforderung);
	Q_EMIT Meldung(trUtf8("Lade PLZ Datei aus dem Internet."));
}
void PLZ_Datenbank::DownloadFertig(QNetworkReply*antwort)
{
	QUrl url= antwort->url();
	if (antwort->error())
		Q_EMIT Fehler(trUtf8("Fehler beim Abrufen von: %1\n%2")
						.arg(url.toString())
						.arg(url.errorString()));
	else
	{
		if (antwort->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 300 )
			DownloadSpeichern(antwort);
		else
			Q_EMIT Fehler(trUtf8("HTTP Status Code 300 empfangen."));
	}
	antwort->deleteLater();
}

void PLZ_Datenbank::DownloadSpeichern(QNetworkReply* antwort)
{
	QTemporaryFile tmpDatei;
	if (!tmpDatei.open())
	{
		Q_EMIT Fehler(trUtf8("Konnnte nicht in die Datei %1 schreiben.\n%1")
					  .arg(tmpDatei.fileName())
					  .arg(tmpDatei.errorString()));
		return;
	}
	tmpDatei.write(antwort->readAll());
	tmpDatei.close();
	DownloadKonvertieren(tmpDatei);
}
void PLZ_Datenbank::DownloadKonvertieren(QFile &datei)
{
	Q_EMIT Meldung(trUtf8("Beginne PLZ Konvertierung."));
	QFile DB_Datei(K_Datei);
	if (DB_Datei.exists())
	{
		if (!DB_Datei.remove())
		{
			Q_EMIT Fehler(trUtf8("Konnte %1 nicht löschen.\n%2")
						  .arg(K_Datei)
						  .arg(DB_Datei.errorString()));
			return;
		}
	}
	QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE");
	DB.setDatabaseName(K_Datei);
	if (!DB.open())
	{
		Q_EMIT Fehler(trUtf8("Konnte die PLZ Datenbank nicht erstellen.\n%1")
					  .arg(DB.lastError().text()));
		return;
	}
	QSqlQuery Abfrage(DB);
	QStringList Abfragen;
	Abfragen<<"create table plz_gps(PLZ int primary key , Lat real not null, Lo real not null);";
	Abfragen<<"create UNIQUE INDEX idx_plz on plz_gps(PLZ)";
	for(QString Aufgabe : Abfragen)
	{
		if (! Abfrage.exec(Aufgabe))
		{
			Q_EMIT Fehler(trUtf8("SQL Fehler: %1 bei %2.")
						  .arg(Abfrage.lastError().text())
						  .arg(Aufgabe));
			DB.close();
			return;
		}
	}
	if (!datei.open(QIODevice::ReadOnly))
	{
		Q_EMIT Fehler(trUtf8("Konnte %1 nicht öffnen.").arg(datei.fileName()));
		return;
	}
	QTextStream PLZ_Daten(&datei);
	QStringList Zeile;
	uint PLZ;
	bool OK;
	while (!PLZ_Daten.atEnd())
	{
		Zeile=PLZ_Daten.readLine().split('\t');
		PLZ = Zeile[7].toUInt(&OK);
		if (OK)
			if ((PLZ >9999) && (!Zeile[4].isEmpty()) && (!Zeile[5].isEmpty()))
			{
				if (!Abfrage.prepare("insert into plz_gps(PLZ,Lat,Lo)"
									 "values(:PLZ,:Lat,:Lo)"))
				{
					Q_EMIT Fehler(trUtf8("Konnte die SQL Abfrage nicht vorbereiten.\n%1")
								  .arg(Abfrage.lastError().text()));
					DB.close();
					datei.close();
					return;
				}
				Abfrage.bindValue(":PLZ",PLZ);
				Abfrage.bindValue(":Lat",Zeile[4].toDouble());
				Abfrage.bindValue(":Lo",Zeile[5].toDouble());
				if (!Abfrage.exec())
				{
					//19 = Datensatz schon da.
					if ( Abfrage.lastError().nativeErrorCode().toInt() != 19 )
					{
						Q_EMIT Fehler(trUtf8("Konnte die Zeile nicht einfügen.\n%1")
									  .arg(Abfrage.lastError().text()));
						DB.close();
						datei.close();
						return;
					}
				}
			}
	}
	datei.close();
	K_Datenbank=true;
	Q_EMIT DatenbankVorhanden();
	Q_EMIT Meldung(trUtf8("PLZ Datenbank erstellt."));
}
const QStringList PLZ_Datenbank::GPS(const uint &plz)
{
	if(!K_Datenbank)
		return QStringList();
	QSqlDatabase DB = QSqlDatabase::addDatabase("QSQLITE");
	DB.setDatabaseName(K_Datei);
	if (!DB.open())
	{
		Q_EMIT Fehler(trUtf8("Konnte die PLZ Datenbank nicht öffnen.\n%1").arg(DB.lastError().text()));
		return QStringList();
	}
	QSqlQuery Abfrage(DB);
	if (!Abfrage.prepare("select Lat,Lo from plz_gps where PLZ=:PLZ"))
	{
		Q_EMIT Fehler(trUtf8("Konnte die SQL Abfrage nicht vorbereiten.\n%1").arg(Abfrage.lastError().text()));
		DB.close();
		return QStringList();
	}
	Abfrage.bindValue(":PLZ",plz);
	if (!Abfrage.exec())
	{
		Q_EMIT Fehler(trUtf8("Konnte die PLZ nicht aus der Datenbank lesen.\n%1").arg(Abfrage.lastError().text()));
		DB.close();
		return QStringList();
	}
	while (Abfrage.next())
		qInfo()<<Abfrage.value(0).toString()<<"-"<<Abfrage.value(1).toString();
	return QStringList();
}
