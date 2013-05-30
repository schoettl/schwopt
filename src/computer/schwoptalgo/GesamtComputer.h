/*
 * GesamtComputer.h
 *
 *  Created on: 21.04.2013
 *      Author: jakob190590
 */

#ifndef GESAMTCOMPUTER_H_
#define GESAMTCOMPUTER_H_

#include "SchwoptAlgoComputer.h"

class GesamtComputer: public SchwoptAlgoComputer
{
public:
	GesamtComputer(const SchwimmerVector&);

	void compute();
	ostream& outputResult(ostream&);
};

#endif /* GESAMTCOMPUTER_H_ */