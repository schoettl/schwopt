/*
 * Schwimmer.cpp
 *
 *  Created on: 14.04.2013
 *      Author: jakob190590
 */

#include <iostream>
#include <iomanip>

#include "Schwimmer.h"
#include "Zeit.h"

using namespace std;

Schwimmer::Schwimmer(const char geschlecht, const string& nachname, const string& vorname, const string& kuerzel) :
		nachname(nachname), vorname(vorname), kuerzel(kuerzel), geschlecht(geschlecht)
{
	for (int i = 0; i < Disziplin::ANZAHL; i++)
		zeiten[i] = 0;
}

Schwimmer::Schwimmer(const char geschlecht, const string& nachname, const string& vorname, const string& kuerzel,
		unsigned brust50, unsigned brust100,
		unsigned rueck50, unsigned rueck100,
		unsigned schm50,  unsigned schm100,
		unsigned frei50,  unsigned frei100) :

		nachname(nachname), vorname(vorname), kuerzel(kuerzel), geschlecht(geschlecht)
		//zeiten(brust50, brust100, rueck50, rueck100,
		//		schm50, schm100, schm50, schm100)
{
	zeiten[Disziplin::BRUST_50]  = brust50;
	zeiten[Disziplin::BRUST_100] = brust100;
	zeiten[Disziplin::RUECK_50]  = rueck50;
	zeiten[Disziplin::RUECK_100] = rueck100;
	zeiten[Disziplin::SCHM_50]   = schm50;
	zeiten[Disziplin::SCHM_100]  = schm100;
	zeiten[Disziplin::FREI_50]   = frei50;
	zeiten[Disziplin::FREI_100]  = frei100;
	zeitLess<0>((const Schwimmer*) this, (const Schwimmer*) this);
}

ostream& operator <<(ostream& os, const Schwimmer& schw)
{
	os << setiosflags(ios::left);
	os << setw(16) << schw.nachname << setw(10) << schw.vorname << setw(2) << schw.geschlecht << setw(3) << schw.kuerzel;
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::BRUST_50]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::BRUST_100]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::RUECK_50]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::RUECK_100]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::SCHM_50]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::SCHM_100]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::FREI_50]);
	os << setw(8) << Zeit::convertToString(schw.zeiten[Disziplin::FREI_100]);
	os << endl;
	return os;
}

istream& operator >>(istream& is, Schwimmer& schw)
{
	is >> schw.nachname >> schw.vorname >> schw.geschlecht >> schw.kuerzel;
	for (int i = 0; i < Disziplin::ANZAHL; i++)
	{
		string zeitInput;
		is >> zeitInput;
		schw.zeiten[i]  = Zeit::convertToUnsigned(zeitInput);
	}

	// Kommentar weglesen, oder was auch immer in ueberzaehligen Spalten steht
	char rest[200];
	is.getline(rest, 200);
	return is;
}

ostream& operator <<(ostream& os, Schwimmer const * const schw)
{
	return operator <<(os, *schw);
}

istream& operator >>(istream& is, Schwimmer * const schw)
{
	return operator >>(is, *schw);
}

ostream& operator <<(ostream& os, const SchwimmerVector& schwVec)
{
	for (SchwimmerVector::const_iterator it = schwVec.begin();
			it != schwVec.end(); ++it)
		os << *it;

	return os;
}


