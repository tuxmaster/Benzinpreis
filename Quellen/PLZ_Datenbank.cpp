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
#include <sqlite3.h>

Q_LOGGING_CATEGORY(logPLZ, APP_NAME  ".PLZ-DB")
PLZ_Datenbank::PLZ_Datenbank(QObject *eltern) : QObject(eltern)
{
	K_Datei=QString("%1/PLZ.db").arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
	K_Datenbank=false;
	qCDebug(logPLZ) << QString("PLZ database %1").arg(K_Datei).toUtf8().constData();
	QTimer::singleShot(0,this,SLOT(los()));
}
void PLZ_Datenbank::los()
{
	if (! QSqlDatabase::isDriverAvailable("QSQLITE"))
	{
		Q_EMIT Fehler(tr("Das SQLite Modul für Qt fehlt."));
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
	Q_EMIT Meldung(tr("Lade PLZ Datei aus dem Internet."));
}
void PLZ_Datenbank::DownloadFertig(QNetworkReply*antwort)
{
	QUrl url= antwort->url();
	if (antwort->error())
		Q_EMIT Fehler(tr("Fehler beim Abrufen von: %1\n%2")
						.arg(url.toString())
						.arg(url.errorString()));
	else
	{
		if (antwort->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 300 )
		{
			qCDebug(logPLZ)<<"download done";
			DownloadSpeichern(antwort);
		}
		else
			Q_EMIT Fehler(tr("HTTP Status Code 300 empfangen."));
	}
	antwort->deleteLater();
}

void PLZ_Datenbank::DownloadSpeichern(QNetworkReply* antwort)
{
	QTemporaryFile tmpDatei;
	if (!tmpDatei.open())
	{
		Q_EMIT Fehler(tr("Konnnte nicht in die Datei %1 schreiben.\n%1")
					  .arg(tmpDatei.fileName())
					  .arg(tmpDatei.errorString()));
		return;
	}
	qCDebug(logPLZ)<<QString("downloaded file %1").arg(tmpDatei.fileName()).toUtf8().constData();
	tmpDatei.write(antwort->readAll());
	tmpDatei.close();
	DownloadKonvertieren(tmpDatei);
}
void PLZ_Datenbank::DownloadKonvertieren(QFile &datei)
{
	Q_EMIT Meldung(tr("Beginne PLZ Konvertierung."));
	QFile DB_Datei(K_Datei);
	if (DB_Datei.exists())
	{
		if (!DB_Datei.remove())
		{
			Q_EMIT Fehler(tr("Konnte %1 nicht löschen.\n%2")
						  .arg(K_Datei)
						  .arg(DB_Datei.errorString()));
			return;
		}
	}
	QSqlDatabase DB_target = QSqlDatabase::addDatabase("QSQLITE", "file");
	QSqlDatabase DB_temp = QSqlDatabase::addDatabase("QSQLITE", "temp");
	DB_temp.setDatabaseName(":memory:");
	qCDebug(logPLZ)<<"tmp db status:"<<DB_temp.isValid();
	qCDebug(logPLZ)<<QString("databases: %1").arg(QSqlDatabase::connectionNames().join(", ")).toUtf8().constData();
	DB_target.setDatabaseName(K_Datei);
	QDir DB_path = QDir(QFileInfo(K_Datei).absolutePath());
	if (!DB_path.mkpath(DB_path.absolutePath()))
	{
		Q_EMIT Fehler(tr("Konnte den Pfad für die PLZ Datenbank nicht erstellen."));
		return;
	}
	if (!DB_temp.open())
	{
		Q_EMIT Fehler(tr("Konnte die temporäre PLZ Datenbank nicht erstellen.\n%1")
					  .arg(DB_temp.lastError().text()));
		return;
	}
	qCDebug(logPLZ)<<"tmp db file"<<DB_temp.databaseName().toUtf8().constData();
	QSqlQuery Abfrage(DB_temp);
	QStringList Abfragen;
	Abfragen<<"create table plz_gps(PLZ int primary key , Lat real not null, Lo real not null);";
	Abfragen<<"create UNIQUE INDEX idx_plz on plz_gps(PLZ)";
	for(QString Aufgabe : Abfragen)
	{
		if (! Abfrage.exec(Aufgabe))
		{
			Q_EMIT Fehler(tr("SQL Fehler: %1 bei %2.")
						  .arg(Abfrage.lastError().text())
						  .arg(Aufgabe));
			DB_temp.close();
			return;
		}
	}
	if (!datei.open(QIODevice::ReadOnly))
	{
		Q_EMIT Fehler(tr("Konnte %1 nicht öffnen.").arg(datei.fileName()));
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
					Q_EMIT Fehler(tr("Konnte die SQL Abfrage nicht vorbereiten.\n%1")
								  .arg(Abfrage.lastError().text()));
					DB_temp.close();
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
						Q_EMIT Fehler(tr("Konnte die Zeile nicht einfügen.\n%1")
									  .arg(Abfrage.lastError().text()));
						DB_temp.close();
						datei.close();
						return;
					}
				}
			}
	}
	datei.close();
	// try to sve the memory database
	if (!DB_target.open())
	{
		Q_EMIT Fehler(tr("Konnte die permanente PLZ Datenbank nicht erstellen.\n%1")
					  .arg(DB_target.lastError().text()));
		return;
	}
	QVariant memDB = DB_temp.driver()->handle();
	QVariant fileDB = DB_target.driver()->handle();
	if (memDB.isValid() && qstrcmp(memDB.typeName(), "sqlite3*") == 0 )
	{
		qCDebug(logPLZ)<<"Have valid QVariant for in memory db.";
		sqlite3 *handle_mem = *static_cast<sqlite3 **>(memDB.data());
		if (handle_mem)
		{
			qCDebug(logPLZ)<<"Have valid handler for in memory db.";
			if (fileDB.isValid() && qstrcmp(fileDB.typeName(), "sqlite3*") == 0 )
			{
				qCDebug(logPLZ)<<"Have valid QVariant for in file db.";
				sqlite3 *handle_file = *static_cast<sqlite3 **>(fileDB.data());
				if (handle_file)
				{
					qCDebug(logPLZ)<<"Have valid handler for in file db.";
					qCDebug(logPLZ)<<"Try to copy the databas";
					sqlite3_backup *pBackup;
					pBackup=sqlite3_backup_init(handle_file, "main", handle_mem, "main");
					bool backup_error=false;
					if(pBackup)
					{
						qCDebug(logPLZ)<<"Backup init OK";
						if (sqlite3_backup_step(pBackup, -1) != SQLITE_DONE)
							backup_error=true;
						sqlite3_backup_finish(pBackup);
						if(backup_error)
						{
							Q_EMIT Fehler(tr("Fehler beim kopieren der Datenbank.\n%1").arg(DB_target.lastError().text()));
							return;
						}
						else
							qCDebug(logPLZ)<<"Backup OK";
					}
					else
					{
						Q_EMIT Fehler(tr("Fehler beim kopieren der Datenbank.\n%1").arg(DB_target.lastError().text()));
						return;
					}
				}
				else
				{
					qCDebug(logPLZ)<<"invalid handler file db.";
					return;
				}
			}
		}
	}
	K_Datenbank=true;
	Q_EMIT DatenbankVorhanden();
	Q_EMIT Meldung(tr("PLZ Datenbank erstellt."));
}
const QStringList PLZ_Datenbank::GPS(const uint &plz)
{
	if(!K_Datenbank)
		return QStringList();
	QSqlDatabase DB;
	if (!QSqlDatabase::connectionNames().contains(PLZ_DB_NAME))
		DB = QSqlDatabase::addDatabase("QSQLITE",PLZ_DB_NAME);
	else
		DB = QSqlDatabase::database(PLZ_DB_NAME);
	DB.setDatabaseName(K_Datei);
	if (!DB.open())
	{
		Q_EMIT Fehler(tr("Konnte die PLZ Datenbank nicht öffnen.\n%1").arg(DB.lastError().text()));
		return QStringList();
	}
	QSqlQuery Abfrage(DB);
	if (!Abfrage.prepare("select Lat,Lo from plz_gps where PLZ=:PLZ"))
	{
		Q_EMIT Fehler(tr("Konnte die SQL Abfrage nicht vorbereiten.\n%1").arg(Abfrage.lastError().text()));
		DB.close();
		return QStringList();
	}
	Abfrage.bindValue(":PLZ",plz);
	if (!Abfrage.exec())
	{
		Q_EMIT Fehler(tr("Konnte die PLZ nicht aus der Datenbank lesen.\n%1").arg(Abfrage.lastError().text()));
		DB.close();
		return QStringList();
	}
	if(Abfrage.first())
		return QStringList()<<Abfrage.value(0).toString()<<Abfrage.value(1).toString();
	return QStringList();
}
