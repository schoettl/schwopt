/*
 * KraulstaffelComputer.h
 *
 *  Created on: 21.04.2013
 *      Author: jakob190590
 */

#ifndef KRAULSTAFFELCOMUTER_H_
#define KRAULSTAFFELCOMUTER_H_

#include <map>

#include "OptComputer.h"

class KraulstaffelComputer: public OptComputer
{
	static const int ANZAHL_POSITIONEN_IN_STAFFEL = 4;
	static const int DISZIPLIN = Disziplin::FREI_50;

public:
	KraulstaffelComputer(const SchwimmerVector&);

	void compute();
	ostream& outputResult(ostream&);
};

#endif /* KRAULSTAFFELCOMUTER_H_ */