// c++  -g -o NN NN.cpp `root-config --libs --glibs --cflags` -lTMVA -lTMVAGui
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

#include "TNtuple.h"
#include "TH2F.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"
#include "TCut.h"


// a more complete example from the ROOT website can be found here:
// https://root.cern.ch/doc/v608/TMVAClassification_8C.html

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

  //PG draw the two distributions
  TCanvas c1 ;
  signal->SetMarkerColor (kRed) ;
  background->SetMarkerColor (kBlue) ;
  signal->Draw ("y:x", "", "P") ;
  background->Draw ("y:x", "", "P same") ;
  c1.Print ("distributions.pdf", "pdf") ;


  // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
  TString outfileName ("TMVA.root") ;
  TFile * outputFile = TFile::Open (outfileName, "RECREATE") ;

  TMVA::Tools::Instance () ;
  TMVA::Factory * factory = new TMVA::Factory 
    (
      "TMVAClassification", 
      outputFile,                                           
      "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" 
    ) ;

  TMVA::DataLoader * dataloader = new TMVA::DataLoader ("dataset") ;

  dataloader->AddVariable ("x", 'F') ;
  dataloader->AddVariable ("y", 'F') ;

  dataloader->AddSignalTree (signal) ;
  dataloader->AddBackgroundTree (background) ;

  TCut mycuts = "" ; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
  TCut mycutb = "" ; // for example: TCut mycutb = "abs(var1)<0.5";
  
  dataloader->PrepareTrainingAndTestTree
    ( 
      mycuts, 
      mycutb,
      "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );

  //PG this is the TMVA ANN based on a multi-layer perceptron -- all ANNs in TMVA are Multilayer Perceptrons
  //PG more algorithms are present in the TMVA example 
  factory->BookMethod 
    (
      dataloader, 
      TMVA::Types::kMLP, 
      "MLP", 
      "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator"
    ) ;

  // Train MVAs using the set of training events
  factory->TrainAllMethods () ;

  // Evaluate all MVAs using the set of test events
  factory->TestAllMethods () ;

  // Evaluate and compare performance of all configured MVAs
  factory->EvaluateAllMethods () ;

  return 0 ;
}

