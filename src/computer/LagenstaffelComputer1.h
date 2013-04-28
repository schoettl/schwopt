/*
 * LagenstaffelComputer1.h
 *
 *  Created on: 21.04.2013
 *      Author: jakob190590
 */

#ifndef LAGENSTAFFELCOMPUTER1_H_
#define LAGENSTAFFELCOMPUTER1_H_

#include <map>

#include "OptComputer.h"

class LagenstaffelComputer1: public OptComputer
{
	static const int ANZAHL_POSITIONEN_IN_STAFFEL = 4;
	static const int DISZIPLINEN_IN_STAFFEL[];

	class NormAbstandComparer;

	typedef pair<int, Schwimmer*> PositionSchwimmerPair;
	typedef set<PositionSchwimmerPair, NormAbstandComparer> SortedPositionSchwimmerSet;

	class NormAbstandComparer
	{
		LagenstaffelComputer1& computer;
	public:
		NormAbstandComparer(LagenstaffelComputer1&);
		bool operator ()(const PositionSchwimmerPair&, const PositionSchwimmerPair&);
	};

	map<Schwimmer*, float> normierteAbstaende[Disziplin::ANZAHL];

public:
	LagenstaffelComputer1(const SchwimmerVector&);

	void compute();
	ostream& outputResult(ostream&);
};

#endif /* LAGENSTAFFELCOMPUTER1_H_ */