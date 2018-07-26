// c++  -g -o unit01 unit01.cpp `root-config --libs --glibs --cflags`
#include "LHEF.h"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>

#include "TLorentzVector.h"
#include "TH1F.h"
#include "TF1.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TNtuple.h"


class LHEreader
{
  public:
    LHEreader (std::string inputFileFile, bool makeClasses = true) ;
    bool readEvent () ;
    ~LHEreader () {} ;

    std::vector<std::pair<int, TLorentzVector> > m_particles ;
    std::vector<std::pair<int, TLorentzVector&> > m_el ; // electrons
    std::vector<std::pair<int, TLorentzVector&> > m_mu ; // muons
    std::vector<std::pair<int, TLorentzVector&> > m_ta ; // taus
    std::vector<std::pair<int, TLorentzVector&> > m_le ; // leptons
    std::vector<std::pair<int, TLorentzVector&> > m_ve ; // electron neutrinos
    std::vector<std::pair<int, TLorentzVector&> > m_vm ; // muon neutrinos
    std::vector<std::pair<int, TLorentzVector&> > m_vT ; // tau neutrinos
    std::vector<std::pair<int, TLorentzVector&> > m_vs ; // all neutrinos
    std::vector<std::pair<int, TLorentzVector&> > m_qu ; // quarks
    std::vector<std::pair<int, TLorentzVector&> > m_gl ; // gluons
    std::vector<std::pair<int, TLorentzVector&> > m_je ; // partons (q+g)

  private:
    std::ifstream m_ifs ;  
    LHEF::Reader  m_reader ;
    bool          m_makeClasses ;

  public:
    int           m_evtNum ;


} ;

LHEreader::LHEreader (std::string inputFileName, bool makeClasses):
  m_ifs (inputFileName.c_str ()),
  m_reader (m_ifs),
  m_evtNum (0),
  m_makeClasses (makeClasses)
{
  std::cout << "[LHEreader] reading file " << inputFileName << "\n" ;
  std::cout << "[LHEreader] preparing sub-vectors " << makeClasses << "\n" ;
}

/** 
read the event with the LHEF class and produces collections of paris (pdgid, TLorentzVector)
*/
bool 
LHEreader::readEvent ()
{
  bool reading = m_reader.readEvent () ; 
  if (!reading) return reading ;
  m_particles.clear () ;
  if (m_makeClasses) 
    {
      m_el.clear () ;
      m_mu.clear () ;
      m_ta.clear () ;
      m_le.clear () ;
      m_ve.clear () ;
      m_vm.clear () ;
      m_vT.clear () ;
      m_vs.clear () ;
      m_qu.clear () ;
      m_gl.clear () ;
      m_je.clear () ;
    }
  if (m_evtNum % 10000 == 0) std::cerr << "event " << m_evtNum << "\n" ;

  //PG loop over particles in the event
  for (int iPart = 0 ; iPart < m_reader.hepeup.IDUP.size (); ++iPart)
    {
      // std::cerr << "\t part type [" << iPart << "] " << m_reader.hepeup.IDUP.at (iPart)
      //           << "\t status " << m_reader.hepeup.ISTUP.at (iPart)
      //           << "\n" ;

      //PG outgoing particles only
      if (m_reader.hepeup.ISTUP.at (iPart) != 1) continue ; 

      int partType = m_reader.hepeup.IDUP.at (iPart) ;
      TLorentzVector particle 
        (
          m_reader.hepeup.PUP.at (iPart).at (0), //PG px
          m_reader.hepeup.PUP.at (iPart).at (1), //PG py
          m_reader.hepeup.PUP.at (iPart).at (2), //PG pz
          m_reader.hepeup.PUP.at (iPart).at (3) //PG E
        ) ;
      m_particles.push_back (std::pair<int, TLorentzVector> (partType, particle)) ;
      if (m_makeClasses)
        {
          if (abs (partType) < 6) 
            {
              m_qu.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
              m_je.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
            }
          else if (abs (partType) == 11 || abs (partType) == 13 || abs (partType) == 15)
            {
              m_le.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
              switch(abs (partType)) 
                {
                  case 11 :
                    m_el.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
                    break ;
                  case 13 : 
                    m_mu.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
                    break ;
                  case 15 : 
                    m_ta.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
                    break ;
                }
            }
          else if (abs (partType) == 12 || abs (partType) == 14 || abs (partType) == 16)
            {
              m_vs.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
              switch(abs (partType)) 
                {
                  case 12 :
                    m_ve.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
                    break ;
                  case 14 : 
                    m_vm.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
                    break ;
                  case 16 : 
                    m_vT.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
                    break ;
                }
            }
          else if (abs (partType) == 21 || abs (partType) == 9)
            {
              m_gl.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
              m_je.push_back (std::pair<int, TLorentzVector&> (partType, m_particles.back ().second)) ;
            }
        } // if (makeClasses)
    } //PG loop over particles in the event
  ++m_evtNum ;
  return true ;
}


//PG ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


using namespace std ;

int main (int argc, char ** argv)
{
  LHEreader reader (argv[1]) ;
  int maxNum = -1 ;
  if (argc > 2) maxNum = atoi (argv[2]) ;
  string outFileName = "output" ;
  if (argc > 3) outFileName = argv[3] ;

  TNtuple mll ("mll", "mll", "mll") ;

  //PG loop over events
  while (reader.readEvent ())
    {
      if (maxNum > 0 && reader.m_evtNum > maxNum) break ;
      if (reader.m_le.size () != 2) continue ;
      TLorentzVector dilepton = reader.m_le.at (0).second + reader.m_le.at (1).second ;
      mll.Fill (dilepton.M ()) ;
    } //PG loop over events

  outFileName += ".root" ;
  TFile outFile (outFileName.c_str (), "recreate") ;
  mll.Write () ;
  outFile.Close () ;

  return 0 ;
}

