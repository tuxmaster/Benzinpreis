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
#include "Tankstellen.h"
#include "Preissuche.h"
#include "Vorgaben.h"
#include "Tankstelle.h"
#include <QtNetwork>

Tankstellen::Tankstellen(QObject *eltern) : QObject(eltern)
{
	K_Preiswecker=new QTimer(this);
	K_NM=new QNetworkAccessManager(this);
	K_Abrufpause=false;
	connect(K_Preiswecker,&QTimer::timeout,this,&Tankstellen::PreisAktualisieren);
	connect(K_NM,&QNetworkAccessManager::finished,this,&Tankstellen::AnfrageFertig);
}
void Tankstellen::PreisAktualisieren()
{
	if (!K_Preiswecker->isActive() or (K_API_Key.isEmpty()))
		return;
	if(K_Abrufpause)
	{
		K_Abrufpause=false;
		return;
	}
	for(Preissuche* Auftrag : K_Aufgaben)
	{
		QString URL=QString("%1list.php?lat=%2&lng=%3&rad=%4&sort=price&type=%5&apikey=%6").arg(TANKSTELLE_URL).arg(Auftrag->PositionHolen().latitude())
																						   .arg(Auftrag->PositionHolen().longitude())
																						   .arg(Auftrag->UmkreisHolen())
																						   .arg(Auftrag->SortenHolen())
																						   //.arg(K_API_Key);
																						   .arg("00000000-0000-0000-0000-000000000002");

		QNetworkRequest Anforderung;
		Anforderung.setUrl(QUrl(URL));
		K_NM->get(Anforderung);
	}
}
void Tankstellen::AufgabenUebernehmen(const QList<Preissuche*> &liste)
{
	K_Aufgaben=liste;
	PreisAktualisieren();
}
void Tankstellen::AktualisieungsintervallSetzen(const uint &zeit)
{
	K_Preiswecker->start(static_cast<int>(zeit)*5000);
	//K_Preiswecker->start(static_cast<int>(zeit)*60000);
}
void Tankstellen::AnfrageFertig(QNetworkReply *antwort)
{
	if(antwort->error() == QNetworkReply::NoError)
	{
		QByteArray Daten=antwort->readAll();
		QJsonDocument Json=QJsonDocument::fromJson(Daten);
		if(!Json.isNull())
		{
			//qInfo()<<Json;
			QJsonValue OK=Json.object().value("ok");
			if(!OK.isUndefined())
			{
				if(OK.toBool())
				{
					//OK True
					if(antwort->url().toString().contains("list.php"))
					{
						//Unkreisliste
						QJsonArray Zapfstellen=Json.object().value("stations").toArray();
						for(QJsonValue Zapfstele : Zapfstellen)
							qInfo()<<Zapfstele;
					}
				}
				else
				{	//OK False
					QJsonValue Fehler=Json.object().value("message");
					K_Abrufpause=true;
					Q_EMIT Warnung(Fehler.toString());
				}
			}
		}
	}
	antwort->deleteLater();
}
