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

Tankstellen::Tankstellen(uint &aktualisierung_preis, QObject *eltern) : QObject(eltern)
{
	K_Aktualisierungsintervall_Preis=aktualisierung_preis;

	K_Preiswecker=new QTimer(this);
	K_Preiswecker->start(static_cast<int>(K_Aktualisierungsintervall_Preis)*60000);
	connect(K_Preiswecker,&QTimer::timeout,this,&Tankstellen::PreisAktualisieren);
}
void Tankstellen::PreisAktualisieren()
{

}
