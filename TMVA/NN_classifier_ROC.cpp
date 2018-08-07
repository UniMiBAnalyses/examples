// c++  -g -o NN_classifier_ROC NN_classifier_ROC.cpp `root-config --libs --glibs --cflags` -lTMVA -lTMVAGui
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
#include "TGraph.h"

using namespace std ;


TH1F * primitive (const TH1F * input)
{
  TH1F * histo = (TH1F *) input->Clone (input->GetName () + TString ("_int")) ;
  histo->SetTitle (input->GetTitle () + TString (" primitive")) ;
  for (int i = 1 ; i <= input->GetNbinsX () ; ++i)
    {
      histo->SetBinContent (i, histo->GetBinContent (i) + histo->GetBinContent (i-1)) ;
    }
  return histo ;  
}


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- 


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

  //PG get primitives and Draw the ROC curve
  //PG which has on the x axis the signal efficiency
  //PG and on the y axis the background rejection, that is 
  //PG one minus the signal efficiency, 
  //PG for a selection which asks the NN output to be greater than a given threshold
  TH1F * h_NN_signal_int = primitive (&h_NN_signal) ;
  TH1F * h_NN_background_int = primitive (&h_NN_background) ;

  Float_t * xaxis = new Float_t[h_NN_background_int->GetNbinsX ()] ;
  Float_t * yaxis = new Float_t[h_NN_background_int->GetNbinsX ()] ;
  for (int i = 0 ; i <= h_NN_background_int->GetNbinsX () ; ++i)
    {
      xaxis[i] = 1 - h_NN_signal_int->GetBinContent (i) / h_NN_signal_int->GetBinContent (h_NN_background_int->GetNbinsX ()) ;
      yaxis[i] = h_NN_background_int->GetBinContent (i) / h_NN_background_int->GetBinContent (h_NN_background_int->GetNbinsX ()) ;
    }
  TGraph ROC (h_NN_background_int->GetNbinsX (), xaxis, yaxis) ;

  //PG save histograms to a root file
  TFile outfile ("NN_distributions2.root", "recreate") ;
  outfile.cd () ;
  h_NN_signal.Write () ;
  h_NN_background.Write () ;
  h_NN_signal_int->Write () ;
  h_NN_background_int->Write () ;
  ROC.Write ("ROC") ;
  outfile.Close () ;
  
  //PG draw histograms
  h_NN_signal.SetLineWidth (2) ;
  h_NN_background.SetLineWidth (2) ;
  h_NN_background.SetLineStyle (2) ;
  h_NN_signal.SetStats (0) ;
  h_NN_background.SetStats (0) ;
  
  TCanvas c1 ;
  ROC.Draw ("AL") ;
  ROC.Draw ("Csame") ;
  c1.Print ("NN_ROC.pdf", "pdf") ;

  delete reader ;
  delete h_NN_signal_int ;
  delete h_NN_background_int ;

  return 0 ;
}

