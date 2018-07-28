// c++  -g -o NN_regression_train NN_regression_train.cpp `root-config --libs --glibs --cflags` -lTMVA -lTMVAGui
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
// https://root.cern.ch/doc/v608/TMVARegression_8C.html

int main (int argc, char ** argv)
{
  // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
  TFile * outputFile = TFile::Open ("NN_regression.root", "RECREATE") ;

  TMVA::Tools::Instance () ;
  TMVA::Factory * factory = new TMVA::Factory
    ( 
      "TMVARegression", 
      outputFile,
      "!V:!Silent:Color:DrawProgressBar:AnalysisType=Regression" 
    ) ;

  // the name "NN_classifier" is the one of the folder where the output will be saved
  TMVA::DataLoader * dataloader = new TMVA::DataLoader ("NN_regression") ;
  
  //PG the following are the input variables to the regression
  dataloader->AddVariable ("var1", "Variable 1", "units", 'F') ;
  dataloader->AddVariable ("var2", "Variable 2", "units", 'F') ;
  
  //PG the following is the target variable of the regression
  dataloader->AddTarget ("fvalue") ;
   
  //PG read the input ROOT TTree example, to be used for the regression training
  TFile input ("tmva_reg_example.root") ;
  // get the regression tree from the TFile
  TTree * regTree = (TTree*) input.Get ("TreeR") ;

  // global event weights per tree
  Double_t regWeight  = 1.0 ;
  // You can add an arbitrary number of regression trees
  dataloader->AddRegressionTree (regTree, regWeight) ;
 
  TCut mycut = "" ; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";

  // tell the DataLoader to use all remaining events in the trees after training for testing:
  dataloader->PrepareTrainingAndTestTree
    ( 
      mycut,
      "nTrain_Regression=5000:nTest_Regression=5000:SplitMode=Random:NormMode=NumEvents:!V" 
    ) ;

  // If no numbers of events are given, half of the events in the tree are used
  // for training, and the other half for testing:
  //
  //     dataloader->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );

  factory->BookMethod
    ( 
      dataloader, 
      TMVA::Types::kMLP, 
      "MLP", 
      "!H:!V:VarTransform=Norm:NeuronType=tanh:NCycles=20000:HiddenLayers=N+20:TestRate=6:TrainingMethod=BFGS:Sampling=0.3:SamplingEpoch=0.8:ConvergenceImprove=1e-6:ConvergenceTests=15:!UseRegulator"
    ) ;

  // Train MVAs using the set of training events
  factory->TrainAllMethods () ;

  // Evaluate all MVAs using the set of test events
  factory->TestAllMethods () ;

  // Evaluate and compare performance of all configured MVAs
  factory->EvaluateAllMethods () ;

  // Save the output
  outputFile->Close () ;

  delete factory ;
  delete dataloader ;
  delete outputFile ;

  return 0 ;
}

