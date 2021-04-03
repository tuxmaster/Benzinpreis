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
#ifndef TANKSTELLEN_H
#define TANKSTELLEN_H

#include <QtCore>

#include "Vorgaben.h"

class QNetworkAccessManager;
class QNetworkReply;
class Preissuche;
class Tankstelle;

Q_DECLARE_LOGGING_CATEGORY(logTankstellen)
class Tankstellen : public QObject
{
		Q_OBJECT
	public:
		explicit					Tankstellen(QObject *eltern = nullptr);
		~Tankstellen();
		void						AufgabenUebernehmen(const QList<Preissuche*> &liste);
		void						AktualisieungsintervallSetzen(const uint &zeit);
		void						API_Key_Setzen(const QString &key);

	Q_SIGNALS:
		void						Warnung(const QString& meldung);

	private Q_SLOTS:
		void						DetailsAktualisieren();
		void						AnfrageFertig(QNetworkReply *antwort);
		void						PreisAktualisierenWecker();
	private:
		QTimer*						K_Preiswecker;
		QTimer*						K_Detailwecker;
		QList<Preissuche*>			K_Aufgaben;
		QHash<QUuid,Tankstelle*>*	K_Tankstellen;
		QNetworkAccessManager*		K_NM;
		QString						K_API_Key;
		bool						K_AbrufpauseListe;
		bool						K_AbrufpauseDetail;
		void						DetailsHolen(const QUuid &tanke=QUuid());
		void						PreisAktualisieren(const bool wecker=false);
};

#endif // TANKSTELLEN_H
