﻿//#include <iostream>
//#include "sqlite3.h"
//
//using namespace std;
//
//sqlite3* otvoriBazu(const char* ime) {
//	sqlite3* db = nullptr;
//	int rc = sqlite3_open(ime, &db);
//	if (rc != SQLITE_OK) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_close(db);
//		throw 1;
//	}
//	return db;
//}
//
//void zatvoriBazu(sqlite3* db) {
//	int rc = sqlite3_close(db);
//	if (rc != SQLITE_OK) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		throw 2;
//	}
//}
//
//void pripremi(sqlite3* db, const char* sql, sqlite3_stmt** stmt) {
//	int rc = sqlite3_prepare(db, sql, 256, stmt, nullptr);
//	if (rc != SQLITE_OK) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_finalize(*stmt);
//		throw 3;
//	}
//}
//
//void izvrsiBezParametara(sqlite3* db, const char* sql) {
//	char* errMsg = nullptr;
//	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
//	if (rc != SQLITE_OK) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_free(errMsg);
//		throw 4;
//	}
//}
//
////ne vidim koja je poenta ovoga
//void ispisSvega(int brKolona, char** rows) {
//	for (int i = 0; i < brKolona; i++) {
//		if (!rows[i]) cout << "null" << '\t';
//		else cout << rows[i] << '\t';
//	}
//	cout << endl;
//}
//
//void sveStavke(sqlite3* db) {
//	cout << "IdSta\t"; cout << "RedBroj\t"; 
//	cout << "Datum\t\t"; cout << "Vreme\t";
//	cout << "Iznos\t"; cout << "IdFil\t"; 
//	cout << "IdRac\t"; cout << endl;
//	//izvrsiBezParametara(db, "SELECT * FROM Stavka");
//	sqlite3_stmt* stmt = nullptr;
//	const char* sql = "SELECT * FROM Stavka";
//	pripremi(db, sql, &stmt);
//	int rc = 0;
//	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//		for (int i = 0; i < 7; i++) {
//			const unsigned char* row = sqlite3_column_text(stmt, i);
//			if (!row) cout << "null" << "\t";
//			else cout << row << "\t";
//		}
//		cout << endl;
//	}
//	sqlite3_finalize(stmt);
//}
//
//void sveUplate(sqlite3* db) {
//	cout << "IdSta\t"; cout << "Osnov\t"; cout << endl;
//	izvrsiBezParametara(db, "SELECT * FROM Uplata");
//}
//
//void sviRacuniKomitenta(sqlite3* db, int idKom) {
//	const char* sql = "SELECT * FROM Racun WHERE IdKom=?";
//	sqlite3_stmt* stmt = nullptr;
//	int rc = 0;
//	pripremi(db, sql, &stmt);
//	sqlite3_bind_int(stmt, 1, idKom);
//
//	cout << "IdRac\t"; cout << "Status\t";
//	cout << "BrojStavki\t"; cout << "DozvMinus\t";
//	cout << "Stanje\t"; cout << "IdFil\t";
//	cout << "IdKom\t"; cout << endl;
//	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//		for (int i = 0; i < 7; i++) {
//			const unsigned char* row = sqlite3_column_text(stmt, i);
//			if (!row) cout << "null" << '\t';
//			else cout << row << '\t';
//		}
//		cout << endl;
//	}
//
//	if (rc != SQLITE_DONE) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_finalize(stmt);
//		throw 5;
//	}
//	sqlite3_finalize(stmt);
//}
//
//void proknjiziStavku(sqlite3* db, int iznos, int idFil, int idRac) {
//	const char* sql = 
//		"INSERT INTO Stavka(RedBroj, Datum, Vreme, Iznos, IdFil, IdRac) "
//		"VALUES(SELECT COALESCE(MAX(S.RedBroj), 0)+1, DATE(), TIME(), ?, ?, ?) "
//		"FROM Stavka S WHERE S.IdRac=?";
//	sqlite3_stmt* stmt = nullptr;
//	pripremi(db, sql, &stmt);
//
//	sqlite3_bind_int(stmt, 1, iznos);
//	sqlite3_bind_int(stmt, 2, idFil);
//	sqlite3_bind_int(stmt, 3, idRac);
//	sqlite3_bind_int(stmt, 4, idRac);
//	int rc = sqlite3_step(stmt);
//	if (rc != SQLITE_DONE) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_finalize(stmt);
//		throw 6;
//	}
//	sqlite3_finalize(stmt);
//}
//
//void proknjiziUplatu(sqlite3* db) {
//	const char* sql = "INSERT INTO Uplata(IdSta, Osnov) VALUES(?, 'Uplata')";
//	sqlite3_stmt* stmt = nullptr;
//	pripremi(db, sql, &stmt);
//
//	sqlite3_bind_int(stmt, 1, sqlite3_last_insert_rowid(db));
//	int rc = sqlite3_step(stmt);
//	if (rc != SQLITE_DONE) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_finalize(stmt);
//		throw 7;
//	}
//	sqlite3_finalize(stmt);
//}
//
//void azurirajRacun(sqlite3* db, int idRac) {
//	const char* sql =
//		"UPDATE Racun "
//		"SET Status = (CASE Status WHEN 'B' THEN 'A' ELSE Status END), "
//		"BrojStavki = (SELECT MAX(IdSta) FROM Stavka WHERE IdRac=?), "
//		"Stanje=0 WHERE IdRac=?";
//	sqlite3_stmt* stmt = nullptr;
//	pripremi(db, sql, &stmt);
//
//	sqlite3_bind_int(stmt, 1, idRac);
//	sqlite3_bind_int(stmt, 2, idRac);
//	int rc = sqlite3_step(stmt);
//	if (rc != SQLITE_OK) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_finalize(stmt);
//		throw 7;
//	}
//	sqlite3_finalize(stmt);
//}
//
//void resiBudalu(int idKom, int idFil, sqlite3* db) {
//	const char* sql =
//		"SELECT IdRac, Stanje FROM Racun "
//		"WHERE IdKom=? AND Stanje<0";
//	sqlite3_stmt* stmt = nullptr;
//	int rc = 0;
//	pripremi(db, sql, &stmt);
//
//	sqlite3_bind_int(stmt, 1, idKom);
//	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
//		izvrsiBezParametara(db, "BEGIN TRANSACTION");
//		try {
//			int idRac = sqlite3_column_int(stmt, 0);
//			int stanje = sqlite3_column_int(stmt, 1);
//			proknjiziStavku(db, -stanje, idFil, idRac);
//			proknjiziUplatu(db);
//			azurirajRacun(db, idRac);
//			izvrsiBezParametara(db, "COMMIT");
//		}
//		catch (...) {
//			izvrsiBezParametara(db, "ROLLBACK");
//			sqlite3_finalize(stmt);
//			throw;
//		}
//	}
//
//	if (rc != SQLITE_DONE) {
//		cout << "Greska: " << sqlite3_errmsg(db) << endl;
//		sqlite3_finalize(stmt);
//		throw 8;
//	}
//	sqlite3_finalize(stmt);
//}
//
//void ispisiStanje(int idKom, sqlite3* db) {
//	sveStavke(db); cout << endl;
//	sveUplate(db); cout << endl;
//	sviRacuniKomitenta(db, idKom); cout << endl;
//}
//
//int main() {
//	try {
//		sqlite3* db = otvoriBazu("Banka.db");
//		int idKom, idFil;
//		cout << "Unesite idKom: "; cin >> idKom;
//		//cout << "Unesite idFil: "; cin >> idFil;
//		cout << "Trenutno stanje:" << endl;
//		ispisiStanje(idKom, db);
//		//resiBudalu(idKom, idFil, db);
//		cout << "Novo stanje:" << endl;
//		ispisiStanje(idKom, db);
//		zatvoriBazu(db);
//	}
//	catch (...) {
//		return 1;
//	}
//	return 0;
//}

#include <iostream>
#include <string>
#include "sqlite3.h"

sqlite3* otvoriBazu(const char* imeBaze) {
    sqlite3* baza = nullptr;
    int kod = sqlite3_open(imeBaze, &baza);
    if (kod != SQLITE_OK) {
        throw std::string("Kod pri otvaranju: ") + std::to_string(kod);
    }
    return baza;
}

void zatvoriBazu(sqlite3* baza) {
    int kod = sqlite3_close(baza);
    if (kod != SQLITE_OK) {
        throw std::string("Kod pri zatvaranju: ") + std::to_string(kod);
    }
}

void pripremi(sqlite3* baza, const char* sql, sqlite3_stmt*& stmt) {
    int kod = sqlite3_prepare(baza, sql, -1, &stmt, nullptr);
    if (kod != SQLITE_OK) {
        std::string greska = std::string("Greška pri pripremanju upita: ") + sqlite3_errmsg(baza);
        sqlite3_finalize(stmt);
        throw greska;
    }
}

void izvrsiBezParametara(sqlite3* baza, const char* sql, int (*callback)(void*, int, char**, char**) = nullptr) {
    char* errmsg = nullptr;
    int kod = sqlite3_exec(baza, sql, callback, nullptr, &errmsg);
    if (kod != SQLITE_OK) {
        std::string greska = std::string("Greška pri izvršavanju: ") + errmsg;
        sqlite3_free(errmsg);
        throw greska;
    }
}

int ispisSvega(void*, int colCount, char** rows, char**) {
    for (int i = 0; i < colCount; ++i) {
        if (rows[i] == nullptr) {
            std::cout << "null\t";
        }
        else {
            std::cout << rows[i] << "\t";
        }
    }
    std::cout << std::endl;
    return 0;
}

void sveStavke(sqlite3* baza) {
    std::cout << "IdSta\tRedBroj\tDatum\t\tVreme\tIznos\tIdFil\tIdRac" << std::endl;
    izvrsiBezParametara(baza, "SELECT * FROM Stavka", ispisSvega);
}

void sveUplate(sqlite3* baza) {
    std::cout << "IdSta\tOsnov" << std::endl;
    izvrsiBezParametara(baza, "SELECT * FROM Uplata", ispisSvega);
}

void sviRacuniKomitenta(sqlite3* baza, int idKom) {
    const char* sql = "SELECT IdRac, Status, BrojStavki, DozvMinus, Stanje, IdFil, IdKom "
        "FROM Racun "
        "WHERE IdKom = ?";
    sqlite3_stmt* stmt = nullptr;
    int kod;
    pripremi(baza, sql, stmt);
    sqlite3_bind_int(stmt, 1, idKom);
    std::cout << "IdRac\tStatus\tBrojStavki\tDozvMinus\tStanje\tIdFil\tIdKom" << std::endl;
    while ((kod = sqlite3_step(stmt)) == SQLITE_ROW) {
        for (int i = 0; i < 7; ++i) {
            const unsigned char* red = sqlite3_column_text(stmt, i);
            if (red == nullptr) {
                std::cout << "null" << "\t";
            }
            else {
                std::cout << red << "\t";
            }
        }
        std::cout << std::endl;
    }
    if (kod != SQLITE_DONE) {
        std::string greska = std::string("Greška prilikom ispisivanja računa: ") + sqlite3_errmsg(baza);
        sqlite3_finalize(stmt);
        throw greska;
    }
    sqlite3_finalize(stmt);
}

void proknjiziStavku(sqlite3* baza, int iznos, int idFil, int idRac) {
    const char* sql = "INSERT INTO Stavka (RedBroj, Datum, Vreme, Iznos, IdFil, IdRac) "
        "SELECT COALESCE(MAX(S.RedBroj), 0) + 1, DATE(), TIME(), ?, ?, ? "
        "FROM Stavka S "
        "WHERE S.IdRac = ?";
    sqlite3_stmt* stmt = nullptr;
    int kod;
    pripremi(baza, sql, stmt);
    sqlite3_bind_int(stmt, 1, iznos);
    sqlite3_bind_int(stmt, 2, idFil);
    sqlite3_bind_int(stmt, 3, idRac);
    sqlite3_bind_int(stmt, 4, idRac);
    kod = sqlite3_step(stmt);
    if (kod != SQLITE_DONE) {
        std::string greska = std::string("Greška pri proknjižavanju stavke: ") + sqlite3_errmsg(baza);
        sqlite3_finalize(stmt);
        throw greska;
    }
    sqlite3_finalize(stmt);
}

void proknjiziUplatu(sqlite3* baza) {
    const char* sql = "INSERT INTO Uplata (IdSta, Osnov) VALUES (?, 'Uplata')";
    sqlite3_stmt* stmt = nullptr;
    int kod;
    pripremi(baza, sql, stmt);
    sqlite3_bind_int(stmt, 1, sqlite3_last_insert_rowid(baza));
    kod = sqlite3_step(stmt);
    if (kod != SQLITE_DONE) {
        std::string greska = std::string("Greška pri proknjižavanju uplate: ") + sqlite3_errmsg(baza);
        sqlite3_finalize(stmt);
        throw greska;
    }
    sqlite3_finalize(stmt);
}

void azurirajRacun(sqlite3* baza, int idRac) {
    const char* sql = "UPDATE Racun "
        "SET Status = (CASE Status "
        "WHEN 'B' THEN 'A' "
        "ELSE Status "
        "END),"
        "BrojStavki = (SELECT MAX(IdSta) FROM Stavka WHERE IdRac = ?),"
        "Stanje = 0 "
        "WHERE IdRac = ?";
    sqlite3_stmt* stmt = nullptr;
    int kod;
    pripremi(baza, sql, stmt);
    sqlite3_bind_int(stmt, 1, idRac);
    sqlite3_bind_int(stmt, 2, idRac);
    kod = sqlite3_step(stmt);
    if (kod != SQLITE_DONE) {
        std::string greska = std::string("Greška pri ažuriranju računa: ") + sqlite3_errmsg(baza);
        sqlite3_finalize(stmt);
        throw greska;
    }
    sqlite3_finalize(stmt);
}

void resi(int idKom, int idFil, sqlite3* baza) {
    const char* sql = "SELECT IdRac, Stanje "
        "FROM Racun "
        "WHERE IdKom = ? AND Stanje < 0";
    sqlite3_stmt* stmt = nullptr;
    int kod;
    pripremi(baza, sql, stmt);
    sqlite3_bind_int(stmt, 1, idKom);
    while ((kod = sqlite3_step(stmt)) == SQLITE_ROW) {
        izvrsiBezParametara(baza, "BEGIN TRANSACTION");
        try {
            int idRac = sqlite3_column_int(stmt, 0);
            int stanje = sqlite3_column_int(stmt, 1);
            proknjiziStavku(baza, -stanje, idFil, idRac);
            proknjiziUplatu(baza);
            azurirajRacun(baza, idRac);
            izvrsiBezParametara(baza, "COMMIT");
        }
        catch (std::string& greska) {
            izvrsiBezParametara(baza, "ROLLBACK");
            sqlite3_finalize(stmt);
            throw;
        }
    }
    if (kod != SQLITE_DONE) {
        std::string greska = std::string("Greška pri dohvatanju računa komitenta: ") + sqlite3_errmsg(baza);
        sqlite3_finalize(stmt);
        throw greska;
    }
    sqlite3_finalize(stmt);
}

void ispisiStanje(int idKom, sqlite3* baza) {
    sveStavke(baza);
    std::cout << std::endl;
    sveUplate(baza);
    std::cout << std::endl;
    sviRacuniKomitenta(baza, idKom);
    std::cout << std::endl;
}

int main(void) {
    try {
        sqlite3* baza = otvoriBazu("Banka.db");
        int idKom, idFil;
        std::cout << "Unesi ID komitenta: ";
        std::cin >> idKom;
        std::cout << "Unesi ID filijale: ";
        std::cin >> idFil;
        std::cout << "Trenutno stanje:" << std::endl;
        ispisiStanje(idKom, baza);
        resi(idKom, idFil, baza);
        std::cout << "Novo stanje:" << std::endl;
        ispisiStanje(idKom, baza);
        zatvoriBazu(baza);
    }
    catch (std::string& greska) {
        std::cout << "Desila se greška." << std::endl << greska << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}