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
#include <QGeoCoordinate>

class QGeoPositionInfoSource;
class PLZ_Datenbank;
class Tankstellen;
class Preissuche;
class Steuerung : public QObject
{
		Q_OBJECT
	public:
		explicit							Steuerung(QObject *eltern = nullptr);
		~Steuerung();
		const QString&						Datenbankdatei()const {return K_Datenbankdatei;}
		const QStringList					GPS(const uint &plz);
		void								EinstellungenSpeichern();
		void								NeuePreissuche(Preissuche *suche);
		void								PreissucheLoeschen(const QString &welche);
		void								API_KeySetzen(const QString &key);
		void								AktualisierungSetzen(const uint&zeit);
		void								PLZ_DBSetzen(const QString &datei) {K_Datenbankdatei=datei;}
		void								LetztePositionSetzen(const QGeoCoordinate &pos){K_LetztePosition=pos;}
		const QString&						API_KeyHolen()const{return K_API_Key;}
		const uint&							AktualisierungHolen()const{return K_Akualisierung;}
		const QString&						PLZ_DBHolen()const {return K_Datenbankdatei;}
		const QGeoCoordinate&				LetztePositionHolen()const{return K_LetztePosition;}
		const QHash<QString,Preissuche*>&	PreissuchenHolen()const {return K_Suchen;}

	Q_SIGNALS:
		void								KeinePLZ_DB();
		void								PLZ_DB_Bereit();
		void								Fehler(const QString &fehler);
		void								Meldung(const QString &meldung);
		void								Position(const QStringList &position);
		void								Warnung(const QString&meldung);

	public Q_SLOTS:

	private Q_SLOTS:
		void								NeuePosition(const QGeoPositionInfo &postion);

	private:
		PLZ_Datenbank*						K_PLZ_DB;
		Tankstellen*						K_Tankstellen;
		QString								K_Datenbankdatei;
		QString								K_API_Key;
		uint								K_Akualisierung;
		QGeoPositionInfoSource*				K_PositionsQuelle;
		QSettings*							K_Einstellungen;
		QGeoCoordinate						K_LetztePosition;
		void								EinstellungenLaden();
		QHash<QString,Preissuche*>			K_Suchen;
};

#endif // STEUERUNG_H
