
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <getopt.h>
#include <iomanip>

#include "Schwimmer.h"
#include "CLI.h"
#include "Arguments.h"

#include "compute/mixed/SchlussstaffelComputer.h"
#include "compute/mixed/LagenstaffelExaktComputer.h"
#include "compute/mixed/LagenstaffelComputer.h"
#include "compute/mixed/GesamtComputer.h"
#include "compute/notmixed/GesamtComputer.h"
#include "compute/GesamtNotComputer.h"

using namespace std;

static const string usage   = "usage: schwopt [--help] [or other options] <filename>";
static const string version = "schwopt version 1.0.1";

static void exitWithError(const string& errmsg)
{
	cerr << "schwopt: " << errmsg << endl;
	exit(EXIT_FAILURE);
}

static void parseArguments(int argc, char* argv[], Arguments& arguments)
{
	// Lesenswert: http://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html
	// Argument Syntax Conventions. Z. B. per Konvention aequivalent: -ifile.txt und -i file.txt
	int c;
	while (1)
	{
		static struct option long_options[] = {
			/* These options set a flag. */
			//{ "dry", no_argument, &flag_dry, 1 }, // wird jetzt unten behandelt, zusammen mit abkuerzung -d
			/* These options don't set a flag.
			 We distinguish them by their indices. */
			// Grossbuchstaben heissen bei mir, es gibt KEINE Kurzform
			{ "help",        no_argument, 0, 'H' },
			{ "version",     no_argument, 0, 'V' },
			{ "dry",         no_argument, 0, 'd' },
			{ "input", optional_argument, 0, 'i' },
			{ "plain",       no_argument, 0, 'p' },
			{ "verbose",     no_argument, 0, 'v' },
			{ "class", required_argument, 0, 'C' },
			{ "block", required_argument, 0, 'B' },
			{ 0, 0, 0, 0 }
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long(argc, argv, "di::pv", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
		case 0:
			/* If this option set a flag, do nothing else now. */
			break;

		case 'H':
			{
				const int OPTION_COL_WIDTH = 20;
				cout << usage << endl << setiosflags(ios::left)
					 << setw(OPTION_COL_WIDTH) << " --help"             << endl
					 << setw(OPTION_COL_WIDTH) << " --version"          << endl
					 << setw(OPTION_COL_WIDTH) << " --class=CLASS"      << "mini-mixed, jugend-w, jugend-m, jugend-mixed, damen, " << endl
					 << setw(OPTION_COL_WIDTH) << ""                    << "herren, mixed (default)" << endl
					 << setw(OPTION_COL_WIDTH) << " --block=BLOCK"      << "lagenstaffel, schlussstaffel, einzelstarts," << endl
					 << setw(OPTION_COL_WIDTH) << ""                    << "gesamt (default)" << endl
					 << setw(OPTION_COL_WIDTH) << " -d, --dry"          << "Nicht berechnen, nur leeres Ergebnis anzeigen" << endl
					 << setw(OPTION_COL_WIDTH) << " -i, --input[=FILE]" << "Nicht berechnen, Belegung der Positionen selbst eingeben" << endl
					 << setw(OPTION_COL_WIDTH) << " -p, --plain"        << "Nur die Schwimmerkuerzel als Ergebnis ausgeben" << endl
					 << setw(OPTION_COL_WIDTH) << " -v, --verbose"      << endl;
				exit(EXIT_SUCCESS);
			}
			break;
		case 'V':
			cout << version << endl;
			exit(EXIT_SUCCESS);
			break;
		case 'd':
			arguments.flagDry = 1;
			break;
		case 'i':
			arguments.flagInput = 1;
			if (optarg)
				arguments.valInput = string(optarg);
			break;
		case 'p':
			arguments.flagPlain = 1;
			break;
		case 'v':
			arguments.flagVerbose = 1;
			break;
		case 'C':
			{
				int cl; // class
				for (cl = 0; cl < ANZAHL_CLASSES; cl++)
					if (strcmp(optarg, CLASS_NAME_TABLE[cl]) == 0)
					{
						arguments.valClass = (Class) cl;
						break;
					}
				if (cl == ANZAHL_CLASSES) // class not found
					exitWithError("invalid value `" + string(optarg) + "' for option -- class");
			}
			break;
		case 'B':
			{
				int bl; // block
				for (bl = 0; bl < ANZAHL_BLOCKS; bl++)
					if (strcmp(optarg, BLOCK_NAME_TABLE[bl]) == 0)
					{
						arguments.valBlock = (Block) bl;
						break;
					}
				if (bl == ANZAHL_BLOCKS) // block not found
					exitWithError("invalid value `" + string(optarg) + "' for option -- block");
			}
			break;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			// getopt failed oder eher: vergessen, einen case zu behandeln
			abort();
		}
	}

	if (optind < argc)
		arguments.valParam = argv[optind];
	else
	{
		cerr << usage << endl;
		exit(EXIT_FAILURE);
	}
}

static void readDataFile(const string& filename, SchwimmerList& schwimmer)
{
	// Datei oeffnen
	ifstream ifs(filename.c_str());
	if (!ifs.is_open())
		exitWithError("cannot open data file `" + filename + "'");

	// Schwimmer aus Datei in list einlesen
	while (ifs)
	{
		string line;
		getline(ifs, line);
		if (line.length() == 0 || line[0] == '#')
		{
			// Leerzeile oder Kommentarzeile
			continue;
		}

		istringstream iss(line);
		string nname, vname, kuerzel;
		char c;
		iss >> nname >> vname >> c >> kuerzel;
		Schwimmer::Geschlecht geschl = (c == 'w') ? Schwimmer::WEIBLICH : Schwimmer::MAENNLICH;
		Schwimmer* schw = new Schwimmer(geschl, nname, vname, kuerzel);

		for (int i = 0; iss && i < Disziplin::ANZAHL; i++)
		{
			string zeitInput;
			iss >> zeitInput;
			schw->zeiten[i] = Zeit::convertToUnsigned(zeitInput);
		}

		schwimmer.push_back(schw);
	}
	ifs.close();
}

// in allSchwimmer wird mit lookupSchwimmer das Schwimmerkuerzel nachgeschlagen, der resultierende Schwimmer wird in resultSchwimmer eingetragen
static void readInput(const Arguments& arguments, const SchwimmerList& allSchwimmer, SchwimmerList& resultSchwimmer)
{
	if (arguments.flagVerbose && arguments.valInput.empty()) // nur wenn verbose und input nicht aus Datei gelesen wird
		cout << "// [Eigene Belegung] GesamtComputer"    << endl
			 << "Manuelle Eingabe von Schwimmerkuerzeln" << endl
			 << "durch Leerzeichen getrennt!"            << endl
			 << "Eingabe beenden mit <Enter> <Strg + Z>" << endl
			 << "bzw. unter Unix mit <Enter> <Strg + D>" << endl;

	string input;
	if (arguments.valInput.empty())
		while (cin >> input)
			resultSchwimmer.push_back(lookupSchwimmer(allSchwimmer, input));
	else
	{
		ifstream ifs(arguments.valInput.c_str());
		if (!ifs.is_open())
			exitWithError("cannot open input file `" + arguments.valInput + "'");
		while (ifs >> input)
			resultSchwimmer.push_back(lookupSchwimmer(allSchwimmer, input));
	}
}

static void coutSchwimmer(Schwimmer* s)
{
	cout << s;
}

static void outputResult(ostream& os, const SchwoptComputer& computer, const bool& plain = false)
{
	if (!plain)
		computer.outputResult(os);
	else
	{
		SchwimmerVector vec = computer.getResult();
		for (SchwimmerVector::const_iterator it = vec.begin();
				it != vec.end(); ++it)
		{
			if (*it)
				os << (*it)->kuerzel;
			else
				os << "N/A";
			os << endl;
		}
	}
}

static void deleteSchwimmer(Schwimmer* s)
{
	delete s;
}

int main(int argc, char* argv[])
{
	Arguments arguments;
	parseArguments(argc, argv, arguments);

	SchwimmerList schwimmer;
	readDataFile(arguments.valParam, schwimmer);

	// So, ab hier kann mit der list schwimmer gearbeitet werden
	if (arguments.flagVerbose)
	{
		cout << "Nachname       Vorname   Kurzl brust50 brust100 rueck50 rueck100 schm50 schm100 frei50 frei100" << endl
		     << "--------------------------------------------------------------------------------------------" << endl;
		for_each(schwimmer.begin(), schwimmer.end(), coutSchwimmer);
		cout << endl;
	}

	SchwimmerList eingesetzteSchwimmer;
	if (arguments.flagInput)
		readInput(arguments, schwimmer, eingesetzteSchwimmer);

	switch (arguments.valClass) {
	case MIXED:
	case JUGEND_MIXED:
		if (arguments.flagVerbose)
			cout << "Wertungsklasse: Mixed (offene Klasse oder Jugend)" << endl;

		switch (arguments.valBlock) {
		case LAGENSTAFFEL:
			{
				if (arguments.flagVerbose) cout << "// [Exakt] LagenstaffelExaktComputer (Exakte Loesung, Durchprobieren)" << endl;
				Mixed::LagenstaffelExaktComputer lagenstaffelExaktComputer(schwimmer);
				lagenstaffelExaktComputer.compute();
				outputResult(cout, lagenstaffelExaktComputer, arguments.flagPlain);

				if (arguments.flagVerbose)
				{
					cout << "// [SchwoptAlgo] LagenstaffelComputer" << endl;
					Mixed::LagenstaffelComputer lagenstaffelComputer(schwimmer);
					lagenstaffelComputer.compute();
					outputResult(cout, lagenstaffelComputer, arguments.flagPlain);
				}
			}
			break;

		case SCHLUSSSTAFFEL:
			{
				if (arguments.flagVerbose) cout << "// [Exakt] KraulstaffelComputer (Exakte Loesung)" << endl;
				Mixed::SchlussstaffelComputer kraulstaffelComputer(schwimmer);
				kraulstaffelComputer.compute();
				outputResult(cout, kraulstaffelComputer, arguments.flagPlain);
			}
			break;

		case EINZELSTARTS:
			{
//				if (arguments.flagVerbose) cout << "// [SchwoptAlgo] Einzelstarts" << endl;
//				Mixed::EinzelstartsComputer einzelstartsComputer(schwimmer);
//				einzelstartsComputer.compute();
//				outputResult(cout, einzelstartsComputer, arguments.flagPlain);
			}
			break;

		case GESAMT:
			if (!arguments.flagDry && !arguments.flagInput)
			{
				// Normale Berechnung und Ausgabe
				if (arguments.flagVerbose) cout << "// [SchwoptAlgo] GesamtComputer" << endl;
				Mixed::GesamtComputer gesamtComputer(schwimmer);
				gesamtComputer.compute();
				outputResult(cout, gesamtComputer, arguments.flagPlain);
			}
			else
			{
				GesamtNotComputer gesamtNotComputer(schwimmer, eingesetzteSchwimmer);
				gesamtNotComputer.compute();
				outputResult(cout, gesamtNotComputer, arguments.flagPlain);
			}
			break;

		default:
			exitWithError("block `" + string(BLOCK_NAME_TABLE[arguments.valBlock]) + "' not supported yet");
			break;
		}
		break;

	case DAMEN:
	case HERREN:
	case JUGEND_W:
	case JUGEND_M:
		if (arguments.flagVerbose)
			cout << "Wertungsklasse: Nicht Mixed" << endl;

		switch (arguments.valBlock) {
		case GESAMT:
			if (!arguments.flagDry && !arguments.flagInput)
			{
				// Normale Berechnung und Ausgabe
				if (arguments.flagVerbose) cout << "// [SchwoptAlgo] GesamtComputer" << endl;
				NotMixed::GesamtComputer gesamtComputer(schwimmer);
				gesamtComputer.compute();
				outputResult(cout, gesamtComputer, arguments.flagPlain);
			}
			else
			{
				GesamtNotComputer gesamtNotComputer(schwimmer, eingesetzteSchwimmer);
				gesamtNotComputer.compute();
				outputResult(cout, gesamtNotComputer, arguments.flagPlain);
			}
			break;

		default:
			exitWithError("block `" + string(BLOCK_NAME_TABLE[arguments.valBlock]) + "' not supported yet");
			break;
		}
		break;

	default:
		exitWithError("class `" + string(CLASS_NAME_TABLE[arguments.valClass]) + "' not supported yet");
		break;
	}

	// Schwimmer in list freigeben
	for_each(schwimmer.begin(), schwimmer.end(), deleteSchwimmer);

	return EXIT_SUCCESS;
}
