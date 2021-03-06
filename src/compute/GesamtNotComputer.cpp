
#include "GesamtNotComputer.h"

GesamtNotComputer::GesamtNotComputer(const SchwimmerList& schwimmer, const SchwimmerList& eingesetzteSchwimmer) :
		Gesamt(schwimmer), eingesetzteSchwimmer(eingesetzteSchwimmer)
{
}

void GesamtNotComputer::compute()
{
	// ergebnis auffuellen und gesamtzeit berechnen
	SchwimmerList::const_iterator it = eingesetzteSchwimmer.begin();
	int pos;
	for (pos = 0; pos < ANZAHL_POSITIONEN && it != eingesetzteSchwimmer.end(); pos++, it++)
	{
		Schwimmer* schw = *it;
		ergebnis[pos] = schw;
		if (schw)
			gesamtzeit += schw->zeiten[positionDisziplinTable[pos]];
	}
}
