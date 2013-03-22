// vim:ts=8
#include <iostream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sstream>
#include <map>


#include <api/SamHeader.h>
#include <api/BamMultiReader.h>
#include <api/BamReader.h>
#include <api/BamWriter.h>
#include <api/BamAux.h>
#include "RGAssign.h"
#include "PutProgramInHeader.h"


#include "utils.h"

using namespace std;
using namespace BamTools;


/****************************************/
/*                                      */
/*          STATIC VARIABLES            */
/*                                      */
/****************************************/


static double rgScoreCutoff  = 80 ;             // two HQ bases can mismatch
static double fracConflict   = 20 ;             // top shall be 100x more likely
static double wrongness      = 0 ;              // 50% chance of being wrong
static int    mismatchesTrie = 2;
static int      maxErrorHits = 20;
PrefixTree<string> * trieKnownString;
static string dashes = "--------------------------------";

static const string  p7_block300[] = {"TCGCAGG",
				      "CTCTGCA",
				      "CCTAGGT",
				      "GGATCAA",
				      "GCAAGAT",
				      "ATGGAGA",
				      "CTCGATG",
				      "GCTCGAA",
				      "ACCAACT",
				      "CCGGTAC",
				      "AACTCCG",
				      "TTGAAGT",
				      "ACTATCA",
				      "TTGGATC",
				      "CGACCTG",
				      "TAATGCG",
				      "AGGTACC",
				      "TGCGTCC",
				      "GAATCTC",
				      "CATGCTC",
				      "ACGCAAC",
				      "GCATTGG",
				      "GATCTCG",
				      "CAATATG",
				      "TGACGTC",
				      "GATGCCA",
				      "CAATTAC",
				      "AGATAGG",
				      "CCGATTG",
				      "ATGCCGC",
				      "CAGTACT",
				      "AATAGTA",
				      "CATCCGG",
				      "TCATGGT",
				      "AGAACCG",
				      "TGGAATA",
				      "CAGGAGG",
				      "AATACCT",
				      "CGAATGC",
				      "TTCGCAA",
				      "AATTCAA",
				      "CGCGCAG",
				      "AAGGTCT",
				      "ACTGGAC",
				      "AGCAGGT",
				      "GTACCGG",
				      "GGTCAAG",
				      "AATGATG",
				      "AGTCAGA",
				      "AACTAGA",
				      "CTATGGC",
				      "CGACGGT",
				      "AACCAAG",
				      "CGGCGTA",
				      "GCAGTCC",
				      "CTCGCGC",
				      "CTGCGAC",
				      "ACGTATG",
				      "ATACTGA",
				      "TACTTAG",
				      "AAGCTAA",
				      "GACGGCG",
				      "AGAAGAC",
				      "GTCCGGC",
				      "TCAGCTT",
				      "AGAGCGC",
				      "GCCTACG",
				      "TAATCAT",
				      "AACCTGC",
				      "GACGATT",
				      "TAGGCCG",
				      "GGCATAG",
				      "TTCAACC",
				      "TTAACTC",
				      "TAGTCTA",
				      "TGCATGA",
				      "AATAAGC",
				      "AGCCTTG",
				      "CCAACCT",
				      "GCAGAAG",
				      "AGAATTA",
				      "CAGCATC",
				      "TTCTAGG",
				      "CCTCTAG",
				      "CCGGATA",
				      "GCCGCCT",
				      "AACGACC",
				      "CCAGCGG",
				      "TAGTTCC",
				      "TGGCAAT",
				      "CGTATAT",
				      "GCTAATC",
				      "GACTTCT",
				      "GTACTAT",
				      "CGAGATC",
				      "CGCAGCC"};

static const string  p7_block1[] = { "ACAGTG",
				     "GATCAG",
				     "ATCACG",
				     "CGATGT",
				     "CTTGTA",
				     "GGCTAC",
				     "TGACCA",
				     "AAAGCA",
				     "AAATGC",
				     "AAGCGA",
				     "AAGGAC",
				     "AATAGG",
				     "ACCCAG",
				     "ACTCTC",
				     "AGAAGA",
				     "AGCATC",
				     "AGGCCG",
				     "ATACGG",
				     "ATCCTA",
				     "ATCTAT",
				     "ATGAGC",
				     "CATTTT",
				     "CCGCAA",
				     "CTCAGA",
				     "GAATAA",
				     "GCCGCG",
				     "GCTCCA",
				     "GGCACA",
				     "GGCCTG",
				     "TCGGCA",
				     "TCTACC",
				     "TGCCAT",
				     "TGCTGG",
				     "AGGTTT",
				     "AGTCAA",
				     "AGTTCC",
				     "ATGTCA",
				     "CCGTCC",
				     "GTAGAG",
				     "GTGAAA",
				     "GTGGCC",
				     "GTTTCG",
				     "CGTACG",
				     "GAGTGG",
				     "GGTAGC",
				     "ACTTGA",
				     "CAGATC",
				     "GCCAAT",
				     "TAGCTT",
				     "TTAGGC",
				     "AACCGCC",
				     "AACGAAC",
				     "AACGCCT",
				     "AACGGTA",
				     "AACTAGT",
				     "AACTGAG",
				     "AAGAATT",
				     "AAGATAG",
				     "AAGCTCT",
				     "AAGTCTG",
				     "AATAACC",
				     "AATCCGT",
				     "ACCGATT",
				     "ACCGTAG",
				     "ACCTCAT",
				     "ACCTTGC",
				     "ACGACCT",
				     "ACGATTC",
				     "ACGCGGC",
				     "ACGGAGG",
				     "ACGTAAC",
				     "ACTACTG",
				     "ACTCGTT",
				     "ACTGCGC",
				     "AGACCTC",
				     "AGACTAG",
				     "AGAGACC",
				     "AGAGCGT",
				     "AGATATG",
				     "AGATTCT",
				     "AGCAAGC",
				     "AGCAGTT",
				     "AGCGCTG",
				     "AGTATAC",
				     "ATAAGTC",
				     "ATAATGG",
				     "ATACTCC",
				     "ATAGAAG",
				     "ATCTCCG",
				     "ATGCAGT",                                           
				     "ATGGTAT",
				     "ATTATCT",
				     "ATTCGAC",
				     "ATTGCTA",
				     "CAACCGG",
				     "CAACTAA",
				     "AATCTTC",
				     "ACCAACG",
				     "AGATGGC",
				     "CCAGGTT",
				     "CCGTTAG",
				     "CGCCTCT",
				     "CTTGCGG",
				     "GGCGGAG",
				     "TGGACGT",
				     "AACCATG",
				     "CAGGAAG",
				     "CATACCT",
				     "CCAATCC",
				     "CCGGCGT",
				     "CGCATAG",
				     "CGTAATC",
				     "CGTTGGT",
				     "CTATACG",
				     "GACCTAC",
				     "GATATTG",
				     "AAGACGC",
				     "GCAGTAT",
				     "GGTCCGC",
				     "GTCGACT",                                           
				     "GTTAGAT",
				     "TAACTCG",
				     "TGCTTCC",
				     "TGGCGCT",
				     "AATGGCG",
				     "ACCAGAC",
				     "ACGCCAG",
				     "ACTAAGT",
				     "AGAACCG",
				     "ATCGTTC",
				     "CAACGTC"};

static const string  p5_block1[] = {"TCGCAGG",
				    "CTCTGCA",
				    "CCTAGGT",
				    "GGATCAA",
				    "GCAAGAT",
				    "ATGGAGA",
				    "CTCGATG",
				    "GCTCGAA",
				    "ACCAACT",
				    "CCGGTAC",
				    "AACTCCG",
				    "TTGAAGT",
				    "ACTATCA",
				    "TTGGATC",
				    "CGACCTG",
				    "TAATGCG",
				    "AGGTACC",
				    "TGCGTCC",
				    "GAATCTC",
				    "CATGCTC",
				    "ACGCAAC",
				    "GCATTGG",
				    "GATCTCG",
				    "CAATATG",
				    "TGACGTC",
				    "GATGCCA",
				    "CAATTAC",
				    "AGATAGG",
				    "CCGATTG",
				    "ATGCCGC",
				    "CAGTACT",
				    "AATAGTA",
				    "CATCCGG",
				    "TCATGGT",
				    "AGAACCG",
				    "TGGAATA",
				    "CAGGAGG",
				    "AATACCT",
				    "CGAATGC",
				    "TTCGCAA",
				    "AATTCAA",
				    "CGCGCAG",
				    "AAGGTCT",
				    "ACTGGAC",
				    "AGCAGGT",
				    "GTACCGG",
				    "GGTCAAG",
				    "AATGATG",
				    "AGTCAGA",
				    "AACTAGA",
				    "CTATGGC",
				    "CGACGGT",
				    "AACCAAG",
				    "CGGCGTA",
				    "GCAGTCC",
				    "CTCGCGC",
				    "CTGCGAC",
				    "ACGTATG",
				    "ATACTGA",
				    "TACTTAG",
				    "AAGCTAA",
				    "GACGGCG",
				    "AGAAGAC",
				    "GTCCGGC",
				    "TCAGCTT",
				    "AGAGCGC",
				    "GCCTACG",
				    "TAATCAT",
				    "AACCTGC",
				    "GACGATT",
				    "TAGGCCG",
				    "GGCATAG",
				    "TTCAACC",
				    "TTAACTC",
				    "TAGTCTA",
				    "TGCATGA",
				    "AATAAGC",
				    "AGCCTTG",
				    "CCAACCT",
				    "GCAGAAG",
				    "AGAATTA",
				    "CAGCATC",
				    "TTCTAGG",
				    "CCTCTAG",
				    "CCGGATA",
				    "GCCGCCT",
				    "AACGACC",
				    "CCAGCGG",
				    "TAGTTCC",
				    "TGGCAAT",
				    "CGTATAT",
				    "GCTAATC",
				    "GACTTCT",
				    "GTACTAT",
				    "CGAGATC",
				    "CGCAGCC"};

static const string  truseq[] = {"ATCACG", //1
				 "CGATGT", //2
				 "TTAGGC", //3
				 "TGACCA", //4
				 "ACAGTG", //5 
				 "GCCAAT", //6
				 "CAGATC", //7
				 "ACTTGA", //8
				 "GATCAG", //9
				 "TAGCTT", //10
				 "GGCTAC", //11
				 "CTTGTA", //12
				 "AGTCAA", //13
				 "AGTTCC", //14
				 "ATGTCA", //15
				 "CCGTCC", //16
				 "NNNNNN", //no 17 
				 "GTCCGC", //18
				 "GTGAAA", //19
				 "GTGGCC", //20
				 "GTTTCG", //21
				 "CGTACG", //22
				 "GAGTGG", //23
				 "NNNNNN", //no 24 
				 "ACTGAT", //25
				 "ATTCCT", //26
				 "NNNNNN"  //no 27
};

// XXX really, a global?!
map<string,int> namesMap; //a map name of RG to count of how many observed

struct compareNameRG {
    bool operator() (pair<string,int> i,pair<string,int> j) { 
        return (i.second>j.second); 
    }
};

static string get_string_field( BamAlignment &al, const char* name ) 
{
    if(al.HasTag(name)) {
        char ttype;
        if( !al.GetTagType(name,ttype) ) {
            cerr << "Unable to get tag type" << endl;
            exit(1);
        }
        if( ttype=='Z' || ttype=='H' ) {
            string tagInfo;
            if(!al.GetTag(name,tagInfo)){
                cerr << "Unable to edit " << name << " tag" << endl;
                exit(1);     
            }
            return tagInfo;
        } else if( ttype=='A' ) {
            int val;
            if( !al.GetTag(name,val) ){
                cerr << "Unable to edit " << name << " tag" << endl;
                exit(1);     
            }
            return string(1,val) ;
        }
    }
    return string() ;
}

static void getIndices( const BamAlignment &al,string & index1,string & index1Q,string & index2,string & index2Q){
    if(!al.GetTag("XI",index1) ){ 	
	cerr << "Cannot retrieve XI field  "<<al.Name << endl;
	exit(1); 
    }
    if(!al.GetTag("YI",index1Q)){ 	
	cerr << "Cannot retrieve YI field  "<<al.Name << endl;
	exit(1); 
    }

    if(!al.GetTag("XJ",index2)) {
        index2 ="";
        index2Q="";
    }
    else if(!al.GetTag("YJ",index2Q)) {
	    cerr << "Cannot retrieve YJ field  "<<al.Name << endl;
	    exit(1); 
    }
}


void updateRecord( BamAlignment &al, const rgAssignment &rg )
{
    // get old ZQ field, remove "ICW"
    string zq = get_string_field(al, "ZQ");
    string::iterator p = zq.begin(), q = zq.begin(), e = zq.end() ;
    while( p != e ) {
        if( *p != 'I' && *p != 'C' && *p != 'W' ) 
        {
            *q = *p ;
            ++q ;
        }
        ++p ;
    }
    zq.erase( q, e ) ;

    if( rg.conflict ) zq += 'C' ;
    if( rg.wrong    ) zq += 'W' ;
    if( rg.unknown  ) zq += 'I' ;

    if( rg.predictedGroup.empty() ) {
        al.RemoveTag("RG");
        al.RemoveTag("Z0");
        al.RemoveTag("Z1");
        al.RemoveTag("Z2");
        zq += 'I';
    } else
    {
        al.EditTag("RG","Z",rg.predictedGroup);
        al.EditTag("Z0","i",(int)round(-10 * rg.logLikelihoodScore));
        if( rg.logRatioTopToSecond <= 0 )
            al.EditTag("Z1","i",(int)round(-10 * rg.logRatioTopToSecond));
        else
            al.RemoveTag("Z1") ;
        if( 10 * rg.topWrongToTopCorrect < rgScoreCutoff ) 
            al.EditTag("Z2","i",(int)round(-10 * rg.topWrongToTopCorrect));
        else
            al.RemoveTag("Z2") ;
        namesMap[ rg.predictedGroup ] ++;
    }

    // store new ZQ field and set FailedQC flag if it isn't empty
    al.SetIsFailedQC( !zq.empty() ) ;
    if( zq.empty() ) al.RemoveTag("ZQ") ;
    else al.EditTag( "ZQ", "Z", zq ) ;
}

inline bool containsNoNs(const string & sN){
    return (sN.find("N") == string::npos);
}

void initializeKnownIndices(PrefixTree<string> * trieKnownString){
    //p7 300
    int sizeArray;
    sizeArray=sizeof(p7_block300)/sizeof(p7_block300[0]);
    for(int i=0;i<sizeArray;i++){
    	if(containsNoNs(p7_block300[i])){
    	    trieKnownString->insertIntoTree( p7_block300[i].c_str() , "p7#"+stringify(i+301));
    	    trieKnownString->insertIntoTree( reverseComplement(p7_block300[i]).c_str() , "p7REVC#"+stringify(i+301));
    	}
    }
    //p7
    sizeArray=sizeof(p7_block1)/sizeof(p7_block1[0]);
    for(int i=0;i<sizeArray;i++){
	if(containsNoNs(p7_block1[i])){
	    trieKnownString->insertIntoTree( p7_block1[i].c_str() , "p7#"+stringify(i+1));
	    trieKnownString->insertIntoTree( reverseComplement(p7_block1[i]).c_str() , "p7REVC#"+stringify(i+1));
	}
    }

    //p5
    sizeArray=sizeof(p5_block1)/sizeof(p5_block1[0]);
    for(int i=0;i<sizeArray;i++){
	if(containsNoNs(p5_block1[i])){
	    trieKnownString->insertIntoTree( p5_block1[i].c_str() , "p5#"+stringify(i+1));
	    trieKnownString->insertIntoTree( reverseComplement(p5_block1[i]).c_str() , "p5REVC#"+stringify(i+1));
	}
    }

    //truseq
    sizeArray=sizeof(truseq)/sizeof(truseq[0]);
    for(int i=0;i<sizeArray;i++){
	if(containsNoNs(truseq[i])){
	    trieKnownString->insertIntoTree( truseq[i].c_str() , "ts#"+stringify(i+1));
	    trieKnownString->insertIntoTree( reverseComplement(truseq[i]).c_str() , "tsREVC#"+stringify(i+1));
	}
    }

    //Other sequences after the primming site
    string IS4="AGATCTC";
    trieKnownString->insertIntoTree( IS4.c_str() ,                    "IS4");
    trieKnownString->insertIntoTree( reverseComplement(IS4).c_str() , "REVC#IS4");


}


void printUnfoundToFile(vector< pair<string,int> > * unfound,ofstream & fileError){

    for(int i=0;i<min(int(unfound->size()),maxErrorHits);i++){	       
	//Searching in known strings
	vector<string> temp = allTokens((*unfound)[i].first,'#');
	vector<string> temp2;
	for(unsigned int j=0;j<temp.size();j++){
	    vector<string> * temp3=new vector<string>();
	    vector<string> * temp4=new vector<string>();

	    trieKnownString->searchMismatch(temp[j].c_str(),temp3,0);
	    trieKnownString->searchMismatch( ( "N"+temp[j].substr(0, temp[j].size() -1)     ).c_str(),temp4,1);
	    trieKnownString->searchMismatch( (     temp[j].substr(1, temp[j].size() -1)+"N" ).c_str(),temp4,1);

	    //adding a tag before the shifted ones
	    for(unsigned int k=0;k<temp4->size();k++)
		(*temp4)[k]="SHFT#"+(*temp4)[k];
	    temp3->insert( temp3->end(), temp4->begin(), temp4->end() );

	    if(temp3->size() == 0 && temp4->size() == 0){
		temp2.push_back( "?");
	    }else{
		temp2.push_back( vectorToString(*temp3,","));  
	    }


	    delete temp3;
	    delete temp4;

	    
	}
	
	fileError<< 
	    vectorToString( temp,"\t" )<<"\t"<<
	    (*unfound)[i].second<<"\t"<<
	    vectorToString( temp2,"\t" )<< endl;
    }
}

void check_thresholds( rgAssignment &rg ) {
    rg.unknown  = -10 * rg.logLikelihoodScore > rgScoreCutoff ;
    rg.conflict = -10 * rg.logRatioTopToSecond < fracConflict && rg.logRatioTopToSecond < 0 ;
    rg.wrong    = -10 * rg.topWrongToTopCorrect > wrongness ;
}

void processSingleEndReads( BamAlignment &al, BamWriter &writer, bool printError, map<string,int> &unknownSeq, map<string,int> &wrongSeq, map<string,int> &conflictSeq)
{
    string index1;
    string index1Q;
    string index2;
    string index2Q;

    getIndices(al,index1,index1Q,index2,index2Q);

    rgAssignment rgReturn=assignReadGroup(index1,index1Q,index2,index2Q,rgScoreCutoff,fracConflict,mismatchesTrie);
    check_thresholds( rgReturn ) ;

    updateRecord(al,rgReturn);
    writer.SaveAlignment(al);

    //record unresolved indices
    if(printError){
	string keyIndex;
	if(index2.empty()){
	    keyIndex=index1;
	}else{
	    keyIndex=index1+"#"+index2;
	}

	if( rgReturn.conflict ) conflictSeq[ keyIndex ] ++;
	if( rgReturn.unknown  ) unknownSeq [ keyIndex ] ++;
	if( rgReturn.wrong    ) wrongSeq   [ keyIndex ] ++;
    }

}

void processPairedEndReads( BamAlignment &al, BamAlignment &al2, BamWriter &writer, bool printError, map<string,int> &unknownSeq, map<string,int> &wrongSeq, map<string,int> &conflictSeq)
{
    string index1;
    string index1Q;
    string index2;
    string index2Q;

    string sindex1;
    string sindex1Q;
    string sindex2;
    string sindex2Q;

    //retrieve indices
    getIndices(al,index1,index1Q,index2,index2Q);
    //check to see if the other indices are the same just for fun
    getIndices(al2,sindex1,sindex1Q,sindex2,sindex2Q);

    if(index1 !=sindex1 ){cerr<<"Seq#1 has a different index 1 than seq #2, exiting "        <<al.Name<<" vs "<<al2.Name<< endl; exit(1);}
    if(index1Q!=sindex1Q){cerr<<"Seq#1 has a different index 1 quality than seq #2, exiting "<<al.Name<<" vs "<<al2.Name<< endl; exit(1);}
    if(index2 !=sindex2 ){cerr<<"Seq#1 has a different index 2 than seq #2, exiting "        <<al.Name<<" vs "<<al2.Name<< endl; exit(1);}
    if(index2Q!=sindex2Q){cerr<<"Seq#1 has a different index 2 quality than seq #2, exiting "<<al.Name<<" vs "<<al2.Name<< endl; exit(1);}

    rgAssignment rgReturn = assignReadGroup(index1,index1Q,index2,index2Q,rgScoreCutoff,fracConflict,mismatchesTrie);
    check_thresholds( rgReturn ) ;

    updateRecord(al, rgReturn);
    updateRecord(al2,rgReturn);
    writer.SaveAlignment(al);
    writer.SaveAlignment(al2);

    // record unresolved indices
    if(printError){
        string keyIndex;
        if(index2.empty()){
            keyIndex=index1;
        }else{
            keyIndex=index1+"#"+index2;
        }

        if( rgReturn.conflict ) conflictSeq[ keyIndex ] += 2;
        if( rgReturn.unknown  ) unknownSeq [ keyIndex ] += 2;
        if( rgReturn.wrong    ) wrongSeq   [ keyIndex ] += 2;
    }
}

int main (int argc, char *argv[]) {

    BamReader reader;
    BamWriter writer;

    string bamFile;
    string bamFileOUT="";

    string index="";
    string outfile;
    bool   printSummary=false;
    string filenameSummary;

    bool   printError=false;
    string filenameError;

    ofstream ratioValuesOS;
    ofstream rgqualOS;

    bool ratioValuesFlag = false;
    bool rgqualFlag      = false;
    bool shiftByOne      = false;


    bool produceUnCompressedBAM=false; 
    const string usage=string(string(argv[0])+
			      " [options] BAMfile"+"\n\n"+

			      "\tCutoffs options:"+"\n"
			      "\t\t"+"--rgqual"  +"\t[quality]"+"\t\t"+""+"Worst quality before flagging as unknown ["+stringify(rgScoreCutoff)+"]\n"+
			      "\t\t"+"--fracconf"+"\t[quality]"+"\t\t"+""+"Maximum quality difference considered a conflict ["+stringify(fracConflict)+"] \n"+
			      "\t\t"+"--wrongness"+"\t[quality]"+"\t\t"+""+"Mininum quality difference to flag as wrongly paired ["+stringify(wrongness)+"] \n"+
			      "\t\t"+"--mm"+"\t\t[mismatches]"+"\t\t"+""+"Maximum # of tolerated mismatches ["+stringify(mismatchesTrie)+"] \n"+

			      "\n\tRG assignment options:"+"\n"+
			      "\t\t"+"" +""+"--shift"+"\t"+"\t\t\t\t"+"Try shifting the index right by one at the cost of a mismatch"+"\n"+


			      "\n\tOutput options:"+"\n"+
			      
			      "\t"+"\tMandatory:"+"\n"+
			      "\t\t"+"-i"+","+"--index"+"\t[index]"+"\t\t\t"+"File describing index sequences used"+"\n"+
			      "\t\t"+"-o"+","+"--outfile"+"\t[outfile]"+"\t\t"+"Specify output file"+"\n"+
			      "\t"+"\tOptional:"+"\n"+
                              "\t\t"+"-u" +"\t\t\t\t\t"           +"Produce uncompressed bam (good for pipe)"+"\n"+ 
			      "\t\t"+"-s"+","+"--summary"+"\t[summary file]"+"\t\t"+"Summarize the RG tally in this file"+"\n"+
			      "\t\t"+"-e"+","+"--error"  +"\t[error file]"+"\t\t"+"Summarize the indices that were not assigned to a RG"+"\n"+
			      "\t\t"+""+""+"--rgval"  +"\t[file]"+"\t\t\t\t"+"Write the rg qualities as a binary file"+"\n"+
			      "\t\t"+""+""+"--ratio"   +"\t\t[file]"+"\t\t\t"+"Write the likelihood ratios as a binary file"+"\n"

			      );
			      

    if( (argc== 1) ||
    	(argc== 2 && string(argv[1]) == "-h") ||
    	(argc== 2 && string(argv[1]) == "-help") ||
    	(argc== 2 && string(argv[1]) == "--help") ){
    	cout<<"Usage:"<<endl;
    	cout<<""<<endl;
    	cout<<usage<<endl;
    	return 1;
    }

    for(int i=1;i<(argc-1);i++){


	if(strcmp(argv[i],"--shift") == 0 ){
	    shiftByOne      = true;
	    continue;
	}

	if(strcmp(argv[i],"--rgval") == 0 ){
	    string temp =string(argv[i+1]);
	    rgqualOS.open(temp.c_str(), ios::out | ios::binary);
	    rgqualFlag      = true;
	    if (!rgqualOS){
		cerr<<"Cannot print to file "<<temp<<endl;
		exit(1);
	    }
	    setFileForRGQual(&rgqualOS);
	    i++;
	    continue;
	}


	if(strcmp(argv[i],"-u") == 0  ){ 
	    produceUnCompressedBAM=true; 
	    continue; 
	} 
	

	if(strcmp(argv[i],"--ratio") == 0 ){
	    string temp =string(argv[i+1]);
	    ratioValuesOS.open(temp.c_str(), ios::out | ios::binary);
	    ratioValuesFlag = true;

	    if (!ratioValuesOS){
		cerr<<"Cannot print to file "<<temp<<endl;
		exit(1);
	    }
	    setFileForRatio(&ratioValuesOS);
	    i++;
	    continue;
	}

	if(strcmp(argv[i],"-e") == 0 || strcmp(argv[i],"--error") == 0 ){
	    printError=true;
	    filenameError =string(argv[i+1]);
	    i++;
	    continue;
	}



	if(strcmp(argv[i],"-s") == 0 || strcmp(argv[i],"--summary") == 0 ){
	    printSummary=true;
	    filenameSummary =string(argv[i+1]);
	    i++;
	    continue;
	}



	if(strcmp(argv[i],"--rgqual") == 0 ){
	    rgScoreCutoff =destringify<double>(argv[i+1]);
	    i++;
	    continue;
	}

	if(strcmp(argv[i],"--fracconf") == 0 ){
	    fracConflict =destringify<double>(argv[i+1]);
	    i++;
	    continue;
	}

	if(strcmp(argv[i],"--wrongness") == 0 ){
	    wrongness =destringify<double>(argv[i+1]);
	    i++;
	    continue;
	}

	if(strcmp(argv[i],"--mm") == 0 ){
	    mismatchesTrie =destringify<int>(argv[i+1]);
	    i++;
	    continue;
	}



	if(strcmp(argv[i],"-i") == 0 || strcmp(argv[i],"--index") == 0 ){
	    index =string(argv[i+1]);
	    i++;
	    continue;
	}

	if(strcmp(argv[i],"-o") == 0 || strcmp(argv[i],"--outfile") == 0 ){
	    outfile=string(argv[i+1]);
	    i++;
	    continue;
	}


	cerr<<"Unknown option "<<argv[i] <<" exiting"<<endl;
	return 1;             
    }

    if(outfile.size() == 0){
	cerr<<"The field -o is mandatory exiting"<<endl;
	return 1;             
    }

    if(index.size() == 0){
	cerr<<"The field -i is mandatory exiting"<<endl;
	return 1;             
    }

    if(printError){
	trieKnownString = new PrefixTree<string>();
	initializeKnownIndices(trieKnownString);
	// //debug
	// vector<string> * temp3=new vector<string>();
	// vector<string> * temp4=new vector<string>();
	// string t="AAGGTCT";
	// trieKnownString->searchMismatch(t.c_str(),temp3,0);
	// trieKnownString->searchMismatch( ( "N"+t.substr(0, t.size() -1)     ).c_str(),temp4,1);
	// trieKnownString->searchMismatch( (     t.substr(1, t.size() -1)+"N" ).c_str(),temp4,1);
	// cout<<"t3 "<<vectorToString(*temp3)<<endl;
	// cout<<"t4 "<<vectorToString(*temp4)<<endl;

	// return 1;
	// //end debug
    }

    bamFile=argv[argc-1];


    map<string,string> rgs =readIndexFile(index,mismatchesTrie,shiftByOne);

    map<string,int> unknownSeq;
    map<string,int> wrongSeq;
    map<string,int> conflictSeq;

    if ( !reader.Open(bamFile) ) {
    	cerr << "Could not open input BAM file  "<<bamFile << endl;
    	return 1;
    }

    SamHeader  myHeader=reader.GetHeader();
    SamProgram sp;
   
    string pID          = "assignRG";   
    string pName        = "assignRG";   
    string pCommandLine = "";
    for(int i=0;i<(argc);i++){
	pCommandLine += (string(argv[i])+" ");
    }

    putProgramInHeader(&myHeader,pID,pName,pCommandLine,returnGitHubVersion(string(argv[0]),".."));

    SamReadGroupDictionary  srgd;
    map<string,string>::const_iterator itRG;   
    for ( itRG=rgs.begin(); itRG != rgs.end(); itRG++ ){
	SamReadGroup srg ( itRG->first );	
	srg.Description  = itRG->second; //description read in index file
	srgd.Add( srg );       
	namesMap[ itRG->first ] =0;
    }

    myHeader.ReadGroups=srgd;
    if(produceUnCompressedBAM)  
	writer.SetCompressionMode(BamWriter::Uncompressed); 

    if( !writer.Open(outfile,myHeader,reader.GetReferenceData() ) ) {
    	cerr << "Could not open output BAM file  "<<outfile << endl;
    	return 1;	
    }


    BamAlignment al;
    BamAlignment al2;

    // damn, this logic is convoluted...
    while( reader.GetNextAlignment(al) ) {
	while(1) {
            if( !reader.GetNextAlignment(al2) ) {
                // EOF, process the one leftover record
                processSingleEndReads(al,writer,printError,unknownSeq,wrongSeq,conflictSeq);
                break; 
            }
            // If it's paired, both should have the same index, and we
            // save some work.  Since the reads are probably not
            // ordered, check the names first
            if( al.IsPaired() && al.Name == al2.Name ) {
                processPairedEndReads(al,al2,writer,printError,unknownSeq,wrongSeq,conflictSeq);
                break ;
            } else {
                // no match, treat one(!) separately
                processSingleEndReads(al ,writer,printError,unknownSeq,wrongSeq,conflictSeq);
                swap(al,al2) ;
            }
        }
    }

    reader.Close();
    writer.Close();

    //Print summary of RG assignment
    if(printSummary){
     	map<string,int>::iterator it;   
	unsigned int totalRG=0;	
	unsigned int totalAssignRG=0;	

	vector< pair<string,int> > toprintVec;
	for ( it=namesMap.begin() ; it != namesMap.end(); it++ ){
	    toprintVec.push_back(  make_pair( it->first , it->second ) );
	    totalRG+=it->second;
	}

	sort (toprintVec.begin(),   toprintVec.end(),   compareNameRG() ); 
	ofstream fileSummary;
	fileSummary.open(filenameSummary.c_str());

	if (fileSummary.is_open()){

	    for(unsigned int i=0;i<toprintVec.size();i++){
		fileSummary << toprintVec[i].first << "\t" << toprintVec[i].second << "\t"
                            << 100.0*double(toprintVec[i].second)/double(totalRG) << "%\n" ;
                totalAssignRG+=toprintVec[i].second;
	    }
	    fileSummary<<dashes<<endl;
	    fileSummary<<"ASSIGNED:\t"<< totalAssignRG<<"\t"<<100.0*double(totalAssignRG)/double(totalRG)<<"%"<<endl;
	    fileSummary<<"PROBLEMS:\t"<< (totalRG-totalAssignRG)<<"\t"<<100.0*double(totalRG-totalAssignRG)/double(totalRG)<<"%"<<endl;

	    fileSummary<<"TOTAL:\t"<<totalRG<<"\t100.0%"<<endl;
	}else{
	    cerr << "Unable to print to file "<<filenameSummary<<endl;
	}
	fileSummary.close();
    }

    //Print over-represented sequences in conflict,unknown,wrong
    if(printError){
	vector< pair<string,int> > conflictToPrint( conflictSeq.begin(), conflictSeq.end() ) ;
	vector< pair<string,int> > unknownToPrint( unknownSeq.begin(), unknownSeq.end() ) ;
	vector< pair<string,int> > wrongToPrint( wrongSeq.begin(), wrongSeq.end() ) ;
     	
	sort (conflictToPrint.begin(),   conflictToPrint.end(),   compareNameRG() ); 
	sort (unknownToPrint.begin(),    unknownToPrint.end(),    compareNameRG() ); 
	sort (wrongToPrint.begin(),      wrongToPrint.end(),      compareNameRG() ); 

	ofstream fileError;
	fileError.open(filenameError.c_str());
	if (fileError.is_open())
        {
	    fileError<<      dashes<<endl<<"Conflict:"<<endl<<dashes<<endl;
	    printUnfoundToFile(&conflictToPrint,fileError);

	    fileError<<endl<<dashes<<endl<<"Unknown:" <<endl<<dashes<<endl;
	    printUnfoundToFile(&unknownToPrint,fileError);

	    fileError<<endl<<dashes<<endl<<"Wrong:"   <<endl<<dashes<<endl;
	    printUnfoundToFile(&wrongToPrint,fileError);
	}else{
	    cerr << "Unable to print to file "<<filenameError<<endl;
	}
	fileError.close();
    }

    //cleaning up
    deallocate();
    if(printError){
	delete trieKnownString;
    }

    if(rgqualFlag)
	rgqualOS.close();

    if(ratioValuesFlag)
	ratioValuesOS.close();

    return 0;
}
