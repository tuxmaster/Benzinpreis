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
#ifndef OEFFNUNGSZEITEN_H
#define OEFFNUNGSZEITEN_H

#include <QtCore>

class Oeffnungszeiten
{
	public:
		Oeffnungszeiten(){}
		Oeffnungszeiten(const QString &text,const QString &start,QString &ende)
		{
			K_Text=text;
			K_Anfang=start;
			K_Ende=ende;
		}
		const QString&	TextHolen()const{return K_Text;}
		const QString&	AnfangHolen()const{return K_Anfang;}
		const QString&	EndeHolen()const{return K_Ende;}


	private:
		QString			K_Text;
		QString			K_Anfang;
		QString			K_Ende;

};
//Damit man in das Objekt reinschauen kann.
QDebug operator <<(QDebug debug, const Oeffnungszeiten &o)
{
	QDebugStateSaver Sicherung(debug);
	debug<<"Öffnungszeiten("<<"Bezeichnung:"<<o.TextHolen()<<"Anfang:"<<o.AnfangHolen()<<"Ende:"<<o.EndeHolen()<<')';
	return debug;
}
#endif // ÖFFNUNGSZEITEN_H
