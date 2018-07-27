// c++  -g -o NN_classifier_use NN_classifier_use.cpp `root-config --libs --glibs --cflags` -lTMVA -lTMVAGui
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
  //PG create a signal example and fill it with a 2D gaussian distribution centered around (1,1)
  //PG with x and y sigma equal to 1
  TNtuple * signal = new TNtuple ("signal", "signal", "x:y") ;
  for (int i = 0 ; i < 10000 ; ++i)
    {
      signal->Fill (gRandom->Gaus (1,1), gRandom->Gaus (1,1)) ;
    }  

  //PG create a background example and fill it with a 2D gaussian distribution centered around (-1,-1)
  //PG with x and y sigma equal to 1
  TNtuple * background = new TNtuple ("background", "background", "x:y") ;
  for (int i = 0 ; i < 10000 ; ++i)
    {
      background->Fill (gRandom->Gaus (-1,1), gRandom->Gaus (-1,1)) ;
    }  

  //PG read the already-trained NN from disk
  TMVA::Reader * reader = new TMVA::Reader( "!Color:!Silent" );
  Float_t x, y ;
  reader->AddVariable ("x", &x) ;
  reader->AddVariable ("y", &y) ;
  reader->BookMVA("NN", "NN_classifier/weights/TMVAClassification_MLP.weights.xml") ;

  //PG draw the signal distribution of the discriminant
  signal->SetBranchAddress ("x", &x) ;
  signal->SetBranchAddress ("y", &y) ;
  
  TH1F h_NN_signal ("h_NN_signal", "signal NN distribution", 100, 0., 1.) ;

  //PG loop over the ntuple events    
  for (int i = 0 ; i < signal->GetEntries (); ++i)
    {
      signal->GetEntry (i) ;
      h_NN_signal.Fill (reader->EvaluateMVA ("NN")) ;    
    }

  //PG draw the background distribution of the discriminant
  background->SetBranchAddress ("x", &x) ;
  background->SetBranchAddress ("y", &y) ;
  
  TH1F h_NN_background ("h_NN_background", "background NN distribution", 100, 0., 1.) ;

  //PG loop over the ntuple events    
  for (int i = 0 ; i < background->GetEntries (); ++i)
    {
      background->GetEntry (i) ;
      h_NN_background.Fill (reader->EvaluateMVA ("NN")) ;    
    }

  //PG save histograms to a root file
  TFile outfile ("NN_distributions.root", "recreate") ;
  outfile.cd () ;
  h_NN_signal.Write () ;
  h_NN_background.Write () ;
  
  //PG draw histograms
  h_NN_signal.SetLineWidth (2) ;
  h_NN_background.SetLineWidth (2) ;
  h_NN_background.SetLineStyle (2) ;
  h_NN_signal.SetStats (0) ;
  h_NN_background.SetStats (0) ;
  
  TCanvas c1 ;
  c1.SetLogy () ;
  if (h_NN_signal.GetMaximum () > h_NN_background.GetMaximum ()) h_NN_signal.Draw () ;
  else h_NN_background.Draw () ;
  h_NN_background.Draw ("same") ;
  h_NN_signal.Draw ("same") ;
  c1.Print ("NN_distributions.pdf", "pdf") ;

  

  delete reader ;

  return 0 ;
}

