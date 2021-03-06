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

Q_LOGGING_CATEGORY(logTankstellen, APP_NAME  ".Tankstellen")
Tankstellen::Tankstellen(QObject *eltern) : QObject(eltern)
{
	K_Preiswecker=new QTimer(this);
	K_Detailwecker=new QTimer(this);
	K_Tankstellen= new QHash<QUuid,Tankstelle*>;
	K_NM=new QNetworkAccessManager(this);
	K_AbrufpauseListe=false;
	K_AbrufpauseDetail=false;
	connect(K_Preiswecker,&QTimer::timeout,this,&Tankstellen::PreisAktualisierenWecker);
	connect(K_Detailwecker,&QTimer::timeout,this,&Tankstellen::DetailsAktualisieren);
	connect(K_NM,&QNetworkAccessManager::finished,this,&Tankstellen::AnfrageFertig);
	if (qApp->arguments().contains("tor"))
	{
		qCDebug(logTankstellen)<<"call the prices over the tor network";
		K_NM->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy,"127.0.0.1", 9150));
	}
	std::chrono::minutes minutes(TANKSTELLE_DETAIL_AKTUALISIERUNG);
	std::chrono::milliseconds ms=minutes;
	K_Detailwecker->start(ms);
}
Tankstellen::~Tankstellen()
{
	for (QUuid Zapfe : K_Tankstellen->keys())
	{
		delete K_Tankstellen->value(Zapfe);
		K_Tankstellen->remove(Zapfe);
	}
	delete K_Tankstellen;
}
void Tankstellen::API_Key_Setzen(const QString &key)
{
	qCDebug(logTankstellen)<<"set api key";
	if (qApp->arguments().contains("demo"))
	{
		qCDebug(logTankstellen)<<"demo mode for the prices";
		K_API_Key=PARAM_API_KEY_DEMO;
	}
	else
		K_API_Key=key;
}
void Tankstellen::PreisAktualisierenWecker()
{
	PreisAktualisieren(true);
}
void Tankstellen::PreisAktualisieren(const bool wecker)
{
	if (K_API_Key.isEmpty())
		return;
	if(wecker)
	{
		if (!K_Preiswecker->isActive())
			return;
	}
	if(K_AbrufpauseListe)
	{
		K_AbrufpauseListe=false;
		return;
	}
	for(Preissuche* Auftrag : K_Aufgaben)
	{
		QString URL=QString("%1list.php?lat=%2&lng=%3&rad=%4&sort=price&type=%5&apikey=%6").arg(TANKSTELLE_URL).arg(Auftrag->PositionHolen().latitude())
																						   .arg(Auftrag->PositionHolen().longitude())
																						   .arg(Auftrag->UmkreisHolen())
																						   .arg(Auftrag->SortenHolen())
																						   .arg(K_API_Key);
		QNetworkRequest Anforderung;
		Anforderung.setUrl(QUrl(URL));
		qCDebug(logTankstellen)<<"send request for"<<Auftrag->NameHolen().toUtf8().constData()<<"at"<<QTime::currentTime().toString("HH:mm:ss").toUtf8().constData();
		qCDebug(logTankstellen)<<"Request"<<Anforderung.url().toString().toUtf8().constData();
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
	std::chrono::minutes minutes(zeit);
	std::chrono::milliseconds ms=minutes;
	qCDebug(logTankstellen)<<"Set refresh rate for fuel prices to"<<minutes.count()<<"minutes.";
	K_Preiswecker->start(ms);
}
void Tankstellen::AnfrageFertig(QNetworkReply *antwort)
{
	if(antwort->error() == QNetworkReply::NoError)
	{
		QByteArray Daten=antwort->readAll();
		QJsonDocument Json=QJsonDocument::fromJson(Daten);
		if(!Json.isNull())
		{
			QJsonValue OK=Json.object().value("ok");
			if(!OK.isUndefined())			{

				if(antwort->url().path() == "/json/list.php" )
				{
					//Umkreisliste
					if(OK.toBool())
					{
						QJsonArray Zapfstellen=Json.object().value("stations").toArray();
						for(QJsonValue Zapfstelle : Zapfstellen)
						{
							QUuid id(Zapfstelle.toObject().value("id").toString());
							Tankstelle* Stelle;
							if(!K_Tankstellen->contains(id))
							{
								K_Tankstellen->insert(id,new Tankstelle(id));
								DetailsHolen(id);
							}
							Stelle=K_Tankstellen->value(id);
							QString Name=Zapfstelle.toObject().value("name").toString();
							QString Marke=Zapfstelle.toObject().value("brand").toString();
							QGeoAddress Adresse;
							QGeoLocation Ort;
							QGeoCoordinate Pos;
							QHash<QString,double> Preise;
							QString Strasse=QString("%1 %2")
											.arg(Zapfstelle.toObject().value("street").toString())
											.arg(Zapfstelle.toObject().value("houseNumber").toString());
							Adresse.setStreet(Strasse);
							Adresse.setCity(Zapfstelle.toObject().value("place").toString());
							Adresse.setPostalCode(QString::number(Zapfstelle.toObject().value("postCode").toInt()));
							Pos.setLatitude(Zapfstelle.toObject().value("lat").toDouble());
							Pos.setLongitude(Zapfstelle.toObject().value("lng").toDouble());
							bool offen=Zapfstelle.toObject().value("isOpen").toBool();
							double Entfernung=Zapfstelle.toObject().value("dist").toDouble();

							if(Zapfstelle.toObject().value("e5").isDouble())
								Preise["e5"]=Zapfstelle.toObject().value("e5").toDouble();
							if(Zapfstelle.toObject().value("e10").isDouble())
								Preise["e10"]=Zapfstelle.toObject().value("e10").toDouble();
							if(Zapfstelle.toObject().value("diesel").isDouble())
								Preise["diesel"]=Zapfstelle.toObject().value("diesel").toDouble();

							if(Zapfstelle.toObject().value("price").isDouble())
							{
								double Preis=Zapfstelle.toObject().value("price").toDouble();
								QString Sorte;
								for(QString Option: antwort->url().query().split("&"))
								{
									if(Option.startsWith("type"))
									{
										Sorte=Option.split("=")[1];
										break;
									}
								}
								Stelle->PreisEinfuegen(Sorte,Preis);
							}

							Ort.setAddress(Adresse);
							Ort.setCoordinate(Pos);
							Stelle->NameSetzen(Name);
							Stelle->MarkeSetzen(Marke);
							Stelle->AdresseSetzen(Ort);
							Stelle->OffenSetzen(offen);
							Stelle->EntfernungSetzen(Entfernung);
							if (!Preise.isEmpty())
								Stelle->PreiseSetzen(Preise);
						}
					}
					else
					{
						QJsonValue Fehler=Json.object().value("message");
						K_AbrufpauseListe=true;
						Q_EMIT Warnung(Fehler.toString());
					}
				}
				else if (antwort->url().path() == "/json/detail.php")				{

					//Detail für eine Tanke
					if(OK.toBool())
					{
						QUuid id=QUuid(antwort->url().query().split('&')[0].split('=')[1]);
						//qInfo()<<Json;
						QJsonObject Infos=Json.object().value("station").toObject();
						QString Bundesland=Infos.value("state").toString();
						bool ganztags=Infos.value("wholeDay").toBool();
						bool offen=Infos.value("isOpen").toBool();
						double e5=Infos.value("e5").toDouble();
						double e10=Infos.value("e10").toDouble();
						double diesel=Infos.value("diesel").toDouble();
						QStringList Ausnahmen;
						for(QVariant Ausnahme : Infos.value("overrides").toArray().toVariantList())
							Ausnahmen.append(Ausnahme.toString());
						QList<Oeffnungszeiten> oz;
						QString of_text;
						QString of_start;
						QString of_ende;
						for(QJsonValue offen : Infos.value("openingTimes").toArray())
						{
							of_text=offen.toObject().value("text").toString();
							of_start=offen.toObject().value("start").toString();
							of_ende=offen.toObject().value("end").toString();
							oz.append(Oeffnungszeiten(of_text,of_start,of_ende));
						}
						Tankstelle* TS=K_Tankstellen->value(id);
						if(TS)
						{
							QGeoAddress Adresse=TS->AdresseHolen().address();
							Adresse.setState(Bundesland);
							QGeoLocation Ort=TS->AdresseHolen();
							Ort.setAddress(Adresse);
							TS->AdresseSetzen(Ort);
							TS->OeffnungszeitenSetzen(oz);
							TS->AusnahmenSetzen(Ausnahmen);
							TS->GanztagsSetzen(ganztags);
							TS->OffenSetzen(offen);
							if(e5 >0)
								TS->PreisEinfuegen("e5",e5);
							if(e10 >0)
								TS->PreisEinfuegen("10",e10);
							if(diesel >0)
								TS->PreisEinfuegen("diesel",diesel);
							//qInfo()<<*TS;
						}
						else
							qCDebug(logTankstellen)<<"Kein Tankstellenobjekt";
					}
					else
					{
						QJsonValue Fehler=Json.object().value("message");
						K_AbrufpauseDetail=true;
						Q_EMIT Warnung(Fehler.toString());
					}
				}
			}
			else
			{
				QString keinStatus=tr("Kein Status im Json");
				qCDebug(logTankstellen)<<"No status part in json responce.";
				Q_EMIT Warnung(keinStatus);
			}
		}
	}
	else
	{
		uint HTTP_Status=antwort->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
		QString error_message;
		if (HTTP_Status==0)
		{
			qCDebug(logTankstellen)<<"Unable connect to the api server. Errorcode"<<antwort->error();
			error_message=tr("Der API Server entwortet nicht.");
		}
		else
		{
			qCDebug(logTankstellen)<<"Server error http status:"<<HTTP_Status;
			qCDebug(logTankstellen)<<"Server error responce:"<<antwort->readAll();
			error_message=tr("Der Server für die API ist nich zu erreichen. Status: %1").arg(HTTP_Status);
		}
		Q_EMIT Warnung(error_message);
	}
	antwort->deleteLater();
}
void Tankstellen::DetailsAktualisieren()
{
	DetailsHolen(QUuid());
}
void Tankstellen::DetailsHolen(const QUuid &tanke)
{
	if(K_AbrufpauseDetail)
	{
		K_AbrufpauseDetail=false;
		return;
	}
	//Eine oder Alle
	if(!tanke.isNull())
	{
		QString URL=QString("%1detail.php?id=%2&apikey=%3").arg(TANKSTELLE_URL)
														   .arg(tanke.toString().remove('{').remove('}'))
														   .arg(K_API_Key);
		QNetworkRequest Anforderung;
		Anforderung.setUrl(QUrl(URL));
		K_NM->get(Anforderung);
	}
	else
	{
		if (!K_Detailwecker->isActive())
			return;
		for(Tankstelle* Stelle : K_Tankstellen->values())
		{
			QString URL=QString("%1detail.php?id=%2&apikey=%3").arg(TANKSTELLE_URL)
															   .arg(Stelle->IDHolen().toString().remove('{').remove('}'))
															   .arg(K_API_Key);
			QNetworkRequest Anforderung;
			Anforderung.setUrl(QUrl(URL));
			K_NM->get(Anforderung);
		}
	}
}
