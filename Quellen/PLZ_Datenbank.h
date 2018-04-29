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
#ifndef PLZ_DATENBANK_H
#define PLZ_DATENBANK_H

#include <QtCore>

class QNetworkAccessManager;
class QNetworkReply;
class PLZ_Datenbank : public QObject
{
		Q_OBJECT
	public:
		explicit				PLZ_Datenbank( const QString &datenbank,QObject *eltern = Q_NULLPTR);
		const QStringList		GPS(const uint &plz);

	public Q_SLOTS:

	Q_SIGNALS:
		void					KeineDatenbank();
		void					DatenbankVorhanden();
		void					Fehler(const QString &meldung);
		void					Meldung(const QString &meldung);

	private Q_SLOTS:
		void					los();
		void					DownloadFertig(QNetworkReply* antwort);

	private:
		QString					K_Datei;
		QNetworkAccessManager*	K_NAM;
		bool					K_Datenbank;
		void					DatenbankErstellen();
		void					DownloadSpeichern(QNetworkReply* antwort);
		void					DownloadKonvertieren(QFile &datei);
};

#endif // PLZ_DATENBANK_H
