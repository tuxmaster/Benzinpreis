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
#include "Preissuche.h"
#include <QtWidgets>
#include <QtPositioning>

DlgHauptfenster::DlgHauptfenster(QWidget *eltern) :
	QMainWindow(eltern)
{
	setupUi(this);
	cbTreibstoff->addItem(trUtf8("Diesel"),"diesel");
	cbTreibstoff->addItem(trUtf8("Super"),"e5");
	cbTreibstoff->addItem(trUtf8("Super E10"),"e10");
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
	K_Steuerung->AktualisierungSetzen(K_Steuerung->AktualisierungHolen());
	txtPLZ_DB->setText(K_Steuerung->PLZ_DBHolen());
	QGeoCoordinate tmp=K_Steuerung->LetztePositionHolen();
	if(tmp.isValid())
		txtPosition->setText(QString("%1,%2").arg(tmp.latitude()).arg(tmp.longitude()));

	for( QString Name : K_Steuerung->PreissuchenHolen().keys())
	{
		twPreise->addTab(NeuerPreistab(),Name);
	}
	connect(K_Steuerung,&Steuerung::KeinePLZ_DB,this,&DlgHauptfenster::KeinePLZDatenbank);
	connect(K_Steuerung,&Steuerung::Fehler,this,&DlgHauptfenster::Fehler);
	connect(K_Steuerung,&Steuerung::Meldung,this,&DlgHauptfenster::Statusmeldung);
	connect(K_Steuerung,&Steuerung::PLZ_DB_Bereit,this,&DlgHauptfenster::PLZ_DB_da);
	connect(K_Steuerung,&Steuerung::Position,this,&DlgHauptfenster::NeuePosition);
	connect(K_Steuerung,&Steuerung::Warnung,this,&DlgHauptfenster::Statusmeldung);
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
void DlgHauptfenster::on_txtPLZ_DB_editingFinished()
{
	K_Steuerung->PLZ_DBSetzen(txtPLZ_DB->text());
}
void DlgHauptfenster::on_tbPLZ_DB_clicked()
{
	QString Datei=QFileDialog::getSaveFileName(this,trUtf8("PLZ Datenbank"),txtPLZ_DB->text());
	if(!Datei.isEmpty())
	{
		txtPLZ_DB->setText(Datei);
		on_txtPLZ_DB_editingFinished();
	}
}
void DlgHauptfenster::on_txtPosition_editingFinished()
{
	QStringList tmp=txtPosition->text().split(',');
	if (GueltigePosition(tmp))
	{
		QGeoCoordinate geo(tmp[0].toDouble(),tmp[1].toDouble());
		K_Steuerung->LetztePositionSetzen(geo);
	}
}
void DlgHauptfenster::on_txtPosition_textChanged(const QString &text)
{
	Q_UNUSED(text);
	on_txtPosition_editingFinished();
}
void DlgHauptfenster::on_pbAnlegen_clicked()
{
	QStringList pos=txtPosition->text().split(',');
	QString Sorte=cbTreibstoff->currentData().toString();
	if((!GueltigePosition(pos)) or (txtName->text().isEmpty()))
		return;
	for(int tab=0;tab<twPreise->count();tab++)
	{
		if(twPreise->tabText(tab)==txtName->text())
			return;
	}
	K_Steuerung->NeuePreissuche(new Preissuche(txtName->text(),QGeoCoordinate(pos[0].toDouble(),pos[1].toDouble()),
											   cbUmkreis->currentText().toUInt(),Sorte));
	twPreise->addTab(NeuerPreistab(),txtName->text());
}
bool DlgHauptfenster::GueltigePosition(const QStringList &pos)const
{
	if(pos.size() != 2)
		return false;
	QGeoCoordinate geo(pos[0].toDouble(),pos[1].toDouble());
	return geo.isValid();
}
void DlgHauptfenster::on_twPreise_tabCloseRequested(int index)
{
	twPreise->widget(index)->deleteLater();
	K_Steuerung->PreissucheLoeschen(twPreise->tabText(index));
	twPreise->removeTab(index);
}
QWidget* DlgHauptfenster::NeuerPreistab()
{
	QWidget* tab=new QWidget(this);
	return tab;
}
