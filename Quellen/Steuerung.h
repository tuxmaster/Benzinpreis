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
#ifndef STEUERUNG_H
#define STEUERUNG_H

#include <QtCore>
#include <QGeoPositionInfo>

class QGeoPositionInfoSource;
class PLZ_Datenbank;
class Tankstellen;
class Steuerung : public QObject
{
		Q_OBJECT
	public:
		explicit				Steuerung(QObject *eltern = Q_NULLPTR);
		const QString&			Datenbankdatei()const {return K_Datenbankdatei;}
		const QStringList		GPS(const uint &plz);
		void					EinstellungenSpeichern();
		void					API_KeySetzen(const QString &key){K_API_Key=key;}
		void					AktualisierungSetzen(const uint&zeit) {K_Akualisierung=zeit;}
		void					PLZ_DBSetzen(const QString &datei) {K_Datenbankdatei=datei;}
		const QString&			API_KeyHolen()const{return K_API_Key;}
		const uint&				AktualisierungHolen()const{return K_Akualisierung;}
		const QString&			PLZ_DBHolen()const {return K_Datenbankdatei;}

	Q_SIGNALS:
		void					KeinePLZ_DB();
		void					PLZ_DB_Bereit();
		void					Fehler(const QString &fehler);
		void					Meldung(const QString &meldung);
		void					Position(const QStringList &position);

	public Q_SLOTS:

	private Q_SLOTS:
		void					NeuePosition(const QGeoPositionInfo &postion);

	private:
		PLZ_Datenbank*			K_PLZ_DB;
		Tankstellen*			K_Tankstellen;
		QString					K_Datenbankdatei;
		QString					K_API_Key;
		uint					K_Akualisierung;
		QGeoPositionInfoSource*	K_PositionsQuelle;
		QSettings*				K_Einstellungen;
		void					EinstellungenLaden();
};

#endif // STEUERUNG_H
