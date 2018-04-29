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
#ifndef DLGHAUPTFENSTER_H
#define DLGHAUPTFENSTER_H

#include "ui_DlgHauptfenster.h"

class Steuerung;
class DlgHauptfenster : public QMainWindow, private Ui::DlgHauptfenster
{
		Q_OBJECT

	public:
		explicit	DlgHauptfenster(QWidget *eltern = Q_NULLPTR);

	private Q_SLOTS:
		void		starten();
		void		KeinePLZDatenbank();
		void		PLZ_DB_da();
		void		Fehler(const QString &fehler);
		void		Statusmeldung(const QString &meldung);
		void		on_tbPLZ_clicked();
		void		on_txtAPI_Key_editingFinished();
		void		on_sbAktualisierung_valueChanged(int wert);
		void		NeuePosition(const QStringList position);

	private:
		Steuerung*	K_Steuerung;
	protected:
		void		changeEvent(QEvent *ereignis) Q_DECL_OVERRIDE;
		void		closeEvent(QCloseEvent *ereignis) Q_DECL_OVERRIDE;
};

#endif // DLGHAUPTFENSTER_H
