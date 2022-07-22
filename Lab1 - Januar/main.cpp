#include <iostream>
#include "sqlite3.h"

using namespace std;

void ispisiGresku(sqlite3* db) {
	cout << "Greska: " << sqlite3_errmsg(db) << endl;
}

void otvoriBazu(sqlite3** db, const char* ime) {
	int rc = sqlite3_open(ime, db);
	if (rc != SQLITE_OK) {
		ispisiGresku(*db);
		sqlite3_close(*db);
		throw 1;
	}
}

void zatvoriBazu(sqlite3* db) {
	int rc = sqlite3_close(db);
	if (rc != SQLITE_OK) {
		ispisiGresku(db);
		throw 2;
	}
}

void pripremi(sqlite3* db, sqlite3_stmt** stmt, const char* sql) {
	int rc = sqlite3_prepare(db, sql, 256, stmt, nullptr);
	if (rc != SQLITE_OK) {
		ispisiGresku(db);
		sqlite3_finalize(*stmt);
		throw 3;
	}
}

void izvrsi(sqlite3* db, const char* sql) {
	char* errMsg = nullptr;
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		ispisiGresku(db);
		sqlite3_free(errMsg);
		throw 4;
	}
}

void stavke(sqlite3* db) {
	cout << "IdSta\t"; cout << "RedBroj\t";
	cout << "Datum\t\t"; cout << "Vreme\t";
	cout << "Iznos\t"; cout << "IdFil\t";
	cout << "IdRac\t"; cout << endl;
	//izvrsi(db, "SELECT * FROM Stavka");

	sqlite3_stmt* stmt = nullptr;
	const char* sql = "SELECT * FROM Stavka";
	pripremi(db, &stmt, sql);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		for (int i = 0; i < 7; i++) {
			const unsigned char* row = sqlite3_column_text(stmt, i);
			if (!row) cout << "null" << "\t";
			else cout << row << "\t";
		}
		cout << endl;
	}

	if (rc != SQLITE_DONE) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 5;
	}
	sqlite3_finalize(stmt);
}

void uplate(sqlite3* db) {
	cout << "IdSta\t"; cout << "Osnov\t"; cout << endl;
	//izvrsi(db, "SELECT * FROM Uplata");

	sqlite3_stmt* stmt = nullptr;
	const char* sql = "SELECT * FROM Uplata";
	pripremi(db, &stmt, sql);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		for (int i = 0; i < 2; i++) {
			const unsigned char* row = sqlite3_column_text(stmt, i);
			if (!row) cout << "null" << "\t";
			else cout << row << "\t";
		}
		cout << endl;
	}

	if (rc != SQLITE_DONE) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 5;
	}
	sqlite3_finalize(stmt);
}

void racuni(sqlite3* db, int idRac) {
	sqlite3_stmt* stmt = nullptr;
	const char* sql = "SELECT * FROM Racun WHERE IdRac=?";
	pripremi(db, &stmt, sql);
	int rc = 0;

	cout << "IdRac\t"; cout << "Status\t";
	cout << "BrojStavki\t"; cout << "DozvMinus\t";
	cout << "Stanje\t"; cout << "IdFil\t";
	cout << "IdKom\t"; cout << endl;
	sqlite3_bind_int(stmt, 1, idRac);
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		for (int i = 0; i < 7; i++) {
			const unsigned char* row = sqlite3_column_text(stmt, i);
			if (!row) cout << "null" << "\t";
			else cout << row << "\t";
		}
		cout << endl;
	}

	if (rc != SQLITE_DONE) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 5;
	}
	sqlite3_finalize(stmt);
}

void dodajStavku(sqlite3* db, int idFil, int idRac, int iznos) {
	const char* sql =
		"INSERT INTO Stavka(RedBroj, Datum, Vreme, Iznos, IdFil, IdRac) "
		"SELECT COALESCE(MAX(RedBroj), 0) + 1, DATE(), TIME(), ?, ?, ? "
		"FROM Stavka S WHERE S.IdRac=?";
	sqlite3_stmt* stmt = nullptr;
	pripremi(db, &stmt, sql);

	sqlite3_bind_int(stmt, 1, iznos);
	sqlite3_bind_int(stmt, 2, idFil);
	sqlite3_bind_int(stmt, 3, idRac);
	sqlite3_bind_int(stmt, 4, idRac);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 7;
	}
	sqlite3_finalize(stmt);
}

void dodajUplatu(sqlite3* db) {
	const char* sql =
		"INSERT INTO Uplata "
		"VALUES(?, 'Uplata')";
	sqlite3_stmt* stmt = nullptr;
	pripremi(db, &stmt, sql);

	sqlite3_bind_int(stmt, 1, sqlite3_last_insert_rowid(db));
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 8;
	}
	sqlite3_finalize(stmt);
}

// obavesti korisnika da li je racun promenio status
// obavesti korisnika da li je uplata bila uspesna
bool azurirajRacun(sqlite3* db, int idRac, int idFil, int iznos) {
	bool promenjen = false;
	const char* sql1 = "SELECT * FROM Racun WHERE IdRac=?";
	const char* sql2 =
		"UPDATE Racun "
		"SET Status=?, Stanje=?, "
		"BrojStavki=(SELECT COUNT(*) FROM Stavka WHERE IdRac=Racun.IdRac) "
		"WHERE IdRac=?";
	sqlite3_stmt* stmt = nullptr;
	
	pripremi(db, &stmt, sql1);
	sqlite3_bind_int(stmt, 1, idRac);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_ROW) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 9;
	}
	int staro = sqlite3_column_int(stmt, 4);
	int minus = sqlite3_column_int(stmt, 3);
	const char* status = (const char*)sqlite3_column_text(stmt, 1);
	sqlite3_finalize(stmt);
	stmt = nullptr;

	pripremi(db, &stmt, sql2);
	if (staro + iznos >= minus) {
		promenjen = true;
		sqlite3_bind_text(stmt, 1, "A", -1, nullptr);
	}
	else
		sqlite3_bind_text(stmt, 1, status, -1, nullptr);
	sqlite3_bind_int(stmt, 2, staro + iznos);
	sqlite3_bind_int(stmt, 3, idRac);
	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		ispisiGresku(db);
		sqlite3_finalize(stmt);
		throw 10;
	}
	sqlite3_finalize(stmt);
	return promenjen;
}

int main() {
	int idFil, idRac, iznos;
	sqlite3* db = nullptr;
	bool poc = false;
		
	otvoriBazu(&db, "Banka.db");
	cout << "Unesite IdFil: "; cin >> idFil;
	cout << "Unesite IdRac: "; cin >> idRac;
	cout << "Unesite iznos: "; cin >> iznos;

	cout << "Staro stanje:" << endl;
	cout << "Stavke:" << endl; 
	stavke(db); cout << endl;
	cout << "Uplate:" << endl; 
	uplate(db); cout << endl;
	cout << "Racun:" << endl;
	racuni(db, idRac);

	poc = true;
	izvrsi(db, "BEGIN TRANSACTION");
	try {
		dodajStavku(db, idFil, idRac, iznos);
		dodajUplatu(db);
		if (azurirajRacun(db, idRac, idFil, iznos))
			cout << "Racun je sada aktivan" << endl;
		else
			cout << "Racun nije promenio status" << endl;
		cout << "Novo stanje:" << endl;
		cout << "Stavke:" << endl;
		stavke(db); cout << endl;
		cout << "Uplate:" << endl;
		uplate(db); cout << endl;
		cout << "Racun:" << endl;
		racuni(db, idRac);

		cout << "Uplata je uspesna" << endl;
		izvrsi(db, "COMMIT");
		zatvoriBazu(db);
	}	
	catch (...) {
		if (poc) izvrsi(db, "ROLLBACK");
		cout << "Uplata nije uspesna";
		zatvoriBazu(db);
		return 1;
	}
	return 0;
}