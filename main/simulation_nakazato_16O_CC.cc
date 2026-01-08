/*Hardcoded for oxygen ve charged-current transitions to 16F

  Modified input arguments:
  [1] - Ex (MeV)
  [2] - nps
  [3] - seed (optional)
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip> 
#include <TROOT.h>
#include <TStyle.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TF2.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TArrow.h>
#include <TText.h>
#include <TPaveText.h>
#include <THStack.h>
#include <TGeoManager.h>

#include "NucDeExUtils.hh"
#include "NucDeExRandom.hh"
#include "NucDeExDeexcitation.hh"
#include "NucDeExEventInfo.hh"

int main(int argc, char* argv[]){
  if(argc<3){
    std::cerr << "Error! Requred input: " << argv[0] << " [Ex (MeV)] [nps] [Random Seed (optional)]" << std::endl;
    return 0;
  }

  //Parse args
  double Ex = atof(argv[1]);
  const int numofevent = atoi(argv[2]);
  int seed=1; // default: 1
  if(argc==4) seed = atoi(argv[3]);

  //Constants
  const int ldmodel = 2; //Back-shfited Fermi Gas
  const bool parity_optmodall = true; //Default optical model
  const int verbose = 0;

  //Hardcoded target/residual
  const int Z_r = 9;
  const int N_r = 7;
  const int Z_t = 8;
  const int N_t = 8;

  //Printing
  std::cout << "VERBOSE = " << verbose << std::endl;
  std::cout << "SEED = " << seed << std::endl;

  // ---- FIXME --- // 
  const bool flag_fig=0; //Plot event by event, slow and only for debugging

  std::ostringstream os;

  //Set-up parameters
  new TGeoManager("NucDeEx", "NucDeEx");
  NucDeEx::Utils::fVerbose=verbose; // optional (default: 0)
  NucDeEx::Random::SetSeed(seed); // optional (default: 1)
  NucDeExDeexcitation* deex = new NucDeExDeexcitation(ldmodel, parity_optmodall,2);// new v2.1~
  deex->Init();

  //Prepare output file
  os.str("");
  os << "sim_out/16O/";
  os << "Ex_" << std::fixed << std::setprecision(3) << Ex;
  os << "_ldmodel" << ldmodel;
  os << "_parity" << (parity_optmodall ? 1 : 0);
  os << ".root";
  TFile* outf = new TFile(os.str().c_str(), "RECREATE");
  TTree* tree = new TTree("tree", ""); //Lab Frame, MEV

  //TTree branches
  int eventID=0, size, shell;
  //
  int PDG[NucDeEx::bins];
  double mass[NucDeEx::bins];
  double totalE[NucDeEx::bins],KE[NucDeEx::bins];
  double PMag[NucDeEx::bins], PX[NucDeEx::bins],PY[NucDeEx::bins],PZ[NucDeEx::bins];
  string decay;
  tree->Branch("eventID",&eventID,"eventID/I");
  tree->Branch("decay",&decay);
  tree->Branch("Ex_MeV",&Ex,"Ex_MeV/D");
  tree->Branch("shell",&shell,"shell/I");
  tree->Branch("size",&size,"size/I");
  tree->Branch("PDG",&PDG,"PDG[size]/I");
  tree->Branch("mass_MeV",&mass,"mass_MeV[size]/D");
  tree->Branch("totalE_MeV",&totalE,"totalE_MeV[size]/D");
  tree->Branch("KE_MeV",&KE,"KE_MeV[size]/D");
  tree->Branch("PMag_MeV",&PMag,"PMag_MeV[size]/D");
  tree->Branch("PX_MeV",&PX,"PX_MeV[size]/D");
  tree->Branch("PY_MeV",&PY,"PY_MeV[size]/D");
  tree->Branch("PZ_MeV",&PZ,"PZ_MeV[size]/D");

  //Optional output plots of events
  gStyle->SetTextSize(0.08);
  gStyle->SetTitleSize(0.045);
  gStyle->SetTitleXSize(0.045);
  gStyle->SetTitleYSize(0.045);
  gStyle->SetTitleYOffset(0.95);

  TCanvas* c_detail;
  string pdfname;
  if (flag_fig) {
    os.str("");
    os << "fig_sim/16O/";
    os << "Ex_" << std::fixed << std::setprecision(3) << Ex;
    os << "_ldmodel" << ldmodel;
    os << "_parity" << (parity_optmodall ? 1 : 0);
    os << "_detail.pdf";

    pdfname = os.str();

    c_detail = new TCanvas("c_detail", "", 0, 0, 800, 800);
    c_detail->Print((pdfname + "[").c_str());
    c_detail->Update();
    c_detail->Clear();
  }

  //Set-up for main loop
  TVector3 Pinit(0,0,0);

  //Main loop
  while(eventID<numofevent){
    
    // --- DO SIMULATION --- //
    NucDeExEventInfo result = deex->DoDeex(Z_t,N_t,Z_r,N_r,Ex,Pinit);

    // --- Storing --- //
    shell = result.fShell;
    vector<NucDeExParticle> particle = result.ParticleVector;
    size = 0;
    os.str("");

    for (auto& p : particle) {
      if (!p._flag) continue;   // skip intermediate daughters

      PDG[size]    = p._PDG;
      mass[size]   = p._mass;
      totalE[size] = p.totalE();
      KE[size]     = p.kE();
      PMag[size]   = p._momentum.Mag();
      PX[size]     = p._momentum.X();
      PY[size]     = p._momentum.Y();
      PZ[size]     = p._momentum.Z();
      
      // for decay mode string
      if (p._name.length() > 4)
        os << p._name.substr(0,1);
      else
        os << p._name;

      size++;
    }

    decay = os.str();
    tree->Fill();

    // prepare detail fig
    if(flag_fig){
      TArrow* lXY[size], *lYZ[size], *lXZ[size];
      std::vector<int> color(size, 0);
      for(int i=0;i<size;i++){
        lXY[i] = new TArrow(0,0,PX[i],PY[i],0.01,"|>");
        lYZ[i] = new TArrow(0,0,PY[i],PZ[i],0.01,"|>");
        lXZ[i] = new TArrow(0,0,PX[i],PZ[i],0.01,"|>");
        lXY[i]->SetLineWidth(1);
        lYZ[i]->SetLineWidth(1);
        lXZ[i]->SetLineWidth(1);
        color[i]=400+1;
        for(int p=0;p<NucDeEx::num_particle;p++){
          if(PDG[i] == NucDeEx::PDG_particle[p]) color[i]=NucDeEx::color_root[p];
        }
        lXY[i]->SetLineColor(color[i]);
        lYZ[i]->SetLineColor(color[i]);
        lXZ[i]->SetLineColor(color[i]);
        lXY[i]->SetFillColor(color[i]);
        lYZ[i]->SetFillColor(color[i]);
        lXZ[i]->SetFillColor(color[i]);
      }
      c_detail->Divide(2,2);
      //
      c_detail->cd(1);
      TH1F* waku1= gPad->DrawFrame(-200,-200,200,200);
      waku1->SetTitle("XY plane");
      waku1->GetXaxis()->SetTitle("Px (MeV)");
      waku1->GetYaxis()->SetTitle("Py (MeV)");
      for(int i=0;i<size;i++){
        lXY[i]->Draw();
      }
      //
      c_detail->cd(2);
      TH1F* waku2= gPad->DrawFrame(-200,-200,200,200);
      waku2->SetTitle("YZ plane");
      waku2->GetXaxis()->SetTitle("Py (MeV)");
      waku2->GetYaxis()->SetTitle("Pz (MeV)");
      for(int i=0;i<size;i++){
        lYZ[i]->Draw();
      }
      //
      c_detail->cd(3);
      TH1F* waku3= gPad->DrawFrame(-200,-200,200,200);
      waku3->SetTitle("XZ plane");
      waku3->GetXaxis()->SetTitle("Px (MeV)");
      waku3->GetYaxis()->SetTitle("Pz (MeV)");
      for(int i=0;i<size;i++){
        lXZ[i]->Draw();
      }
      //
      c_detail->cd(4);
      TPaveText* t = new TPaveText(0.1,0.1,0.9,0.9);
      os.str("");
      os << "eventID = " << eventID;
      t->AddText(os.str().c_str());
      os.str("");
      os << "Ex = " << fixed << setprecision(2) << Ex
         << ",  shell = " << shell;
      t->AddText(os.str().c_str());
      //
      t->AddText("--- Decay products ---");
      for(int i=0;i<size;i++){
        os.str("");
        os << "PDG " << PDG[i]
          << ": KE = " << fixed << setprecision(2) << KE[i] << " MeV"
          << ", P=(" << setprecision(1)
          << PX[i] << ", " << PY[i] << ", " << PZ[i] << ")";
        t->AddText(os.str().c_str());
        ((TText*)t->GetListOfLines()->Last())->SetTextColor(color[i]);
      }
      t->AddText("---------------------");
      t->AddText("LAB frame, MeV");
      ((TText*)t->GetListOfLines()->Last())->SetTextSize(0.045);
      ((TText*)t->GetListOfLines()->Last())->SetTextColor(kGray+1);
      t->Draw("same");
      //
      c_detail->Print(pdfname.c_str());
      c_detail->Update();
      c_detail->Clear();
    }

    eventID++;
  }

  if(flag_fig){
    c_detail->Print( (pdfname + (string)"]").c_str() );
    delete c_detail;
  }

  outf->cd();
  tree->Write();
  outf->Close();
  delete outf;

  return 0;
}
