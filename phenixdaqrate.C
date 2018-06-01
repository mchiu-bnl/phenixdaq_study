//
// estimate optimal livetime vs daq rate
//
#include <TGraph.h>
#include <TFile.h>
#include <TH1.h>
#include <TRandom3.h>
#include <TString.h>
#include <TPad.h>
#include <iostream>

TGraph *g_haggerty = 0;		// measured raw rate curve using BBC (from noise triggers)
TGraph *g_model = 0;		// the modeled livetime vs raw rate curve
TGraph *g_modelreal = 0;	// the modeled livetime vs real event rate curve (accepted by DAQ)

// Get John Haggerty's max curve
void gethaggerty(const char *fname = "livetime_measurements_20100129.txt")
{
  g_haggerty = new TGraph(fname);
  g_haggerty->SetName("g_haggerty");
  g_haggerty->GetHistogram()->SetXTitle("raw rate (Hz)");
  g_haggerty->GetHistogram()->SetYTitle("livetime");
}

void phenixdaqrate(const int max_converttime = 550,
    const int max_endat = 2*400,
    const int max_counter = 4,
    const int ncrossings = 20000000)
{
  const int max_deadfor = 15;		// in clocks
  //const int max_endat = 2*480;		// in clocks, mplex 2, seems to match data
  //const int max_endat = 2*400;		// in clocks, mplex 2, for EMCAL
  //const int max_converttime = 550;	// in clocks
  //const int max_counter = 4;		// number of triggers in buffer (count-4)

  // set up dead crossings (excludes PC FEM unreliable since )
  Int_t dead_crossing[120];       // these are crossings with collisions (non-abort gap) but deaded out by us
                                  // examples are GL1 reset, VTXS reset, etc
  for (int i=0; i<120; i++)
  {
    dead_crossing[i] = 0;
  }
  //dead_crossing[1] = 1; // GL1 Reset
  //dead_crossing[60] = 1; // VTXS Reset
 
  Int_t pcfem_unreliable = 0; // whether to enable pcfem unreliable

  gethaggerty();
  TString name = "noise (black) compared to model (red), endat ";
  name += max_endat/2; name += ", convert "; name += max_converttime;
  g_haggerty->GetHistogram()->SetTitle(name);
  //g_haggerty->Draw("al");
  //gPad->SetGridx(1);
  //gPad->SetGridy(1);

  TRandom3 trand(0);

  TFile *savefile = new TFile("daqrate.root","RECREATE");
  TH1 *hrand = new TH1F("hrand","random number",1000,0,1);
  TH1 *hrawcross = new TH1F("hrawcross","crossing",120,0,120);  // Raw triggers by crossing
  TH1 *hcross = new TH1F("hcross","crossing",120,0,120);        // Accepted trigs by crossing
  hcross->SetLineColor(2);
  // the time in ticks between accepted triggers
  TH1 *h_prev[1000] = {0};

  Int_t npoints = 0;
  Double_t rawrate[1000];
  Double_t eventrate[1000];
  Double_t model_livetime[1000];

  // rate here is the raw event rate
  //for (double rate=1000.; rate<=2500000.; rate+=1000.)
  //for (double rate=100.; rate<=25000.; rate+=100.)
  for (double rate=1000.; rate<=50000.; rate+=1000.)
  {
    // prob for collision
    double prob = rate/(9.38e6*(110./120.));  // The 110/120 factor accounts for the abort gap

    int deadfor = 0;
    int isdeadbusy = 0;	// deadfor busy
    int iscountbusy = 0;	// count-4 busy
    int clockcounter = 0;		// clock counter
    int prevtrigclock = -1;	// clock counter of last trigger
    int nlivetrig = 0;
    int nrawtrig = 0;
    int bufcounter = 0;		// gtm count-5 counter
    int endatcounter[1000];
    int convertcounter[1000];

    name = "h_prev"; name += int(rate);
    TString title = "Time Between Triggers, Rate = "; title += int(rate);
    h_prev[npoints] = new TH1F(name,title,10000,0,10000);

    for (int icount=0; icount<max_counter; icount++)
    {
      endatcounter[icount] = 0;
      convertcounter[icount] = 0;
    }

    for (int icross=0; icross<ncrossings; icross++)
    {
      clockcounter++;

      int crossing = icross%120;  // crossing number (0-119)

      /*
         cout << clockcounter << " " << isdeadbusy << " " << iscountbusy << "  ";
         for (int ibuf=0; ibuf<bufcounter; ibuf++)
         {
         cout << " " << convertcounter[ibuf] << " " << endatcounter[ibuf];
         }
         cout << endl;
         */

      Double_t rndm = trand.Rndm();
      hrand->Fill( rndm );

      // Increment the deadfor counter
      if ( deadfor>0 && deadfor<=max_deadfor )
      {
        deadfor++;
        isdeadbusy = 1;

        if ( deadfor==(max_deadfor+1) )
        {
          deadfor = 0;
          isdeadbusy = 0;
        }
      }

      // Keep track of the count-4 counter
      if ( bufcounter>0 )
      {
        for (int ibuf=0; ibuf<max_counter; ibuf++)
        {
          if ( convertcounter[ibuf]==0 && ibuf==0 )
          {
            // decrement endat if convert is 0
            endatcounter[0]--;

            // we reached end of endat, data is sent!
            if ( endatcounter[0]==0 )
            {
              // decrement buffer counter
              bufcounter--;

              for (int ibuf2=1; ibuf2<max_counter; ibuf2++)
              {
                convertcounter[ibuf2-1] = convertcounter[ibuf2];
                endatcounter[ibuf2-1] = endatcounter[ibuf2];
              }
            }
          }

          if ( convertcounter[ibuf]>0 )
          {
            convertcounter[ibuf]--;
            break;
          }
        }

        if ( bufcounter<max_counter ) iscountbusy = 0;
      }

      // Check if we trigger, no triggers in abort gap (crossings 110-119)
      if ( rndm<prob && crossing<110 )
      {
        nrawtrig++;

        // Can't trigger on dead crossings
        if ( dead_crossing[crossing] == 1 ) continue;

        // PC FEM unreliable 
        if ( pcfem_unreliable==1 && ((icross/120)%101)==100 && crossing<60 )
        {
          continue;
        }

        // Found a rawtrig
        hrawcross->Fill(crossing);
 
        // If not busy, enable trigger
        if ( isdeadbusy==0 && iscountbusy==0 )
        {
          nlivetrig++;
          deadfor = 1;
          isdeadbusy = 1;

          // Found an accepted trig
          hcross->Fill(crossing);

          convertcounter[bufcounter] = max_converttime;
          endatcounter[bufcounter] = max_endat;
          bufcounter++;

          if ( bufcounter==max_counter )
          {
            iscountbusy = 1;
          }

          // keep track of counts to prev trigger
          if ( prevtrigclock >= 0 )
          {
            int trigdiff = clockcounter - prevtrigclock;
            h_prev[npoints]->Fill(trigdiff);
          }
          prevtrigclock = clockcounter;
        }

      }
    }	// loop over crossings

    double livetime = (double)nlivetrig/(double)nrawtrig;
    cout << rate << "\t" << nlivetrig << "\t" << nrawtrig << "\t" << clockcounter << "\t" << livetime << endl;
    rawrate[npoints] = rate;
    eventrate[npoints] = rate*livetime;
    model_livetime[npoints] = livetime;
    npoints++;

  }	// loop over rates

  cout << "npoints " << npoints << endl;

  // Livetime vs Raw Event Rate
  //hrand->Draw();
  g_model = new TGraph(npoints,rawrate,model_livetime);
  g_model->SetName("model");
  g_model->SetLineColor(2);
  g_model->SetMarkerColor(2);
  g_model->Draw("al");
  g_model->GetHistogram()->SetXTitle("rate (Hz)");
  g_model->GetHistogram()->SetYTitle("Livetime");
  g_model->GetHistogram()->SetTitle("PHENIX DAQ Model");
  g_haggerty->Draw("lsame");
  gPad->SetGridx(1);
  gPad->SetGridy(1);

  // Livetime vs Accepted Event Rate
  g_modelreal = new TGraph(npoints,eventrate,model_livetime);
  g_modelreal->SetName("modelreal");
  g_modelreal->SetLineColor(8);
  g_modelreal->SetMarkerColor(8);
  g_modelreal->Draw("lsame");

  g_model->Write();
  g_modelreal->Write();

  savefile->Write();
  savefile->Close();

}

