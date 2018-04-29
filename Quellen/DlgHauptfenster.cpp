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
#include "DlgHauptfenster.h"
#include "Steuerung.h"
#include <QtWidgets>

DlgHauptfenster::DlgHauptfenster(QWidget *eltern) :
	QMainWindow(eltern)
{
	setupUi(this);
	QTimer::singleShot(0,this,SLOT(starten()));
}

void DlgHauptfenster::changeEvent(QEvent *ereignis)
{
	QMainWindow::changeEvent(ereignis);
	switch (ereignis->type())
	{
		case QEvent::LanguageChange:
			retranslateUi(this);
			break;
		default:
			break;
	}
}
void DlgHauptfenster::starten()
{
	K_Steuerung=new Steuerung(this);
	txtAPI_Key->setText(K_Steuerung->API_KeyHolen());
	sbAktualisierung->setValue(static_cast<int>(K_Steuerung->AktualisierungHolen()));

	connect(K_Steuerung,&Steuerung::KeinePLZ_DB,this,&DlgHauptfenster::KeinePLZDatenbank);
	connect(K_Steuerung,&Steuerung::Fehler,this,&DlgHauptfenster::Fehler);
	connect(K_Steuerung,&Steuerung::Meldung,this,&DlgHauptfenster::Statusmeldung);
	connect(K_Steuerung,&Steuerung::PLZ_DB_Bereit,this,&DlgHauptfenster::PLZ_DB_da);
	connect(K_Steuerung,&Steuerung::Position,this,&DlgHauptfenster::NeuePosition);
}
void DlgHauptfenster::KeinePLZDatenbank()
{
	 QMessageBox::information(this,trUtf8("Keine Datenbank"),trUtf8("Die Postleizahlendatenbank %1 wurde nicht gefunden.\nDaher wird sie erstellt.")
															.arg(K_Steuerung->Datenbankdatei()));
}
void DlgHauptfenster::Fehler(const QString &fehler)
{
	QMessageBox::critical(this,trUtf8("Fehler"),fehler);
	close();
}
void DlgHauptfenster::Statusmeldung(const QString &meldung)
{
	txtStatus->showMessage(meldung);
}
void DlgHauptfenster::PLZ_DB_da()
{
	gbTabkstelle->setEnabled(true);
}
void DlgHauptfenster::on_tbPLZ_clicked()
{
	bool OK;
	uint PLZ= static_cast<uint>(QInputDialog::getInt(this,trUtf8("Positionsbestimmung via Postleitzahl"),
								   trUtf8("Deine Postleitzahl"),0,10000,99999,1,&OK));
	if (OK)
		txtPosition->setText(K_Steuerung->GPS(PLZ).join(','));
}
void DlgHauptfenster::NeuePosition(const QStringList position)
{
	txtPosition->setText(position.join(','));
}
void DlgHauptfenster::closeEvent(QCloseEvent *ereignis)
{
	K_Steuerung->EinstellungenSpeichern();
	ereignis->accept();
}
void DlgHauptfenster::on_txtAPI_Key_editingFinished()
{
	K_Steuerung->API_KeySetzen(txtAPI_Key->text());
}
void DlgHauptfenster::on_sbAktualisierung_valueChanged(int wert)
{
	K_Steuerung->AktualisierungSetzen(static_cast<uint>(wert));
}
