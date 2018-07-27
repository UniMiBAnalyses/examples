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
  //PG create a  example and fill it with a 2D gaussian distribution centered around (0,0)
  //PG with x and y sigma equal to 2
  TNtuple * dataset = new TNtuple ("signal", "signal", "x:y") ;
  for (int i = 0 ; i < 10000 ; ++i)
    {
      dataset->Fill (gRandom->Gaus (0,2), gRandom->Gaus (0,2)) ;
    }  

  //PG read the already-trained NN from disk
  TMVA::Reader * reader = new TMVA::Reader( "!Color:!Silent" );
  Float_t x, y ;
  reader->AddVariable ("x", &x) ;
  reader->AddVariable ("y", &y) ;
  reader->BookMVA("NN", "NN_classifier/weights/TMVAClassification_MLP.weights.xml") ;

  //PG draw the signal distribution of the discriminant
  dataset->SetBranchAddress ("x", &x) ;
  dataset->SetBranchAddress ("y", &y) ;
  
  TH2F h_NN_signal_like ("h_NN_signal_like", "events distribution", 100, -4., 4., 100, -4., 4.) ;
  TH2F h_NN_background_like ("h_NN_background_like", "events distribution", 100, -4., 4., 100, -4., 4.) ;

  //PG loop over the ntuple events    
  for (int i = 0 ; i < dataset->GetEntries (); ++i)
    {
      dataset->GetEntry (i) ;
      if (reader->EvaluateMVA ("NN") > 0.5) 
        {
          h_NN_signal_like.Fill (x, y) ;
        }
      else 
        {
          h_NN_background_like.Fill (x, y) ;
        }
    }

  //PG draw histograms
  h_NN_signal_like.SetMarkerColor (kRed+2) ;
  h_NN_background_like.SetMarkerColor (kBlue+2) ;
  
  TCanvas c1 ;
  h_NN_signal_like.Draw () ;
  h_NN_background_like.Draw ("same") ;
  c1.Print ("NN_separation.pdf", "pdf") ;

  delete reader ;

  return 0 ;
}

