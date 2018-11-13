//This script executes the showering and jet clustering of LHE events  
// using Pythia and Fastjet. 
// The script doesn't save a HepMC file but a TTree with the necessary 
// quadri-momenta.

#include <iostream>
#include <fstream>

#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC2.h"
// FastJet3 integration with Pythia
#include "Pythia8Plugins/FastJet3.h"

// ROOT include
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TLorentzVector.h"
#include <algorithm>
#include <vector>

using namespace Pythia8;
using namespace std;
using namespace HepMC;

int pdgids_partons [] = {1,2,3,4,5,9,21};
int unstable [] = {23, 24, 25};

bool is_parton(int pdg_id)
{
  return (find(begin(pdgids_partons), end(pdgids_partons), pdg_id) != end(pdgids_partons));
}

bool is_unstable(int pdg_id)
{
  return (find(begin(unstable), end(unstable), pdg_id) != end(unstable));
}


int main(int argc, char* argv[]) {

  bool ttbar_check = false;
  int nttbar = 0;

  // Check that correct number of command-line arguments
  if (argc < 5) {
    cerr << " Unexpected number of command-line arguments ("<<argc<<"). \n"
         << " You are expected to provide the arguments" << endl
         << " 1. Input file for settings" << endl
         << " 2. Name of the input LHE file" << endl
         << " 3. Output name" << endl
         << " 4. Number of events" << endl 
         << " Program stopped. " << endl;
    return 1;
  }

  // Check that the provided input name corresponds to an existing file.
  ifstream is(argv[1]);
  if (!is) {
    cerr << " Command-line file " << argv[1] << " was not found. \n"
         << " Program stopped! " << endl;
    return 1;
  }

  // Confirm that external files will be used for input and output.
  cout << "\n >>> PYTHIA settings will be read from file " << argv[1]
       << " <<< \n >>> LHE files will be read from file "
       << argv[2] << " <<< \n" 
	<< " <<< \n >>> Output will be written to file "
       << argv[3] << " <<< \n" << endl;


  // TTree to save jets
  TFile * jetfile = new TFile(argv[3], "RECREATE");
  TTree * jetstree = new TTree("tree", "tree");
  int njets = 0;  
  double E_jets[25];
  double px_jets[25];
  double py_jets[25];
  double pz_jets[25];
  jetstree->Branch("njets", &njets, "njets/I");
  jetstree->Branch("E_jets", &E_jets, "E_jets[njets]/D");
  jetstree->Branch("px_jets", &px_jets, "px_jets[njets]/D");
  jetstree->Branch("py_jets", &py_jets, "py_jets[njets]/D");
  jetstree->Branch("pz_jets", &pz_jets, "pz_jets[njets]/D");
  int lep_flavour = 0; // 0 = muon, 1= electron
  double p_lep[4];
  double p_nu[4];
  double p_lep_lhe[4];
  double met[4];
  double unclustered_met[4]; 
  double E_parton[4];
  double px_parton[4];
  double py_parton[4];
  double pz_parton[4];
  int partons_flavour[4];
  int npartons = 0; 
  int ttbar = 0;
  jetstree->Branch("npartons", &npartons, "npartons/I");
  jetstree->Branch("E_parton", &E_parton, "E_parton[npartons]/D");
  jetstree->Branch("px_parton", &px_parton, "px_parton[npartons]/D");
  jetstree->Branch("py_parton", &py_parton, "py_parton[npartons]/D");
  jetstree->Branch("pz_parton", &pz_parton, "pz_parton[npartons]/D");
  jetstree->Branch("partons_flavour", &partons_flavour, "partons_flavour[4]/I");
  jetstree->Branch("lep_flavour", &lep_flavour, "lep_flavour/I");
  jetstree->Branch("p_lep", &p_lep, "p_lep[4]/D");
  jetstree->Branch("p_lep_lhe", &p_lep_lhe, "p_lep_lhe[4]/D"); 
  jetstree->Branch("p_nu", &p_nu, "p_nu[4]/D"); 
  jetstree->Branch("met", &met, "met[4]/D");
  jetstree->Branch("unclustered_met", &unclustered_met, "unclustered_met[4]/D");
  if (ttbar_check){
    jetstree->Branch("ttbar", &ttbar, "ttbar/I");
  }
  
  // Generator.
  Pythia pythia;
  // Event placeholder
  Event& event = pythia.event;
  // This event is equivalent to LHE data
  Event& hard_process = pythia.process;
  // Interface for conversion from Pythia8::Event to HepMC event.
  HepMC::Pythia8ToHepMC ToHepMC;

  // Read in commands from external file.
  pythia.readFile(argv[1]); 

  // Extract settings to be used in the main program.
  int nEvents = stoi(argv[4]);
  int nAbort = nEvents +1; 
  pythia.settings.word("Main:numberOfEvents", to_string(nEvents) );
  pythia.settings.word("Main:timesAllowErrors", to_string(nAbort));
  pythia.settings.word("Main:spareMode1", to_string(nEvents));
  // Setup the lhe file
  pythia.settings.word("Beams:LHEF", string(argv[2]));

  // Initialization.
  pythia.init();

  // Select common parameters FastJet analyses.
  int    power     = -1;     // -1 = anti-kT; 0 = C/A; 1 = kT.
  double R         = 0.4;    // Jet size.
  double pTMin_jet = 20.0;    // Min jet pT.
  double etaMax    = 6.0;    // Pseudorapidity range of detector.

  // Set up FastJet jet finder.
  //   one can use either explicitly use antikt, cambridge, etc., or
  //   just use genkt_algorithm with specification of power
  //fastjet::JetAlgorithm algorithm;
  //if (power == -1)      algorithm = fastjet::antikt_algorithm;
  //if (power ==  0)      algorithm = fastjet::cambridge_algorithm;
  //if (power ==  1)      algorithm = fastjet::kt_algorithm;
  fastjet::JetDefinition jetDef(fastjet::genkt_algorithm, R, power);
  std::vector <fastjet::PseudoJet> fastjet_input;


  // Begin event loop.
  int iAbort = 0;
  for (int iEvent = 0; iEvent < nEvents; ++iEvent) {

    // Clear arrays for tree
    std::fill_n(E_jets, 25, 0.);
    std::fill_n(px_jets, 25, 0.);
    std::fill_n(py_jets, 25, 0.);
    std::fill_n(pz_jets, 25, 0.);
    std::fill_n(E_parton, 4, 0.);
    std::fill_n(px_parton, 4, 0.);
    std::fill_n(py_parton, 4, 0.);
    std::fill_n(pz_parton, 4, 0.);
    std::fill_n(p_lep, 4, 0.);
    std::fill_n(p_lep_lhe, 4, 0.);
    std::fill_n(p_nu, 4, 0.);
    std::fill_n(met, 4, 0.);
    std::fill_n(unclustered_met, 4, 0.);


    // Generate event.
    if (!pythia.next()) {
      // If failure because reached end of file then exit event loop.
      if (pythia.info.atEndOfFile()) {
        cout << " Aborted since reached end of Les Houches Event File\n";
        break;
      }
      // First few failures write off as "acceptable" errors, then quit.
      if (++iAbort < nAbort) continue;
      cout << " Event generation aborted prematurely, owing to error!\n";
      break;
    }

    //##########################################
    //#######   Primary vertex analysis
    //#######   Extract partons and muon info from hepmc information
    //##########################################
 
    // HEPMC analysis to find initial partons, muon and neutrino
    GenEvent* hepmcevt = new GenEvent();
    ToHepMC.fill_next_event( pythia, hepmcevt );
    
    bool found_neutrino = false;
    bool found_lepton = false;
    Vec4 lepton_lhe;
    Vec4 nu_lhe;
    bool top_found = false;
    bool antitop_found = false; 

    // Extract partons from status=23
    // Check if there is a muon/electron in hard process (status = 23)
    npartons = 0;
    for(auto part_it=hepmcevt->particles_begin(); part_it!=hepmcevt->particles_end(); part_it++){
      GenParticle* p = *part_it;
      if ( p->status() == 23){
        int pdg = abs(p->pdg_id());
        // Save if parton with status 23
        if (is_parton(pdg)){
          E_parton[npartons] = p->momentum().e();
          px_parton[npartons] = p->momentum().px();
          py_parton[npartons] = p->momentum().py();
          pz_parton[npartons] = p->momentum().pz();
          partons_flavour[npartons] = p->pdg_id();
          npartons++;
        }
        // Check if there is a muon/electron or neutrino
        if( pdg == 13 || pdg == 11 ){
          lepton_lhe =Vec4(p->momentum().px(), p->momentum().py(), 
                        p->momentum().pz(), p->momentum().e());
          // save flavour
          (pdg == 11) ? lep_flavour = 1 : lep_flavour = 0; 
          found_lepton = true;

        }else if ( pdg ==14){
          nu_lhe = Vec4(p->momentum().px(), p->momentum().py(), 
                        p->momentum().pz(), p->momentum().e());
          found_neutrino = true;
        }
      }
      //Check if ttbar process
      // if (ttbar_check){
      //   if ( p->pdg_id() == 6){
      //     top_found = true;
      //   }else if (p->pdg_id() == -6){
      //     antitop_found = true; 
      //   }
        
      // }
    }
    // if (top_found || antitop_found ){
    //   hard_process.list();
    //   cout << "OK" << endl;
    // }else{
    //   //cout << "NOTOP" << endl;
    // }
    // // Save ttbar event type flag if two tops are found in intermediate state
    // if (ttbar_check == true && top_found && antitop_found){
    //   ttbar = 1; 
    //   nttbar+=1; 
    //   cout << ">>>>>>>>>>>TTBAR" << endl;
    // }

    // If not found, look for status 1 (final status) but with lower barcode
    if (!found_lepton){
        for(auto part_it=hepmcevt->particles_begin(); part_it!=hepmcevt->particles_end(); part_it++){
          GenParticle* p = *part_it;
          if ( p->status() == 1 && ( abs(p->pdg_id()) == 13 || abs(p->pdg_id()) == 11)){
            lepton_lhe = Vec4(p->momentum().px(), p->momentum().py(), 
                        p->momentum().pz(), p->momentum().e());
            // save flavour
            ( abs(p->pdg_id()) == 11) ? lep_flavour = 1 : lep_flavour = 0;
            break;
          } 
        }
    }
    if (!found_neutrino){
        for(auto part_it=hepmcevt->particles_begin(); part_it!=hepmcevt->particles_end(); part_it++){
          GenParticle* p = *part_it;
          if ( p->status() == 1 && abs(p->pdg_id()) == 14){
            nu_lhe = Vec4(p->momentum().px(), p->momentum().py(), 
                        p->momentum().pz(), p->momentum().e());
            break;
          } 
        }
    }

    // Save LHE info for muon and neutrino
    p_lep_lhe[0] = lepton_lhe.px();
    p_lep_lhe[1] = lepton_lhe.py();
    p_lep_lhe[2] = lepton_lhe.pz();
    p_lep_lhe[3] = lepton_lhe.e();
    p_nu[0] = nu_lhe.px();
    p_nu[1] = nu_lhe.py();
    p_nu[2] = nu_lhe.pz();
    p_nu[3] = nu_lhe.e();

    // Destroy hepMc object
    delete hepmcevt;
    
    //##########################################
    //#######  Final particle extraction
    //##########################################
    // List of muons in final state
    vector<Particle> final_leptons;

    fastjet_input.resize(0);
    for (int i = 0; i < event.size(); ++i) {
      Particle& particle = event[i];

      // Final state only
      if (!particle.isFinal()) continue;

      // No neutrinos in shower 
      if (particle.idAbs() == 12 || particle.idAbs() == 14 
          || particle.idAbs() == 16) continue;

      // Save final state muons/electrons
      if (particle.idAbs() == 13 || particle.idAbs() == 11) {
        final_leptons.push_back(particle);
        // Insert the particles in fastjet later, after the exclusion of the signal jet
        continue;
      }
      
      // Create a PseudoJet from the complete Pythia particle.
      fastjet::PseudoJet particleFastJet = particle;
      // Store acceptable particles as input to Fastjet.
      fastjet_input.push_back(particleFastJet);
    }

    //########################################################
    //#### Lepton analysis   
    //#### Find the lepton coming from primary vertex looking 
    //#### at DeltaR with lhe lepton
    //########################################################
    
    double min_deltaR_lepton = -1.;
    Vec4 final_lepton;
    int final_lepton_i = -1;
    // Match the lhe muon with final muons after showering
    for (int i = 0; i< final_leptons.size(); i++){
      double deltaR = REtaPhi(lepton_lhe, final_leptons[i].p());
      if(min_deltaR_lepton < 0 || min_deltaR_lepton > deltaR){
        final_lepton = final_leptons[i].p(); 
        final_lepton_i = i;
        min_deltaR_lepton = deltaR;
      }
    }

    //Save final muon data
    p_lep[0] = final_lepton.px();
    p_lep[1] = final_lepton.py();
    p_lep[2] = final_lepton.pz();
    p_lep[3] = final_lepton.e();

    // Insert back all leptons in the fastjet
    for (int i = 0; i< final_leptons.size(); i++){
      if (i != final_lepton_i){
        fastjet_input.push_back(final_leptons[i]);
      }
    }

    // if (min_deltaR_lepton > 0.4) {
    //   cout << "Problem!! lepton dR"<< min_deltaR_lepton << endl;
    // }  
  
    //##########################################
    //#######  Fastjet extraction
    //##########################################

    // Run Fastjet algorithm and sort jets in pT order.
    vector <fastjet::PseudoJet> inclusiveJets, sortedJets;
    fastjet::ClusterSequence clustSeq(fastjet_input, jetDef);
    // Cluster jet with 0 pt_min
    inclusiveJets = clustSeq.inclusive_jets();
    sortedJets    = sorted_by_pt(inclusiveJets);

    //#########################################
    //########    Jet analysis   ##############
    //#########################################
    
    fastjet::PseudoJet _met;
    fastjet::PseudoJet _unclmet;
    njets = 0;
    // Save the jets CUTTING for pt
    for(int i = 0; i< int(sortedJets.size()); ++i ){
      fastjet::PseudoJet j = sortedJets[i];

      // Cut jet on Pt 
      if (j.pt() < pTMin_jet) {
        // Save unclustered met info for cutted jets
        _unclmet -= j;
        continue;
      }
      //Save met
      _met -= j ;
      // Save jets 4momentum
      E_jets[njets] = j.e();
      px_jets[njets] = j.px();
      py_jets[njets] = j.py();
      pz_jets[njets] = j.pz();
      njets++;
    }  
    // Remove also the muon from met
    // Saving the MET
    met[0] = _met.px() - p_lep[0];
    met[1] = _met.py() - p_lep[1];
    met[2] = _met.pz() - p_lep[2];
    met[3] = _met.e()  - p_lep[3];
    unclustered_met[0] = _unclmet.px();
    unclustered_met[1] = _unclmet.py();
    unclustered_met[2] = _unclmet.pz();
    unclustered_met[3] = _unclmet.e();

    //######################################

    // Fill the tree
    jetstree->Fill();       
    
  } // End of events loop
   
  jetfile->Write();
  jetfile->Close();
  // Done.
  return 0;
}


