#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"
#include "ooo_cpu.h"
// #include "bt9.h"
// #include "bt9_reader.h"

#define BORNTICK 1024
//for the allocation policy

//To get the predictor storage budget on stderr  uncomment the next line
#define PRINTSIZE
#include <vector>

#define SC // Enables the statiscal corrctor + 5.7 % MPKI without SC

#define GSC						// global history in SC  + 0.4% //if no GSC, no local, no loop: +3.2 %
#define IMLI					// 0.2 %
#define LOCALH				// + 0.9 % without local
#define LOOPPREDICTOR //loop predictor enable //0.4 % mispred reduction

//The statistical corrector components
//The two BIAS tables in the SC component
//We play with confidence here
#define LOGBIAS 7
int8_t Bias[(1 << LOGBIAS)];

#define INDBIAS (((((PC ^ (PC >> 2)) << 1) ^ (LowConf & (LongestMatchPred != alttaken))) << 1) + pred_inter) & ((1 << LOGBIAS) - 1)
int8_t BiasSK[(1 << LOGBIAS)];
#define INDBIASSK (((((PC ^ (PC >> (LOGBIAS - 2))) << 1) ^ (HighConf)) << 1) + pred_inter) & ((1 << LOGBIAS) - 1)

int8_t BiasBank[(1 << LOGBIAS)];
#define INDBIASBANK (pred_inter + (((HitBank + 1) / 4) << 4) + (HighConf << 1) + (LowConf << 2) + ((AltBank != 0) << 3)) & ((1 << LOGBIAS) - 1)

long long IMLIcount; // use to monitor the iteration number
#ifdef IMLI

#define LOGINB 7
#define INB 1
int Im[INB] = {8};
int8_t IGEHLA[INB][(1 << LOGINB)] = {{0}};

int8_t *IGEHL[INB];

#endif

#define LOGBWNB 7
#define BWNB 2

int BWm[BWNB] = {16, 8};
int8_t BWGEHLA[BWNB][(1 << LOGBWNB)] = {{0}};

int8_t *BWGEHL[BWNB];  // Backward branch history table
long long BWHIST;      // Backward branch history

//global branch GEHL

#define LOGGNB 7
#define GNB 2
long long GHIST;
int Gm[GNB] = {6, 3};
int8_t GGEHLA[GNB][(1 << LOGGNB)] = {{0}};

int8_t *GGEHL[GNB];

//large local history

#define LOGLNB 7
#define LNB 2
int Lm[LNB] = {6, 3};
int8_t LGEHLA[LNB][(1 << LOGLNB)] = {{0}};

int8_t *LGEHL[LNB];

#define LOGLOCAL 6
#define NLOCAL (1 << LOGLOCAL) // 64 Local histories
#define INDLOCAL ((PC ^ (PC >> 2)) & (NLOCAL - 1))
long long L_shist[NLOCAL];     // Local history tables (64x)

//update threshold for the statistical corrector
#define VARTHRES
//more than one update threshold
#define WIDTHRES 12
#define WIDTHRESP 8
#ifdef VARTHRES
#define LOGSIZEUP 6 //not worth increasing:  0-> 6 0.05 MPKI
#else
#define LOGSIZEUP 0
#endif
#define LOGSIZEUPS (LOGSIZEUP / 2)
int updatethreshold;
int Pupdatethreshold[(1 << LOGSIZEUP)]; //size is fixed by LOGSIZEUP
#define INDUPD (PC ^ (PC >> 2)) & ((1 << LOGSIZEUP) - 1)
#define INDUPDS ((PC ^ (PC >> 2)) & ((1 << (LOGSIZEUPS)) - 1))

int8_t WB[(1 << LOGSIZEUPS)];
int8_t WG[(1 << LOGSIZEUPS)];
int8_t WL[(1 << LOGSIZEUPS)];
int8_t WI[(1 << LOGSIZEUPS)];
int8_t WBW[(1 << LOGSIZEUPS)];

#define EWIDTH 6

// The two counters used to choose between TAGE ang SC on High Conf TAGE/Low Conf SC
int8_t FirstH, SecondH;

int LSUM;

#define CONFWIDTH 7						//for the counters in the choser
#define HISTBUFFERLENGTH 4096 // we use a 4K entries history buffer to store the branch history

// utility class for index computation
// this is the cyclic shift register for folding
// a long global history into a smaller number of bits; see P. Michaud's PPM-like predictor at CBP-1
class folded_history
{
public:
	unsigned comp;
	int CLENGTH;
	int OLENGTH;
	int OUTPOINT;

	folded_history()
	{
	}

	void init(int original_length, int compressed_length, int N)
	{
		comp = 0;
		OLENGTH = original_length;
		CLENGTH = compressed_length;
		OUTPOINT = OLENGTH % CLENGTH;
	}

	void update(uint8_t *h, int PT)
	{
		comp = (comp << 1) ^ h[PT & (HISTBUFFERLENGTH - 1)];
		comp ^= h[(PT + OLENGTH) & (HISTBUFFERLENGTH - 1)] << OUTPOINT;
		comp ^= (comp >> CLENGTH);
		comp = (comp) & ((1 << CLENGTH) - 1);
	}
};

class bentry // TAGE bimodal table entry
{
public:
	int8_t hyst;
	int8_t pred;

	bentry()
	{
		pred = 0;

		hyst = 1;
	}
};
class gentry // TAGE global table entry
{
public:
	int8_t ctr;
	uint tag;
	int8_t u;

	// meta info
	uint64_t full_pc;

	gentry()
	{
		ctr = 0;
		u = 0;
		tag = 0;
		full_pc = 0;
	}
};

#define POWER
//use geometric history length
#define NHIST 30 //in practice 15 different lengths
#define BORN 11	 //tables below BORN shared NBBANK[0] banks, ..
int NBBANK[2] = {9, 17};



#define BORNINFASSOC 7 //2 -way assoc for the lengths between the two borns: 0.6 %
#define BORNSUPASSOC 21

#define MINHIST 4
#define MAXHIST 1000

#define LOGG 7 /* logsize of a bank in TAGE tables */

#define TBITS 8 //minimum width of the tags  (low history lengths), +4 for high history lengths

bool NOSKIP[NHIST + 1]; //management of partial associativity
bool LowConf;
bool HighConf;
bool MedConf; // is the TAGE prediction medium confidence



#define NNN 1				// number of extra entries allocated on a TAGE misprediction: 0.4 % better if allocation of 2 elements instead of 1
#define HYSTSHIFT 2 // bimodal hysteresis shared by 4 entries
#define LOGB 12			// log of number of entries in bimodal predictor
#define PERCWIDTH 6 //Statistical corrector maximum counter width: 5 bits would  be sufficient

#define PHISTWIDTH 27 // width of the path history used in TAGE
#define UWIDTH 2			// u counter width on TAGE (2 bits- -> 1 bit about 1.5 %)
#define CWIDTH 3			// predictor counter width on the TAGE tagged tables

//the counter(s) to chose between longest match and alternate prediction on TAGE when weak counters
#define LOGSIZEUSEALT 3
bool AltConf; // Confidence on the alternate prediction
#define ALTWIDTH 5
#define SIZEUSEALT (1 << (LOGSIZEUSEALT))
#define INDUSEALT ((((HitBank - 1) / 8) << 1) + AltConf)
//#define INDUSEALT 0
int8_t use_alt_on_na[SIZEUSEALT];
int8_t BIM;

int TICK; // for the reset of the u counter

uint8_t ghist[HISTBUFFERLENGTH];
int ptghist;
long long phist;									 //path history
folded_history ch_i[NHIST + 1];		 //utility for computing TAGE indices
folded_history ch_t[2][NHIST + 1]; //utility for computing TAGE tags

//For the TAGE predictor
bentry *btable;						 //bimodal TAGE table
gentry *gtable[NHIST + 1]; // tagged TAGE tables
int m[NHIST + 1];
int TB[NHIST + 1];  				// Tagged predictor components Ti
int logg[NHIST + 1];

int GI[NHIST + 1];		// indexes to the different tables are computed only once
uint GTAG[NHIST + 1]; // tags for the different tables are computed only once
int BI;								// index of the bimodal table
bool pred_taken;			// prediction
bool alttaken;				// alternate  TAGEprediction
bool tage_pred;				// TAGE prediction
bool _bim_pred, _tage_pred, _loop_pred, _sc_pred;				// predicton of each component separately
bool LongestMatchPred;
int HitBank; // longest matching bank
int AltBank; // alternate matching bank
int Seed;		 // for the pseudo-random number generator
bool pred_inter;

#ifdef LOOPPREDICTOR
//parameters of the loop predictor
#define LOGL 3
#define WIDTHNBITERLOOP 10 // we predict only loops with less than 1K iterations
#define LOOPTAG 10				 //tag width in the loop predictor

class lentry //loop predictor entry
{
public:
	uint16_t NbIter;			//10 bits
	uint8_t confid;				// 4bits
	uint16_t CurrentIter; // 10 bits

	uint16_t TAG; // 10 bits
	uint8_t age;	// 4 bits
	bool dir;			// 1 bit

	//39 bits per entry
	lentry()
	{
		confid = 0;
		CurrentIter = 0;
		NbIter = 0;
		TAG = 0;
		age = 0;
		dir = false;
	}
};

lentry *ltable; //loop predictor table
//variables for the loop predictor
bool predloop; // loop predictor prediction
int LIB;
int LI;
int LHIT;				 //hitting way in the loop predictor
int LTAG;				 //tag on the loop predictor
bool LVALID;		 // validity of the loop predictor prediction
int8_t WITHLOOP; // counter to monitor whether or not loop prediction is beneficial

#endif

int predictorsize()
{
	int STORAGESIZE = 0;
	int inter = 0;

	STORAGESIZE +=
			NBBANK[1] * (1 << (logg[BORN])) * (CWIDTH + UWIDTH + TB[BORN]);
	STORAGESIZE += NBBANK[0] * (1 << (logg[1])) * (CWIDTH + UWIDTH + TB[1]);
	STORAGESIZE += (SIZEUSEALT)*ALTWIDTH;
	STORAGESIZE += (1 << LOGB) + (1 << (LOGB - HYSTSHIFT));
	STORAGESIZE += m[NHIST];
	STORAGESIZE += PHISTWIDTH;
	STORAGESIZE += 10; //the TICK counter

	fprintf(stderr, " (TAGE %d) ", STORAGESIZE);
#ifdef SC
#ifdef LOOPPREDICTOR

	inter = (1 << LOGL) * (2 * WIDTHNBITERLOOP + LOOPTAG + 4 + 4 + 1);
	fprintf(stderr, " (LOOP %d) ", inter);
	STORAGESIZE += inter;

#endif

	inter = WIDTHRESP * (1 << LOGSIZEUP); //the update threshold counters
	inter += WIDTHRES;
	inter += EWIDTH * (1 << LOGSIZEUPS); // the extra weight of the partial sums
	inter += (PERCWIDTH)*3 * (1 << LOGBIAS);
#ifdef GSC
	inter += GNB * (1 << LOGGNB) * (PERCWIDTH);
	inter += Gm[0];											 // the global  history
	inter += EWIDTH * (1 << LOGSIZEUPS); // the extra weight of the partial sums
	inter += BWNB * (1 << LOGBWNB) * PERCWIDTH;
	inter += EWIDTH * (1 << LOGSIZEUPS); // the extra weight of the partial sums
	inter += BWm[0];
#endif
#ifdef LOCALH
	inter += LNB * (1 << LOGLNB) * (PERCWIDTH);
	inter += NLOCAL * Lm[0];						 // the local history
	inter += EWIDTH * (1 << LOGSIZEUPS); // the extra weight of the partial sums
#endif

#ifdef IMLI
	inter += (1 << LOGINB) * PERCWIDTH;
	inter += Im[0];
	inter += EWIDTH * (1 << LOGSIZEUPS); // the extra weight of the partial sums

#endif
	inter += 2 * CONFWIDTH; //the 2 counters in the choser
	STORAGESIZE += inter;

	fprintf(stderr, " (SC %d) ", inter);
#endif
#ifdef PRINTSIZE
	fprintf(stderr, " (TOTAL %d bits = %d Kbits = %d kB) ", STORAGESIZE,
					STORAGESIZE / 1024, STORAGESIZE / 8192);
	fprintf(stdout, " (TOTAL %d bits = %d Kbits = %d kB) ", STORAGESIZE,
					STORAGESIZE / 1024, STORAGESIZE / 8192);
#endif

	return (STORAGESIZE);
}

class PREDICTOR
{
public:
	// Accessor to the parent CPU for the analysis.
	O3_CPU* parent_cpu;

	int THRES;

	PREDICTOR(void)
	{

		reinit();
#ifdef PRINTSIZE
		predictorsize();
#endif
	}

	void reinit()
	{

		m[1] = MINHIST;
		m[NHIST / 2] = MAXHIST;
		for (int i = 2; i <= NHIST / 2; i++)
		{
			m[i] =
					(int)(((double)MINHIST *
								 pow((double)(MAXHIST) / (double)MINHIST,
										 (double)(i - 1) / (double)(((NHIST / 2) - 1)))) +
								0.5);
		}

		for (int i = 1; i <= NHIST; i++)
		{
			NOSKIP[i] = ((i - 1) & 1) || ((i >= BORNINFASSOC) & (i < BORNSUPASSOC));
		}

		for (int i = NHIST; i > 1; i--)
		{
			m[i] = m[(i + 1) / 2];
		}
		for (int i = 1; i <= NHIST; i++)
		{
			TB[i] = TBITS + 4 * (i >= BORN);
			logg[i] = LOGG;
		}

#ifdef LOOPPREDICTOR
		ltable = new lentry[1 << (LOGL)];
#endif

		gtable[1] = new gentry[NBBANK[0] * (1 << LOGG)];
		gtable[BORN] = new gentry[NBBANK[1] * (1 << LOGG)];
		for (int i = BORN + 1; i <= NHIST; i++)
			gtable[i] = gtable[BORN];
		for (int i = 2; i <= BORN - 1; i++)
			gtable[i] = gtable[1];
		btable = new bentry[1 << LOGB];

		for (int i = 1; i <= NHIST; i++)
		{

			ch_i[i].init(m[i], 17 + (2 * ((i - 1) / 2) % 4), i - 1);
			ch_t[0][i].init(ch_i[i].OLENGTH, 13, i);
			ch_t[1][i].init(ch_i[i].OLENGTH, 11, i + 2);
		}
#ifdef LOOPPREDICTOR
		LVALID = false;
		WITHLOOP = -1;
#endif
		Seed = 0;

		TICK = 0;
		phist = 0;
		Seed = 0;

		for (int i = 0; i < HISTBUFFERLENGTH; i++)
			ghist[0] = 0;
		ptghist = 0;
		updatethreshold = 35 << 3;

		for (int i = 0; i < (1 << LOGSIZEUP); i++)
			Pupdatethreshold[i] = 0;
		for (int i = 0; i < GNB; i++)
			GGEHL[i] = &GGEHLA[i][0];
		for (int i = 0; i < LNB; i++)
			LGEHL[i] = &LGEHLA[i][0];

		for (int i = 0; i < GNB; i++)
			for (int j = 0; j < ((1 << LOGGNB) - 1); j++)
			{
				if (!(j & 1))
				{
					GGEHL[i][j] = -1;
				}
			}
		for (int i = 0; i < LNB; i++)
			for (int j = 0; j < ((1 << LOGLNB) - 1); j++)
			{
				if (!(j & 1))
				{
					LGEHL[i][j] = -1;
				}
			}

		for (int i = 0; i < BWNB; i++)
			BWGEHL[i] = &BWGEHLA[i][0];
		for (int i = 0; i < BWNB; i++)
			for (int j = 0; j < ((1 << LOGBWNB) - 1); j++)
			{
				if (!(j & 1))
				{
					BWGEHL[i][j] = -1;
				}
			}

#ifdef IMLI

		for (int i = 0; i < INB; i++)
			IGEHL[i] = &IGEHLA[i][0];
		for (int i = 0; i < INB; i++)
			for (int j = 0; j < ((1 << LOGINB) - 1); j++)
			{
				if (!(j & 1))
				{
					IGEHL[i][j] = -1;
				}
			}

#endif

		for (int i = 0; i < (1 << LOGB); i++)
		{
			btable[i].pred = 0;
			btable[i].hyst = 1;
		}

		for (int j = 0; j < (1 << LOGBIAS); j++)
		{
			switch (j & 3)
			{
			case 0:
				BiasSK[j] = -8;
				break;
			case 1:
				BiasSK[j] = 7;
				break;
			case 2:
				BiasSK[j] = -32;

				break;
			case 3:
				BiasSK[j] = 31;
				break;
			}
		}
		for (int j = 0; j < (1 << LOGBIAS); j++)
		{
			switch (j & 3)
			{
			case 0:
				Bias[j] = -32;

				break;
			case 1:
				Bias[j] = 31;
				break;
			case 2:
				Bias[j] = -1;
				break;
			case 3:
				Bias[j] = 0;
				break;
			}
		}
		for (int j = 0; j < (1 << LOGBIAS); j++)
		{
			switch (j & 3)
			{
			case 0:
				BiasBank[j] = -32;

				break;
			case 1:
				BiasBank[j] = 31;
				break;
			case 2:
				BiasBank[j] = -1;
				break;
			case 3:
				BiasBank[j] = 0;
				break;
			}
		}
		for (int i = 0; i < SIZEUSEALT; i++)
		{
			use_alt_on_na[i] = 0;
		}
		for (int i = 0; i < (1 << LOGSIZEUPS); i++)
		{
			WB[i] = 4;
			WG[i] = 7;
			WL[i] = 7;
			WI[i] = 7;
			WBW[i] = 7;
		}
		TICK = 0;
		for (int i = 0; i < NLOCAL; i++)
		{
			L_shist[i] = 0;
		}

		GHIST = 0;
		ptghist = 0;
		phist = 0;
	}

	// index function for the bimodal table

	int bindex(UINT64 PC)
	{
		return ((PC ^ (PC >> LOGB)) & ((1 << (LOGB)) - 1));
	}

	// the index functions for the tagged tables uses path history as in the OGEHL predictor
	//F serves to mix path history: not very important impact

	int F(long long A, int size, int bank)
	{
		int A1, A2;
		A = A & ((1 << size) - 1);
		A1 = (A & ((1 << logg[bank]) - 1));
		A2 = (A >> logg[bank]);

		if (bank < logg[bank])
			A2 =
					((A2 << bank) & ((1 << logg[bank]) - 1)) +
					(A2 >> (logg[bank] - bank));
		A = A1 ^ A2;
		if (bank < logg[bank])
			A =
					((A << bank) & ((1 << logg[bank]) - 1)) + (A >> (logg[bank] - bank));
		return (A);
	}

	// gindex computes a full hash of PC, ghist and phist
	int gindex(unsigned int PC, int bank, long long hist,
						 folded_history *ch_i)
	{
		int index;
		int M = (m[bank] > PHISTWIDTH) ? PHISTWIDTH : m[bank];
		index = PC ^ (PC >> (abs(logg[bank] - bank) + 1)) ^ ch_i[bank].comp ^ F(hist, M, bank);

		return ((index ^ (index >> logg[bank]) ^ (index >> 2 * logg[bank])) &
						((1 << (logg[bank])) - 1));
	}

	//  tag computation
	uint16_t gtag(unsigned int PC, int bank, folded_history *ch0,
								folded_history *ch1)
	{
		int tag = (ch_i[bank - 1].comp << 2) ^ PC ^ (PC >> 2) ^ (ch_i[bank].comp);
		int M = (m[bank] > PHISTWIDTH) ? PHISTWIDTH : m[bank];
		tag = (tag >> 1) ^ ((tag & 1) << 10) ^ F(phist, M, bank);
		tag ^= ch0[bank].comp ^ (ch1[bank].comp << 1);

		return ((tag ^ (tag >> TB[bank])) & ((1 << (TB[bank])) - 1));
	}

	// up-down saturating counter
	void ctrupdate(int8_t &ctr, bool taken, int nbits)
	{
		if (taken)
		{
			if (ctr < ((1 << (nbits - 1)) - 1))
				ctr++;
		}
		else
		{
			if (ctr > -(1 << (nbits - 1)))
				ctr--;
		}
	}

	bool getbim()
	{
		// BI = bimodal index
		BIM = (btable[BI].pred << 1) + (btable[BI >> HYSTSHIFT].hyst);
		HighConf = (BIM == 0) || (BIM == 3);
		LowConf = !HighConf;
		AltConf = HighConf;
		MedConf = false;
		return (btable[BI].pred > 0);
	}

	void baseupdate(bool Taken)
	{
		int inter = BIM;
		if (Taken)
		{
			if (inter < 3)
				inter += 1;
		}
		else if (inter > 0)
			inter--;
		btable[BI].pred = inter >> 1;
		btable[BI >> HYSTSHIFT].hyst = (inter & 1);
	};

	//just a simple pseudo random number generator: use available information
	// to allocate entries  in the loop predictor
	int MYRANDOM()
	{
		Seed++;
		Seed ^= phist;
		Seed = (Seed >> 21) + (Seed << 11);
		Seed ^= ptghist;
		Seed = (Seed >> 10) + (Seed << 22);
		Seed ^= GTAG[BORN + 2];
		return (Seed);
	};

uint _pred_comes_from;

	//  TAGE PREDICTION: same code at fetch or retire time but the index and tags must recomputed
	void Tagepred(UINT64 PC, int filter=0)
	{
		HitBank = 0;
		AltBank = 0;
		for (int i = 1; i <= NHIST; i += 2)
		{
			GI[i] = gindex(PC, i, phist, ch_i);
			GTAG[i] = gtag(PC, i, ch_t[0], ch_t[1]);
			GTAG[i + 1] = GTAG[i];
			GI[i + 1] = GI[i] ^ (GTAG[i] & ((1 << LOGG) - 1));
		}

		uint T = (PC ^ (phist & ((1 << m[BORN]) - 1))) % NBBANK[1];
		for (int i = BORN; i <= NHIST; i++)
			if (NOSKIP[i])
			{
				GI[i] += (T << LOGG);
				T++;
				T = T % NBBANK[1];
			}
		T = (PC ^ (phist & ((1 << m[1]) - 1))) % NBBANK[0];
		for (int i = 1; i <= BORN - 1; i++)
			if (NOSKIP[i])
			{
				GI[i] += (T << LOGG);
				T++;
				T = T % NBBANK[0];
			}
		//just do not forget most address are aligned on 4 bytes
		// BI = bimodal index
		BI = (PC ^ (PC >> 2)) & ((1 << LOGB) - 1);

		{
			// Get Bimodal prediction (default/base prediction)
			alttaken = _bim_pred = getbim();
			tage_pred = alttaken;
			LongestMatchPred = alttaken;
		}

		if(!filter) {
		//Look for the bank with longest matching history
		for (int i = NHIST; i > 0; i--)
		{
			if (NOSKIP[i])
				if (gtable[i][GI[i]].tag == GTAG[i])
				{
					HitBank = i;
					LongestMatchPred = (gtable[HitBank][GI[HitBank]].ctr >= 0);
					break;
				}
		}

		//Look for the alternate bank
		for (int i = HitBank - 1; i > 0; i--)
		{
			if (NOSKIP[i])
				if (gtable[i][GI[i]].tag == GTAG[i])
				{

					AltBank = i;
					break;
				}
		}
		}

		//computes the prediction and the alternate prediction

		if (HitBank > 0)
		{
			if (AltBank > 0)
			{
				alttaken = _tage_pred = (gtable[AltBank][GI[AltBank]].ctr >= 0); _pred_comes_from = 3;
				AltConf = (abs(2 * gtable[AltBank][GI[AltBank]].ctr + 1) > 1);
			}
			else
				alttaken = getbim();

			//if the entry is recognized as a newly allocated entry and
			//USE_ALT_ON_NA is positive  use the alternate prediction

			bool Huse_alt_on_na = (use_alt_on_na[INDUSEALT] >= 0);
			if ((!Huse_alt_on_na) || (abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) > 1)) {
				tage_pred = _tage_pred = LongestMatchPred;  _pred_comes_from = 3;
			}else
				tage_pred = alttaken;

			HighConf =
					(abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) >= (1 << CWIDTH) - 1);     // >= 7 (-4 or 3)
			LowConf = (abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) == 1); // (-1 or 0)
			MedConf = (abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) == 5); // (-3 or 2)  
		}
	}

	
	//compute the prediction
	bool GetPrediction(UINT64 PC, int filter)
	{
		// computes the TAGE table addresses and the partial tags

		_pred_comes_from = 0;
		Tagepred(PC, filter);
		pred_taken = tage_pred;
#ifndef SC
		return (tage_pred);
#endif

#ifdef LOOPPREDICTOR
		predloop = _loop_pred = getloop(PC); // loop prediction
		if ((WITHLOOP >= 0) && (LVALID)) {
			pred_taken = predloop;
			_pred_comes_from = 1;
		}
		// pred_taken = () ? predloop : pred_taken;
#endif
		pred_inter = pred_taken;

		//Compute the SC prediction

		LSUM = 0;

		//integrate BIAS prediction
		int8_t ctr = Bias[INDBIAS]; // First table is indexed with PC, LowConf, direction

		LSUM += (2 * ctr + 1);
		ctr = BiasSK[INDBIASSK];
		LSUM += (2 * ctr + 1);
		ctr = BiasBank[INDBIASBANK];
		LSUM += (2 * ctr + 1);
#ifdef VARTHRES
		LSUM = (1 + (WB[INDUPDS] >= 0)) * LSUM;
#endif
		//integrate the GEHL predictions

#ifdef GSC
		LSUM += Gpredict(PC, GHIST, Gm, GGEHL, GNB, LOGGNB, WG);
		LSUM += Gpredict(PC, BWHIST, BWm, BWGEHL, BWNB, LOGBWNB, WBW);
#endif
#ifdef LOCALH
		LSUM += Gpredict(PC, L_shist[INDLOCAL], Lm, LGEHL, LNB, LOGLNB, WL);
#endif
#ifdef IMLI
		LSUM += Gpredict(PC, IMLIcount, Im, IGEHL, INB, LOGINB, WI);

#endif

		// get the component with the highest impact.
		if (warmup_complete[parent_cpu->cpu])
		{
			int highest = 0;
			int ctr = abs(Bias[INDBIAS]);
			int tmp = abs(BiasSK[INDBIASSK]);
			if (tmp > ctr) { ctr = tmp; highest = 1; }
			tmp = abs(BiasBank[INDBIASBANK]);
			if (tmp > ctr) { ctr = tmp; highest = 2; }

			tmp = abs(Gpredict(PC, GHIST, Gm, GGEHL, GNB, LOGGNB, WG));
			if (tmp > ctr) { ctr = tmp; highest = 3; }
			tmp = abs(Gpredict(PC, BWHIST, BWm, BWGEHL, BWNB, LOGBWNB, WBW));
			if (tmp > ctr) { ctr = tmp; highest = 4; }
			tmp = abs(Gpredict(PC, L_shist[INDLOCAL], Lm, LGEHL, LNB, LOGLNB, WL));
			if (tmp > ctr) { ctr = tmp; highest = 5; }
			tmp = abs(Gpredict(PC, IMLIcount, Im, IGEHL, INB, LOGINB, WI));
			if (tmp > ctr) { ctr = tmp; highest = 6; }

			switch (highest) {
				case 0: parent_cpu->bte->second.sc_info.bias++;
				case 1: parent_cpu->bte->second.sc_info.biassk++;
				case 2: parent_cpu->bte->second.sc_info.biasbank++;
				case 3: parent_cpu->bte->second.sc_info.ghist++;
				case 4: parent_cpu->bte->second.sc_info.bwhist++;
				case 5: parent_cpu->bte->second.sc_info.lhist++;
				case 6: parent_cpu->bte->second.sc_info.imli++;
			}
		}



		bool SCPRED = _sc_pred = (LSUM >= 0);
		THRES = Pupdatethreshold[INDUPD] + (updatethreshold >> 3);

#ifdef VARTHRES
		+6 * ((WB[INDUPDS] >= 0)
#ifdef LOCALH
					+ (WL[INDUPDS] >= 0)
#endif
#ifdef GSC
					+ (WG[INDUPDS] >= 0) + (WBW[INDUPDS] >= 0)
#endif
#ifdef IMLI
					+ (WI[INDUPDS] >= 0)
#endif
				 )
#endif
				;
		//Minimal benefit in trying to exploit high confidence on TAGE
		if (pred_inter != SCPRED)
		{
			//Choser uses TAGE confidence and |LSUM|
			uint tmp = _pred_comes_from;
			pred_taken = SCPRED;	_pred_comes_from = 2;
		
			if (HighConf)
			{
				if ((abs(LSUM) < THRES / 4))
				{
					pred_taken = pred_inter; _pred_comes_from = tmp;
				}

				else if ((abs(LSUM) < THRES / 2))
					if (SecondH < 0) {
						pred_taken = SCPRED; _pred_comes_from = 2;
					} else {
						pred_taken = pred_inter; _pred_comes_from = tmp;
					}
					
			}

			if (MedConf)
				if ((abs(LSUM) < THRES / 4))
				{
					if (FirstH < 0) {
						pred_taken = SCPRED; _pred_comes_from = 2;
					} else {
						pred_taken = pred_inter; _pred_comes_from = tmp;
					}
				}
		}

		return pred_taken;
	}

	void HistoryUpdate(UINT64 PC, OpType opType, bool taken,
										 UINT64 target, long long &X, int &Y,
										 folded_history *H, folded_history *G,
										 folded_history *J)
	{
		int brtype = 0;

		switch (opType)
		{
		case OPTYPE_RET_UNCOND:
		case OPTYPE_JMP_INDIRECT_UNCOND:
		case OPTYPE_JMP_INDIRECT_COND:
		case OPTYPE_CALL_INDIRECT_UNCOND:
		case OPTYPE_CALL_INDIRECT_COND:
		case OPTYPE_RET_COND:
			brtype = 2;
			break;
		case OPTYPE_JMP_DIRECT_COND:
		case OPTYPE_CALL_DIRECT_COND:
		case OPTYPE_JMP_DIRECT_UNCOND:
		case OPTYPE_CALL_DIRECT_UNCOND:
			brtype = 0;
			break;
		default:
			exit(1);
		}
		switch (opType)
		{
		case OPTYPE_JMP_DIRECT_COND:
		case OPTYPE_CALL_DIRECT_COND:
		case OPTYPE_JMP_INDIRECT_COND:
		case OPTYPE_CALL_INDIRECT_COND:
		case OPTYPE_RET_COND:
			brtype += 1; /// Conditional inst. are odd
			break;
		}

		//special treatment for indirect  branchs;
		int maxt = 2;
		if (brtype & 1)
			maxt = 2;
		else if (brtype & 2)
			maxt = 3;
#ifdef IMLI
		if (brtype & 1)  /// Conditional
		{
			if (target < PC)

			{
				//This branch corresponds to a loop
				if (!taken)
				{
					//exit of the "loop"
					IMLIcount = 0;
				}
				if (taken)
				{

					if (IMLIcount < ((1 << Im[0]) - 1))
						IMLIcount++;
				}
			}
		}

#endif

		if (brtype & 1)
		{
			BWHIST = (BWHIST << 1) + ((target < PC) & taken);
			GHIST = (GHIST << 1) + (taken);
			L_shist[INDLOCAL] = (L_shist[INDLOCAL] << 1) + taken;
		}

		int T = ((PC ^ (PC >> 2))) ^ taken;
		int PATH = PC ^ (PC >> 2) ^ (PC >> 4);
		if ((brtype == 3) & taken)  // for indirect conditional we add also the target
		{
			T = (T ^ (target >> 2));
			PATH = PATH ^ (target >> 2) ^ (target >> 4);
		}

		for (int t = 0; t < maxt; t++)
		{
			bool DIR = (T & 1);
			T >>= 1;
			int PATHBIT = (PATH & 127); // mask 7 bit
			PATH >>= 1;
			//update  history
			Y--;   // Y is the actual pointer in the history table
			ghist[Y & (HISTBUFFERLENGTH - 1)] = DIR;
			X = (X << 1) ^ PATHBIT;

			for (int i = 1; i <= NHIST; i++)
			{

				H[i].update(ghist, Y);
				G[i].update(ghist, Y);
				J[i].update(ghist, Y);
			}
		}

		//END UPDATE  HISTORIES
	}

	// PREDICTOR UPDATE

	void UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir,
											 bool predDir, UINT64 branchTarget, int filter)
	{
		
		// Write some statistics for the current branch.
		// where does the prediction come from?
		bool _t = warmup_complete[parent_cpu->cpu];
		if (_t) {
			switch (_pred_comes_from) {
				case 1:
					parent_cpu->bte->second.tage_info.loop_pred++;
					break;
				case 2:
					parent_cpu->bte->second.tage_info.sc_pred++;
					break;
				case 3:
					parent_cpu->bte->second.tage_info.tage_pred++;
					break;
				default:
					parent_cpu->bte->second.tage_info.bim_pred++;
					break;
			}
			if (resolveDir != _loop_pred)
				parent_cpu->bte->second.tage_info.loop_miss++;
			if (resolveDir != _sc_pred)
				parent_cpu->bte->second.tage_info.sc_miss++;
			if (resolveDir != _tage_pred)
				parent_cpu->bte->second.tage_info.tage_miss++;
			if (resolveDir != _bim_pred)
				parent_cpu->bte->second.tage_info.bim_miss++;
		}



		// For filter we update the history only but not any of the tables.
		// if (filter) {
		// 	HistoryUpdate(PC, opType, resolveDir, branchTarget,
		// 								phist, ptghist, ch_i, ch_t[0], ch_t[1]);
		// 	return;
		// }

		// parent_cpu->bte->second.tage_info



#ifdef SC
#ifdef LOOPPREDICTOR
		if (LVALID)
		{
			if (pred_taken != predloop)
				ctrupdate(WITHLOOP, (predloop == resolveDir), 7);
		}
		loopupdate(PC, resolveDir, (pred_taken != resolveDir));
#endif

		bool SCPRED = (LSUM >= 0);
		
		// Update the statical corrector if the
		if (pred_inter != SCPRED)
		{
			
			if ((abs(LSUM) < THRES))
				if ((HighConf))
				{

					if ((abs(LSUM) < THRES / 2))
						if ((abs(LSUM) >= THRES / 4))
							ctrupdate(SecondH, (pred_inter == resolveDir), CONFWIDTH);
				}
			if ((MedConf))
				if ((abs(LSUM) < THRES / 4))
				{
					ctrupdate(FirstH, (pred_inter == resolveDir), 7);
				}
		}

		// if statical corrector was not correct
		if ((SCPRED != resolveDir) || ((abs(LSUM) < THRES)))
		{
			{
				if (SCPRED != resolveDir)
				{
					Pupdatethreshold[INDUPD] += 1;
					updatethreshold += 1;
				}

				else
				{
					Pupdatethreshold[INDUPD] -= 1;
					updatethreshold -= 1;
				}

				if (Pupdatethreshold[INDUPD] >= (1 << (WIDTHRESP - 1)))
					Pupdatethreshold[INDUPD] = (1 << (WIDTHRESP - 1)) - 1;
				//Pupdatethreshold[INDUPD] could be negative
				if (Pupdatethreshold[INDUPD] < -(1 << (WIDTHRESP - 1)))
					Pupdatethreshold[INDUPD] = -(1 << (WIDTHRESP - 1));
				if (updatethreshold >= (1 << (WIDTHRES - 1)))
					updatethreshold = (1 << (WIDTHRES - 1)) - 1;
				//updatethreshold could be negative
				if (updatethreshold < -(1 << (WIDTHRES - 1)))
					updatethreshold = -(1 << (WIDTHRES - 1));
			}
#ifdef VARTHRES
			{
				int XSUM =
						LSUM - ((WB[INDUPDS] >= 0) * ((2 * Bias[INDBIAS] + 1) +
																					(2 * BiasSK[INDBIASSK] + 1) +
																					(2 * BiasBank[INDBIASBANK] + 1)));
				if ((XSUM +
								 ((2 * Bias[INDBIAS] + 1) + (2 * BiasSK[INDBIASSK] + 1) +
									(2 * BiasBank[INDBIASBANK] + 1)) >=
						 0) != (XSUM >= 0))
					ctrupdate(WB[INDUPDS],
										(((2 * Bias[INDBIAS] + 1) +
													(2 * BiasSK[INDBIASSK] + 1) +
													(2 * BiasBank[INDBIASBANK] + 1) >=
											0) == resolveDir),
										EWIDTH);
			}
#endif
			ctrupdate(Bias[INDBIAS], resolveDir, PERCWIDTH);
			ctrupdate(BiasSK[INDBIASSK], resolveDir, PERCWIDTH);
			ctrupdate(BiasBank[INDBIASBANK], resolveDir, PERCWIDTH);
			Gupdate(PC, resolveDir, GHIST, Gm, GGEHL, GNB, LOGGNB, WG);
			Gupdate(PC, resolveDir, BWHIST, BWm, BWGEHL, BWNB, LOGBWNB, WBW);
#ifdef LOCALH
			Gupdate(PC, resolveDir, L_shist[INDLOCAL], Lm, LGEHL, LNB,
							LOGLNB, WL);
#endif
#ifdef IMLI

			Gupdate(PC, resolveDir, IMLIcount, Im, IGEHL, INB, LOGINB, WI);
#endif // IMLI
		}
#endif // SC


		// Misspredicted
		if (_t)	parent_cpu->bte->second.tage_info.hit_bank += HitBank; // Later an average can be calulated

		
		//TAGE UPDATE
		bool ALLOC = ((tage_pred != resolveDir) & (HitBank < NHIST));
		if (pred_taken == resolveDir)
			if ((MYRANDOM() & 31) != 0)
				ALLOC = false;
		//do not allocate too often if the overall prediction is correct
		if (HitBank > 0)
		{
			// Manage the selection between longest matching and alternate matching
			// for "pseudo"-newly allocated longest matching entry
			// this is extremely important for TAGE only (0.166 MPKI), not that important when the overall predictor is implemented (0.006 MPKI)
			bool PseudoNewAlloc =
					(abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) <= 1);
			// an entry is considered as newly allocated if its prediction counter is weak
			if (PseudoNewAlloc)
			{
				if (LongestMatchPred == resolveDir)
					ALLOC = false;
				// if it was delivering the correct prediction, no need to allocate a new entry
				//even if the overall prediction was false
				if (LongestMatchPred != alttaken)
				{
					ctrupdate(use_alt_on_na[INDUSEALT],
										(alttaken == resolveDir), ALTWIDTH);
				}
			}
		}
		// Test what happen if we do not allocate hardest to predict branches.
		if (filter) {
			ALLOC=false;
		}

		if (ALLOC)
		{
			int T = NNN;

			int A = 1;
			if ((MYRANDOM() & 127) < 32)
				A = 2;
			int Penalty = 0;
			int TruePen = 0;
			int NA = 0;   // number of allocations
			int DEP = ((((HitBank - 1 + 2 * A) & 0xffe)) ^ (MYRANDOM() & 1));

			for (int I = DEP; I < NHIST; I += 2)
			{
				int i = I + 1;
				bool Done = false;
				if (NOSKIP[i])  // Bank interleaving
				{
					if ((gtable[i][GI[i]].u == 0))
					{
						{

#define DIPINSP
#ifdef DIPINSP

							gtable[i][GI[i]].u = ((MYRANDOM() & 31) == 0);
// protect randomly from fast replacement
#endif
							gtable[i][GI[i]].tag = GTAG[i];
							gtable[i][GI[i]].ctr = (resolveDir) ? 0 : -1;
							gtable[i][GI[i]].full_pc = PC;
							NA++;
							if (T <= 0)  // are 1 + NNN (extra entries) per missprediction allocated?
							{
								break;
							}
							I += 2;
							T -= 1;
						}
					}
					else
					{
#ifdef DIPINSP
						if ((gtable[i][GI[i]].u ==
								 1) &
								(abs(2 * gtable[i][GI[i]].ctr + 1) == 1))
						{
							if ((MYRANDOM() & 7) == 0)
								gtable[i][GI[i]].u = 0;
						}
						else
#endif
							TruePen++;
						Penalty++;
					}

					if (!Done)    // if not already allocated one entry
					{
						i = (I ^ 1) + 1;
						if (NOSKIP[i])
						{
							if ((gtable[i][GI[i]].u == 0))
							{
#ifdef DIPINSP
								gtable[i][GI[i]].u = ((MYRANDOM() & 31) == 0);
#endif
								gtable[i][GI[i]].tag = GTAG[i];
								gtable[i][GI[i]].ctr = (resolveDir) ? 0 : -1;
								gtable[i][GI[i]].full_pc = PC;
								NA++;
								if (T <= 0)
								{
									break;
								}
								I += 2;
								T -= 1;
							}

							else
							{
#ifdef DIPINSP
								if ((gtable[i][GI[i]].u ==
										 1) &
										(abs(2 * gtable[i][GI[i]].ctr + 1) ==
										 1))
								{
									if ((MYRANDOM() & 7) == 0)
										gtable[i][GI[i]].u = 0;
								}
								else
#endif
									TruePen++;
								Penalty++;
							}
						}
					}
				}
			}
			TICK += (TruePen + Penalty - 5 * NA);

			//just the best formula for the Championship:
			//In practice when one out of two entries are useful
			if (TICK < 0)
				TICK = 0;
			if (TICK >= BORNTICK)
			{

				for (int i = 1; i <= BORN; i += BORN - 1)
					for (int j = 0; j <= NBBANK[i / BORN] * (1 << logg[i]) - 1; j++)
					{

						//                gtable[i][j].u >>= 1;
						/*this is not realistic: in a real processor:    gtable[i][j].u >>= 1;  */
						if (gtable[i][j].u > 0)
							gtable[i][j].u--;
					}
				TICK = 0;
			}

			// check all entries and count the number of entries used for this branch.	
			if (_t) {

				uint n_entries = 0, n_useful_entries = 0;
				uint n_high_conf = 0, n_med_conf = 0, n_low_conf = 0;
				uint n_tot_high_conf = 0, n_tot_med_conf = 0, n_tot_low_conf = 0;
				
				// First search the lower banks. They are in subsequent order starting at 1
				int base = 1, size = NBBANK[0] * (1 << LOGG);
				for (int i = 0; i < size; i++) {
					
					if (gtable[base][i].full_pc == PC) {
						n_entries++;
						if(gtable[base][i].u != 0) {
							n_useful_entries++;
						}
						// Get confidence
						int conf = abs(2 * gtable[base][i].ctr + 1);
						if (conf >= (1 << CWIDTH) - 1) n_high_conf++;
						if (conf == 1) n_med_conf++;
						if (conf == 5) n_low_conf++;
					}
				}
				// Secondly search the higher banks. As well in subsequent order starting at BORN
				base = BORN; size = NBBANK[1] * (1 << LOGG);
				for (int i = 0; i < size; i++) {
					if (gtable[base][i].full_pc == PC) {
						n_entries++;
						if(gtable[base][i].u != 0) {
							n_useful_entries++;
						}
						// Get confidence
						int conf = abs(2 * gtable[base][i].ctr + 1);
						if (conf >= (1 << CWIDTH) - 1) n_high_conf++;
						if (conf == 1) n_med_conf++;
						if (conf == 5) n_low_conf++;
					}
				}

				parent_cpu->bte->second.tage_info.n_alloc++;
				parent_cpu->bte->second.tage_info.n_entries_alloc += NA;
				parent_cpu->bte->second.tage_info.utilization += n_entries;
				parent_cpu->bte->second.tage_info.n_useful_entries += n_useful_entries;
				
				parent_cpu->bte->second.tage_info.n_high_conf += n_high_conf;
				parent_cpu->bte->second.tage_info.n_med_conf += n_med_conf;
				parent_cpu->bte->second.tage_info.n_low_conf += n_low_conf;
				
				if (parent_cpu->bte->second.tage_info.max_util < n_entries) {
					parent_cpu->bte->second.tage_info.max_util = n_entries;
				}
			} 
		} /// End allocate

		//update predictions the already existing entry.
		if (HitBank > 0)
		{
			if (abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) == 1) // Low confidence
				if (LongestMatchPred != resolveDir)
				{ // acts as a protection
					if (AltBank > 0)
					{
						if (abs(2 * gtable[AltBank][GI[AltBank]].ctr + 1) == 1)
							gtable[AltBank][GI[AltBank]].u = 0;
						//just mute from protected to unprotected
						ctrupdate(gtable[AltBank][GI[AltBank]].ctr,
											resolveDir, CWIDTH);
						if (abs(2 * gtable[AltBank][GI[AltBank]].ctr + 1) == 1)
							gtable[AltBank][GI[AltBank]].u = 0;
					}
					if (AltBank == 0)
						baseupdate(resolveDir);
				}
			if (abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) == 1)  // Low confidence set the usefulness to 0
				gtable[HitBank][GI[HitBank]].u = 0;
			//just mute from protected to unprotected
			ctrupdate(gtable[HitBank][GI[HitBank]].ctr, resolveDir, CWIDTH);
			//sign changes: no way it can have been useful
			if (abs(2 * gtable[HitBank][GI[HitBank]].ctr + 1) == 1)  // transition from 0 to -1 or -1 to 0
				gtable[HitBank][GI[HitBank]].u = 0;
			if (alttaken == resolveDir)
				if (AltBank > 0)
					if (abs(2 * gtable[AltBank][GI[AltBank]].ctr + 1) == 7)
						if (gtable[HitBank][GI[HitBank]].u == 1)
						{
							if (LongestMatchPred == resolveDir)
							{
								gtable[HitBank][GI[HitBank]].u = 0;
							}
						}
		}
		else
			baseupdate(resolveDir);

		if (LongestMatchPred != alttaken)
			if (LongestMatchPred == resolveDir)
			{
				if (gtable[HitBank][GI[HitBank]].u < (1 << UWIDTH) - 1)
					gtable[HitBank][GI[HitBank]].u++;
			}
		//END TAGE UPDATE

		HistoryUpdate(PC, opType, resolveDir, branchTarget,
									phist, ptghist, ch_i, ch_t[0], ch_t[1]);

		//END PREDICTOR UPDATE
	}


	void TrackOtherInst(UINT64 PC, OpType opType, bool taken,
											UINT64 branchTarget)
	{

		HistoryUpdate(PC, opType, taken, branchTarget, phist,
									ptghist, ch_i, ch_t[0], ch_t[1]);
	}













// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Statistical Corrector
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




#define GINDEX (((long long)PC) ^ bhist ^ (bhist >> (8 - i)) ^ (bhist >> (16 - 2 * i)) ^ (bhist >> (24 - 3 * i)) ^ (bhist >> (32 - 3 * i)) ^ (bhist >> (40 - 4 * i))) & ((1 << logs) - 1)
	int Gpredict(UINT64 PC, long long BHIST, int *length,
							 int8_t **tab, int NBR, int logs, int8_t *W)
	{
		int PERCSUM = 0;
		for (int i = 0; i < NBR; i++)
		{
			long long bhist = BHIST & ((long long)((1 << length[i]) - 1));
			long long index = GINDEX;

			int8_t ctr = tab[i][index];

			PERCSUM += (2 * ctr + 1);
		}
#ifdef VARTHRES
		PERCSUM = (1 + (W[INDUPDS] >= 0)) * PERCSUM;
#endif
		return ((PERCSUM));
	}
	void Gupdate(UINT64 PC, bool taken, long long BHIST, int *length,
							 int8_t **tab, int NBR, int logs, int8_t *W)
	{

		int PERCSUM = 0;

		for (int i = 0; i < NBR; i++)
		{
			long long bhist = BHIST & ((long long)((1 << length[i]) - 1));
			long long index = GINDEX;

			PERCSUM += (2 * tab[i][index] + 1);
			ctrupdate(tab[i][index], taken, PERCWIDTH);
		}
#ifdef VARTHRES
		{
			int XSUM = LSUM - ((W[INDUPDS] >= 0)) * PERCSUM;
			if ((XSUM + PERCSUM >= 0) != (XSUM >= 0))
				ctrupdate(W[INDUPDS], ((PERCSUM >= 0) == taken), EWIDTH);
		}
#endif
	}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LOOP Predictor
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef LOOPPREDICTOR
	int lindex(UINT64 PC)
	{
		return (((PC ^ (PC >> 2)) & ((1 << (LOGL - 2)) - 1)) << 2);
	}

//loop prediction: only used if high confidence
//skewed associative 4-way
//At fetch time: speculative
#define CONFLOOP 15   // 4-bit counter = 0-15
	bool getloop(UINT64 PC)
	{
		LHIT = -1;
		LI = lindex(PC);
		LIB = ((PC >> (LOGL - 2)) & ((1 << (LOGL - 2)) - 1));
		LTAG = (PC >> (LOGL - 2)) & ((1 << 2 * LOOPTAG) - 1);
		LTAG ^= (LTAG >> LOOPTAG);
		LTAG = (LTAG & ((1 << LOOPTAG) - 1));

		for (int i = 0; i < 4; i++) // 4 times because of four way associativity
		{
			int index = (LI ^ ((LIB >> i) << 2)) + i;

			if (ltable[index].TAG == LTAG)
			{
				LHIT = i;
				LVALID = ((ltable[index].confid == CONFLOOP) || (ltable[index].confid * ltable[index].NbIter > 128));

				if (ltable[index].CurrentIter + 1 == ltable[index].NbIter)
					return (!(ltable[index].dir));
				return ((ltable[index].dir));
			}
		}

		LVALID = false;
		return (false);
	}

	void loopupdate(UINT64 PC, bool Taken, bool ALLOC)
	{
		if (LHIT >= 0)
		{
			int index = (LI ^ ((LIB >> LHIT) << 2)) + LHIT;
			//already a hit
			if (LVALID)
			{
				if (Taken != predloop)
				{
					// free the entry
					ltable[index].NbIter = 0;
					ltable[index].age = 0;
					ltable[index].confid = 0;
					ltable[index].CurrentIter = 0;
					return;
				}
				else if ((predloop != tage_pred) || ((MYRANDOM() & 7) == 0))
					if (ltable[index].age < CONFLOOP)
						ltable[index].age++;
			}

			ltable[index].CurrentIter++;
			ltable[index].CurrentIter &= ((1 << WIDTHNBITERLOOP) - 1); // 10 bit = 0-1023
			//loop with more than 2** WIDTHNBITERLOOP iterations are not treated correctly; but who cares :-)
			if (ltable[index].CurrentIter > ltable[index].NbIter)
			{
				ltable[index].confid = 0;
				ltable[index].NbIter = 0;
				//treat like the 1st encounter of the loop
			}
			if (Taken != ltable[index].dir) // The direction was different then the entry point...
			{
				if (ltable[index].CurrentIter == ltable[index].NbIter) // ... because it was was correcty predicted to terminate at this interation count
				{
					if (ltable[index].confid < CONFLOOP)
						ltable[index].confid++;
					if (ltable[index].NbIter < 3)
					//just do not predict when the loop count is 1 or 2
					{
						// free the entry
						ltable[index].dir = Taken;
						ltable[index].NbIter = 0;
						ltable[index].age = 0;
						ltable[index].confid = 0;
					}
				}
				else // ...because it was not correct predicted.
				{
					if (ltable[index].NbIter == 0)
					{
						// first complete nest;
						ltable[index].confid = 0;
						ltable[index].NbIter = ltable[index].CurrentIter;
					}
					else
					{
						//not the same number of iterations as last time: free the entry
						ltable[index].NbIter = 0;
						ltable[index].confid = 0;
					}
				}
				ltable[index].CurrentIter = 0;
			}
		}

		else if (ALLOC)
		{
			UINT64 X = MYRANDOM() & 3;

			if ((MYRANDOM() & 3) == 0)
				for (int i = 0; i < 4; i++) // For way associativity
				{
					int LHIT = (X + i) & 3;
					int index = (LI ^ ((LIB >> LHIT) << 2)) + LHIT;
					if (ltable[index].age == 0) // An entry can be replaced only if its age counter is null.
					{
						ltable[index].dir = !Taken;
						// most of mispredictions are on last iterations
						ltable[index].TAG = LTAG;
						ltable[index].NbIter = 0;
						ltable[index].age = 7;
						ltable[index].confid = 0;
						ltable[index].CurrentIter = 0;
						break;
					}
					else
						ltable[index].age--;
					break;
				}
		}
	}
#endif
};

#endif
