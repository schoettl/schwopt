
#include <set>
#include <bitset>
#include <cassert>
#include <iomanip>
#include <algorithm>

#include "GesamtComputer.h"
#include "../Lagenstaffel.h"
#include "../Schlussstaffel.h"
#include "../Einzelstarts.h"
#include "../../Debugging.h"
#include "../../Zeit.h"

using namespace std;
using namespace NotMixed;

GesamtComputer::GesamtComputer(const SchwimmerList& schwimmer) :
		Gesamt(schwimmer), abstaendeInDisziplinen(createAbstandsMap(schwimmerSortiert))
{
}

PositionSchwimmerPair* GesamtComputer::findMostWanted(PositionSchwimmerPairList& list)
{
	PositionSchwimmerPair* result = NULL;
	// Abstand in Diziplin auf der angegebenen Position, fuer den Schwimmer, der fuer diese Position vorgesehen ist
	unsigned greatestAbstand = 0;
	bool foundOne = false; // ohne dem wird gar keiner gefunden, wenn alle abstaende 0 sind

	for (PositionSchwimmerPairList::iterator it = list.begin();
			it != list.end(); ++it)
	{
		unsigned abstand = abstaendeInDisziplinen[positionDisziplinTable[it->first]][it->second];
		if (abstand > greatestAbstand || !foundOne)
		{
			greatestAbstand = abstand;
			result = &*it;
		}
	}
	return result;
}

void GesamtComputer::removeFromAvailable(Schwimmer* schw,
		SchwimmerSet& availableSchwimmer,
		SchwimmerListVector& schwimmerSortiert,
		SchwimmerAbstandMapVector& abstaendeInDisziplinen) const
{
	availableSchwimmer.erase(schw);
	for (int disziplin = 0; disziplin < Disziplin::ANZAHL; disziplin++)
	{
		SchwimmerList& schwimmerzeitList = schwimmerSortiert[disziplin]; // list, sorted by zeiten in disziplin, with Schwimmer*
		SchwimmerAbstandMap& abstandsMap = abstaendeInDisziplinen[disziplin]; // map, sorted by abstand der zeiten in disziplin, Schwimmer* => unsigned

		abstandsMap.erase(schw);

		// Abstaende in abstandsMap evtl. korrigieren!
		SchwimmerList::iterator it = find(schwimmerzeitList.begin(), schwimmerzeitList.end(), schw);
		if (it == schwimmerzeitList.end())
			continue;

		if (it == schwimmerzeitList.begin())
		{
			// nothing to do (except remove from list)
			schwimmerzeitList.erase(it);
			continue;
		}

		// Standardfall: Abstand neu berechnen
		schwimmerzeitList.erase(it--);

		SchwimmerList::iterator next = it;
		next++; // next soll auf Naechstschlechteren zeigen

		unsigned itZeit   = (*it)->zeiten[disziplin];
		unsigned nextZeit = Zeit::MAX_UNSIGNED_VALUE; // falls it der letzte Schwimmer ist...
		if (next != schwimmerzeitList.end())
			nextZeit = (*next)->zeiten[disziplin];

		abstandsMap[*it] = nextZeit - itZeit;
	}
}

void GesamtComputer::ensureMax3Bedingung(Schwimmer* schw,
		SchwimmerIntMap& nAvailableSchwimmer,
		SchwimmerSet& availableSchwimmer,
		SchwimmerListVector& schwimmerSortiert,
		SchwimmerAbstandMapVector& abstaendeInDisziplinen,
		vector<SchwimmerSet>& availableSchwimmerPerBlock,
		vector<SchwimmerListVector>& schwimmerSortiertPerBlock,
		vector<SchwimmerAbstandMapVector>& abstaendeInDisziplinenPerBlock) const
{
	if (!nAvailableSchwimmer[schw])
	{
		removeFromAvailable(schw, availableSchwimmer, schwimmerSortiert, abstaendeInDisziplinen);
		for (int block = 0; block < ANZAHL_BLOCKE; block++)
			removeFromAvailable(schw, availableSchwimmerPerBlock[block], schwimmerSortiertPerBlock[block], abstaendeInDisziplinenPerBlock[block]);
	}
}

void GesamtComputer::ensureStaffelBedingung(Schwimmer* schw, int block,
		vector<SchwimmerSet>& availableSchwimmerPerBlock,
		vector<SchwimmerListVector>& schwimmerSortiertPerBlock,
		vector<SchwimmerAbstandMapVector>& abstaendeInDisziplinenPerBlock) const
{
	if (block == LAGENSTAFFEL || block == KRAULSTAFFEL)
		removeFromAvailable(schw, availableSchwimmerPerBlock[block], schwimmerSortiertPerBlock[block], abstaendeInDisziplinenPerBlock[block]);
	// weil dieser schwimmer in dieser staffel (block) dann nicht mehr darf.
}

void GesamtComputer::compute()
{
	// Anzahl Positionen, die noch nicht vergeben sind
	int vacantPositionen = ANZAHL_POSITIONEN;
	int vacantPositionenPerBlock[ANZAHL_BLOCKE] = {
			Lagenstaffel::ANZAHL_POSITIONEN,
			Schlussstaffel::ANZAHL_POSITIONEN,
			Einzelstarts::ANZAHL_POSITIONEN / 2,
			Einzelstarts::ANZAHL_POSITIONEN / 2 };

	// Positionen, die schon fest vergeben sind
	bitset<ANZAHL_POSITIONEN> assignedPositionen;

	// Noch verfuegbare Schwimmer
	SchwimmerSet availableSchwimmer;
	SchwimmerIntMap nAvailableSchwimmer;
	for (SchwimmerList::const_iterator it = schwimmer.begin();
			it != schwimmer.end(); ++it)
		nAvailableSchwimmer[*it] = 3; // Jeder Schwimmer maximal 3 x

	vector<SchwimmerSet>              availableSchwimmerPerBlock    (ANZAHL_BLOCKE, SchwimmerSet(schwimmer.begin(), schwimmer.end()));
	vector<SchwimmerListVector>       schwimmerSortiertPerBlock     (ANZAHL_BLOCKE, this->schwimmerSortiert);
	vector<SchwimmerAbstandMapVector> abstaendeInDisziplinenPerBlock(ANZAHL_BLOCKE, this->abstaendeInDisziplinen);

	// Hier geht's los
	while (vacantPositionen)
	{

		// Ueberall wo noch nicht vergeben ist, Besten einsetzen
		PositionSchwimmerPairList eligibleSchwimmer;
		for (int pos = 0; pos < ANZAHL_POSITIONEN; pos++)
			if (!assignedPositionen[pos])
			{
				const int disziplin = positionDisziplinTable[pos];
				SchwimmerList::const_iterator first = schwimmerSortiertPerBlock[getBlock(pos)][disziplin].begin();
				if (first == schwimmerSortiertPerBlock[getBlock(pos)][disziplin].end()) // Kein passender Schwimmer verfuegbar :O
					continue; // D. h. wir haben fuer diese Position keinen Vorschlag

				Schwimmer* const schw = *first;
				eligibleSchwimmer.push_back(PositionSchwimmerPair(pos, schw));
			}

		PositionSchwimmerPair* mostWanted = findMostWanted(eligibleSchwimmer);

#ifdef DEBUG
		eligibleSchwimmer.sort(NormAbstandComparer(*this)); // Sortierung nur fuer die Debug-Ausgabe
		gscheideDebugAusgabe(cout, mostWanted, positionDisziplinTable, schwimmerSortiert, eligibleSchwimmer, abstaendeInDisziplinen);
#endif
		if (mostWanted == NULL)
			break; // dann is' es vorbei (sollte nur vorkommen, wenn uebrige positionen nicht mehr besetzt werden koennen)

		// Diesen Schwimmer festsetzen fuer seine Position
		const int position    = mostWanted->first;
		Schwimmer* const schw = mostWanted->second;
		const int disziplin   = positionDisziplinTable[position];
		const int block       = getBlock(position);

		vacantPositionen--;
		vacantPositionenPerBlock[block]--;
		assignedPositionen[position] = true;
		assert(nAvailableSchwimmer[schw] > 0); // sonst haette schw gar nicht eingesetzt werden duerfen
		nAvailableSchwimmer[schw]--;

		ensureMax3Bedingung(schw, nAvailableSchwimmer, availableSchwimmer, schwimmerSortiert, abstaendeInDisziplinen, availableSchwimmerPerBlock, schwimmerSortiertPerBlock, abstaendeInDisziplinenPerBlock);
		ensureStaffelBedingung(schw, block, availableSchwimmerPerBlock, schwimmerSortiertPerBlock, abstaendeInDisziplinenPerBlock);

		// Ergebnis updaten
		ergebnis[position] = schw;
		gesamtzeit += schw->zeiten[disziplin];
	}
}
