// c++  -g -o NN_regression_read NN_regression_read.cpp `root-config --libs --glibs --cflags` -lTMVA -lTMVAGui
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

#include "TNtuple.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/TMVAGui.h"
#include "TCut.h"

using namespace std ;

// a more complete example from the ROOT website can be found here:
// https://root.cern.ch/doc/v608/TMVAMultipleBackgroundExample_8C.html

int main (int argc, char ** argv)
{

  //PG read the already-trained NN from disk
  TMVA::Reader * reader = new TMVA::Reader( "!Color:!Silent" );
  // Create a set of variables and declare them to the reader
  // - the variable names MUST corresponds in name and type to those given in the weight file(s) used
  Float_t var1, var2, fvalue ;
  reader->AddVariable( "var1", &var1 );
  reader->AddVariable( "var2", &var2 );

  reader->BookMVA("NN", "NN_regression/weights/TMVARegression_MLP.weights.xml") ;

  //PG read the input ROOT TTree example, to be used for the regression training
  TFile input ("tmva_reg_example.root") ;
  // get the regression tree from the TFile
  TTree * regTree = (TTree*) input.Get ("TreeR") ;

  //PG draw the signal distribution of the discriminant
  regTree->SetBranchAddress ("var1", &var1) ;
  regTree->SetBranchAddress ("var2", &var2) ;
  regTree->SetBranchAddress ("fvalue", &fvalue) ;
  
  TH1F h_compare ("h_compare", "relative difference", 100, -0.06, 0.06) ;

  //PG loop over the ntuple events    
  for (int i = 0 ; i < regTree->GetEntries (); ++i)
    {
      regTree->GetEntry (i) ;
      h_compare.Fill ((reader->EvaluateRegression ("NN")[0] - fvalue) / fvalue) ;    
    }

  //PG save histograms to a root file
  TFile outfile ("NN_regression_result.root", "recreate") ;
  outfile.cd () ;
  h_compare.Write () ;
  outfile.Close () ;

  //PG draw histogram
  h_compare.SetLineWidth (2) ;
  
  TCanvas c1 ;
  h_compare.Draw ("") ;
  c1.Print ("NN_regression_result.pdf", "pdf") ;

  delete reader ;

  return 0 ;
}

