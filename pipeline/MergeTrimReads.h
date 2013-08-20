#ifndef MergeTrimReads_h
#define MergeTrimReads_h

#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <vector>
#include <cfloat>
#include <math.h>
#include <sys/time.h>

#include "utils.h"

using namespace std;

extern  double maxLikelihoodRatio  ;

extern  double likelihoodChimera  ;
extern double likelihoodAdapterSR ;
/* static const double likelihoodAdapterPR = -1.0; */
extern double likelihoodAdapterPR ;


extern const size_t min_length ;
extern const int    qualOffset ;

/* static const double max_prob_N = 0.25; */

/* extern double cutoff_merge_trim; */
extern size_t maxadapter_comp;

extern size_t min_overlap_seqs;


/* //  Key variables /// */
extern bool handle_key;
extern bool options_allowMissing;
extern string keys0;
extern string keys1;
extern int len_key1;
extern int len_key2;
extern size_t options_trimCutoff;
extern bool options_mergeoverlap;
extern double max_prob_N ;
/* extern size_t min_length ; */

//Chimera options and adapter
extern  const char* const chimInit[];/* = { */
    /* "ACACTCTTTCCCTACACGTCTGAACTCCAG", */
    /* "ACACTCTTTCCCACACGTCTGAACTCCAGT", */
    /* "ACACTCTTTCCCTACACACGTCTGAACTCC", */
    /* "CTCTTTCCCTACACGTCTGAACTCCAGTCA", */
    /* "GAAGAGCACACGTCTGAACTCCAGTCACII", */
    /* "GAGCACACGTCTGAACTCCAGTCACIIIII", */
    /* "GATCGGAAGAGCACACGTCTGAACTCCAGT", */
    /* "AGATCGGAAGAGCACACGTCTGAACTCCAG", */
    /* "AGAGCACACGTCTGAACTCCAGTCACIIII", */
    /* "ACACGTCTGAACTCCAGTCACIIIIIIIAT", */
    /* "GTGCACACGTCTGAACTCCAGTCACIIIII", */
    /* "AGCACACGTCTGAACTCCAGTCACIIIIII", */
    /* "CGTATGCCGTCTTCTGCTTGAAAAAAAAAA"}; */
     
extern vector<string> adapter_chimeras ;
extern string options_adapter_F;
extern string options_adapter_S;

string returnFirstToken(string * toparse,string delim);

/* typedef struct { */
/*     char   base; */
/*     int    qual; */
/*     double prob; */
/* } baseQual; */

/* typedef struct { */
/*     string         sequence; */
/*     string         quality; */
/*     vector<double> probabilities; */
/*     vector<int>    logProbs; */
/* } seqQual; */

typedef struct{
    char   code;
    string sequence;
    string quality;
} merged;

typedef struct {
    char   base;
    int    qual;
    double prob;
} baseQual;

/* static inline int edits(const string & seq1,const string & seq2); */
/* static string returnFirstToken(string * toparse,string delim); */

void    setLikelihoodScores(double likelihoodChimera_,
			    double likelihoodAdapterSR_,
			    double likelihoodAdapterPR_);

void set_options(int trimcutoff=1,bool allowMissing=false,bool mergeoverlap=false);
void set_adapter_sequences(const string& forward="", const string& reverse="", const string& chimera="");
void set_keys(const string& key1, const string& key2="");
void initMerge();
merged process_PE(string  read1,string  qual1,string read2,string qual2);
merged process_SR(string  read1, string qual1);

/* static inline double detectChimera(const string      & read, */
/* 			   const vector<int> & qual, */
/* 			   const string      & chimeraString, */
/* 			   unsigned int offsetChimera=0); */

/* static inline double detectAdapter(const string      & read, */
/* 				   const vector<int> & qual, */
/* 				   const string      & adapterString, */
/* 				   unsigned int offsetRead=0, */
/* 				   double * iterations=0); */

/* static inline double measureOverlap(const string      & read1, */
/* 				    const vector<int> & qual1, */
/* 				    const string      & read2, */
/* 				    const vector<int> & qual2, */
/* 				    const  int startRead1, */
/* 				    const  int startRead2,				    */
/* 				    int	maxLength, */
/* 				    /\* const unsigned int startRead,				    *\/ */
/* 				    /\* const unsigned int endRead, *\/ */
/* 				    /\* const unsigned int	    maxLengthForPair, *\/ */
/* 				    double * iterations=0); */


/* def convert_quality_logprob(qualstring): */
/* def revcompl(seq): */
/* def cons_base_prob(base1,base2,prob1,prob2): */
/* static void process_PE(const string  read1,const string qual1,const string read2,const string qual2); */
/* static void process_SR(const string  read1,const string qual1); */

/* class MergeTrimReads{ */
/* private: */

/* public: */

/* }; */
#endif
