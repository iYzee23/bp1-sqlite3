#include <iostream>
#include <exception>
#include "sqlite3.h"

using namespace std;

class Bulja : public exception {
public:
	const char* what() const override {
		return "Dlakava gguzica";
	}
};

void openConnection(sqlite3** db, const char* name) {
	int rc = sqlite3_open(name, db);
	if (rc != SQLITE_OK) {
		cout << "Error: " << sqlite3_errmsg(*db) << endl;
		throw Bulja();
	}
	cout << "Connection successfully opened .i." << endl;
}

void closeConnection(sqlite3** db) {
	sqlite3_close(*db);
	*db = nullptr;
}

void prepare(sqlite3* db, sqlite3_stmt** stmt, const char* sql) {
	int rc = sqlite3_prepare(db, sql, -1, stmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(*stmt);
		throw Bulja();
	}
}

void exec(sqlite3* db, const char* sql) {
	int rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
	if (rc != SQLITE_OK) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		throw Bulja();
	}
}

void selectRacun(sqlite3* db, int idkom) {
	const char* sql = "select * from racun where idkom=?";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);
	sqlite3_bind_int(stmt, 1, idkom);
	int rc;
	cout << "IdRac\tStatus\tBrStav\tDzMinus\tStanje\t";
	cout << "IdFil\tIdKom\t\n";
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		for (int i = 0; i < 7; i++) {
			const unsigned char* row = sqlite3_column_text(stmt, i);
			if (row) cout << row << "\t";
			else cout << "null" << "\t";
		}
		cout << endl;
	}
}

void select(sqlite3* db, const char* sql, int n) {
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		for (int i = 0; i < n; i++) {
			const unsigned char* row = sqlite3_column_text(stmt, i);
			if (!row) cout << "null" << '\t';
			else cout << row << '\t';
		}
		cout << endl;
	}
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	cout << "Successfully executed select" << endl;
	sqlite3_finalize(stmt);
}

void addStavka(sqlite3* db, int iznos, int idfil, int idrac) {
	const char* sql =
		"insert into stavka "
		"values((select max(idsta) from stavka)+1, (select max(redbroj) from stavka)+1, date(), time(), ?, ?, ?)";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);

	sqlite3_bind_int(stmt, 1, iznos);
	sqlite3_bind_int(stmt, 2, idfil);
	sqlite3_bind_int(stmt, 3, idrac);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}

void addUplata(sqlite3* db) {
	const char* sql =
		"insert into uplata "
		"values(?, 'Uplata')";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);

	sqlite3_bind_int(stmt, 1, /*sqlite3_last_insert_rowid(db)*/50);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}

void updateRacun(sqlite3* db, int idrac) {
	const char* sql =
		"update racun "
		"set status = (case status when 'B' then 'A' else status end), "
		"brojStavki = brojStavki+1, "
		"stanje = 0 "
		"where idrac = ?";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);

	sqlite3_bind_int(stmt, 1, idrac);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}

void function(sqlite3* db, int idfil, int idkom) {
	const char* sql =
		"select idrac, stanje "
		"from racun "
		"where idkom=? and stanje<0";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);
	sqlite3_bind_int(stmt, 1, idkom);
	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int idrac = sqlite3_column_int(stmt, 0);
		int stanje = -sqlite3_column_int(stmt, 1);
		exec(db, "BEGIN TRANSACTION");
		try {
			addStavka(db, stanje, idfil, idrac);
			addUplata(db);
			updateRacun(db, idrac);
			exec(db, "COMMIT");
		}
		catch (Bulja b) {
			exec(db, "ROLLBACK");
		}
	}
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}

void updateDjole(sqlite3* db, int idrac) {
	const char* sql =
		"update racun "
		"set stanje=stanje+1000 "
		"where idrac=?";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);
	sqlite3_bind_int(stmt, 1, idrac);
	int rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}

void helpDjole(sqlite3* db, int idkom) {
	const char* sql =
		"select r.idrac "
		"from racun r "
		"where r.idkom = ? and r.stanje = ( "
		"select min(stanje) "
		"from racun "
		"where idkom = r.idkom)";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);
	sqlite3_bind_int(stmt, 1, idkom);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int idrac = sqlite3_column_int(stmt, 0);
		updateDjole(db, idrac);
	}
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}

void mainDjole(sqlite3* db) {
	const char* sql =
		"select idkom from racun "
		"where idkom not in( "
		"select r.idkom from racun r where r.status = 'B') "
		"group by idkom having sum(stanje)>0";
	sqlite3_stmt* stmt = nullptr;
	prepare(db, &stmt, sql);
	int rc = 0;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int idkom = sqlite3_column_int(stmt, 0);
		exec(db, "BEGIN TRANSACTION");
		try {
			helpDjole(db, idkom);
			exec(db, "COMMIT");
		}
		catch (Bulja b) {
			exec(db, "ROLLBACK");
		}
	}
	if (rc != SQLITE_DONE) {
		cout << "Error: " << sqlite3_errmsg(db) << endl;
		sqlite3_finalize(stmt);
		throw Bulja();
	}
	sqlite3_finalize(stmt);
}



int main() {
	sqlite3* db;
	int idKom, idFil;
	try {
		openConnection(&db, "Banka.db");
		/*const char* sql1 =
			"select idkom, sum(stanje) "
			"from racun "
			"group by idkom";
		select(db, sql1, 7);
		cout << endl;*/
		//cout << "idKom? "; cin >> idKom;
		//cout << "idFil? "; cin >> idFil;
		mainDjole(db);
		selectRacun(db, 2); cout << endl;
		mainDjole(db);
		selectRacun(db, 2); cout << endl;
		//function(db, 10, 1);
		//selectRacun(db, 1); cout << endl;
	}
	catch (Bulja b) {
		cout << b.what() << endl;
		closeConnection(&db);
	}
}