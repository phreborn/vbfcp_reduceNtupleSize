#include <iterator>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <dirent.h>

#include <sstream>
#include <fstream>

using namespace std;


int fileNameFilter(const struct dirent *cur) {
    std::string str(cur->d_name);
    if (str.find("..") != std::string::npos) {
        return 0;
    }
    return 1;
}

std::vector<std::string> getDirBinsSortedPath(std::string dirPath) {
    struct dirent **namelist;
    std::vector<std::string> ret;
    int n = scandir(dirPath.c_str(), &namelist, fileNameFilter, alphasort);
    if (n < 0) {
        return ret;
    }
    for (int i = 0; i < n; ++i) {
        std::string filePath(namelist[i]->d_name);
        ret.push_back(filePath);
        free(namelist[i]);
    };
    free(namelist);
    return ret;
}

static bool readConfigFile(const char * cfgfilepath, const string & key, string & value)
{
    fstream cfgFile;
    cfgFile.open(cfgfilepath);
    if( ! cfgFile.is_open())
    {
        cout<<"can not open cfg file!"<<endl;
        return false;
    }
    char tmp[1000];
    while(!cfgFile.eof())
    {
        cfgFile.getline(tmp,1000);
        string line(tmp);
        size_t pos = line.find(':');
        if(pos==string::npos) continue;
        string tmpKey = line.substr(0,pos);
        if(key==tmpKey)
        {
            value = line.substr(pos+1);
        }
    }
    return false;
}

void getSysList(TChain &tree, TString anchorVar, std::vector<TString> &sysList){
  TObjArray *branches = tree.GetListOfBranches();

  for(int i = 0; i < branches->GetEntries(); i++){
    TBranch *branch = (TBranch *)branches->At(i);
    TString bname = branch->GetName();

    if (!bname.Contains(anchorVar)) { continue; }

    bname.ReplaceAll("."+anchorVar, "");

    if (bname.IsNull()) { continue; } cout<<bname<<endl;

    sysList.push_back(bname);
  }
}

void reduceSysNtupleSize(TString spSys, int sysInit, int sysFin){
  bool doSys = true;

  map<TString, int> sampleID;
  sampleID["VBF"] = 346214;
  sampleID["ggF"] = 343981;

  //TString dirpath = "/eos/user/a/ahabouel/VBFCP/";
  //TString dirpath = "/eos/atlas/atlascerngroupdisk/phys-higgs/HSG1/VBFCP/h025/";
  TString dirpath = "/eos/user/h/huirun/vbf_cp/h026/sys/";

  map<TString,double> lumi;
  lumi["mc16a"] = 36207.66;
  lumi["mc16d"] = 44307.4;
  lumi["mc16e"] = 58450.1;

  TString specificSysSet = "";
  //specificSysSet = "jetsys";
  //specificSysSet = "photonallsys1";
  //specificSysSet = "photonallsys2";

  int iSysInit = 1;
  int iSysFin = 9999;

  specificSysSet = spSys;
  iSysInit = sysInit;
  iSysFin = sysFin;

  map<TString, vector<TString>> failSysList;

  std::string path_str = dirpath.Data();
  std::vector<std::string> sub_dirs = getDirBinsSortedPath(path_str);

  std::vector<std::string> files(0);

  for(auto d : sub_dirs){
    if(d==".") continue;
    if(d.find("mc") == std::string::npos) continue;
    cout<<"d: "<<path_str+d<<endl;
    std::vector<std::string> fs = getDirBinsSortedPath(path_str+d+"/");
    for(auto f : fs){
      if(f==".") continue;
      if(f.find(".root") == std::string::npos) continue; // should be root file in /mc16*
      if(f.find("Sherpa_yy") != std::string::npos) continue; // not using full simulation yy sample
      cout<<"f: "<<path_str+"/"+d+"/"+f<<endl;
      files.push_back(path_str+d+"/"+f);

    }
  }

  for(auto sample : sampleID){
    TString sampleName = sample.first;
    int iID = sample.second;
    for(auto mc : lumi){
      TString camp = mc.first; cout<<endl<<sampleName<<" "<<camp<<endl;

      std::vector<std::string> subFiles(0);
      //TChain ch("output", "output");
      for(auto f : files){
        if(f.find((sampleName+"_").Data()) == std::string::npos) continue;
        if(f.find(camp.Data()) == std::string::npos) continue;
        if(specificSysSet!="" && f.find(specificSysSet.Data()) == std::string::npos) continue;
        TString filepath = f.data();
        //ch.Add(filepath); cout<<"chain added: "<<filepath<<endl;
        subFiles.push_back(f); cout<<"sub file: "<<filepath<<endl;
      }

      for(auto f : subFiles){
        TString filepath = f.data(); cout<<endl<<filepath<<endl;

        TChain ch("output", "output");
        ch.Add(filepath);

        TString inRootFileName = filepath.Replace(filepath.First('/'), filepath.Last('/')-filepath.First('/')+1, "");
        inRootFileName = inRootFileName.ReplaceAll(".root", ""); cout<<inRootFileName<<endl<<endl;

        TString sysSet = sampleName+"_"+specificSysSet;

          //TString dirName = camp+"/"+sysSet;
          TString dirName = camp+"/"+inRootFileName;
          TString tsCommand = "mkdir -p "+dirName;
          system(tsCommand.Data());

        std::vector<TString> sysList;
        sysList.clear();
        if(doSys) getSysList(ch, "m_yy", sysList);
        else sysList.push_back("Nominal");

        bool nominalProcessed = false;
        int test_counter = 0;
        for(auto sys : sysList){
          test_counter++;
          //if(test_counter != 2) continue;
          if(test_counter < iSysInit || test_counter > iSysFin) continue;
          //if(test_counter > iSysFin) break;

          if(sys == "Nominal") nominalProcessed = true;

          char *sysName = (char*)sys.Data();
          string sSys = sys.Data();

          //ROOT::RDataFrame df(ch, {"Nominal.m_yy"});
          ROOT::RDataFrame df("output", f);

          if(!df.HasColumn(Form("%s.isPassed", sysName))){
            failSysList[camp+"_"+sampleName].push_back(sys);
            continue;
          }

          string cuts = Form("%s.isPassed && !isDalitz && %s.N_j_30>1 && %s.m_jj_30/1000>400 && (%s.DeltaEta_jj<-2 || %s.DeltaEta_jj>2) && (%s.Zepp>-5 && %s.Zepp<5)", sysName, sysName, sysName, sysName, sysName, sysName, sysName); cout<<"cuts : "<<endl<<cuts<<endl;

          auto df_cut = df.Filter(cuts)
                          .Alias(Form("%s_xsec_kF_eff", sysName), "xsec_kF_eff")
                          .Alias(Form("%s_WeightDtilde1", sysName), "WeightDtilde1")
                          .Alias(Form("%s_WeightDtilde2", sysName), "WeightDtilde2")
                          .Alias(Form("%s_isDalitz", sysName), "isDalitz");

          string nameVar = sSys+".*";
          //string nameVar = "(^[^\.]+$|^"+sSys+".*$)";
          df_cut.Snapshot(sSys, Form("%s/%i_%s_%s.root", dirName.Data(), iID, sampleName.Data(), sysName), nameVar);

          if(sSys=="Nominal"){
            TFile *f1 = TFile::Open(f.data()); cout<<f.data()<<endl;
            TFile *f_nom = new TFile(Form("%s/%i_%s_%s.root", dirName.Data(), iID, sampleName.Data(), sysName), "update");
            for(auto k : *f1->GetListOfKeys()) { // refer to io/loopdir11.C
              TKey *key = static_cast<TKey*>(k);
              TClass *cl = gROOT->GetClass(key->GetClassName());
              if (!cl->InheritsFrom("TH1")) continue;
              TH1 *h = key->ReadObject<TH1>(); cout<<h->GetName()<<endl;
              f_nom->cd();
              h->Write();
            }
            f_nom->Close();
            delete f_nom;
          }

        }// sys
      }// file

///*** merge mxaod cutflows and save in nominal root file  ***/
//      if(! nominalProcessed) continue;
//      ifstream f(Form("%s/%i_%s_%s.root", dirName.Data(), iID, sampleName.Data(), "Nominal"));
//      if(! f.good()) continue;
//
//      TFile *f_nom = new TFile(Form("%s/%i_%s_%s.root", dirName.Data(), iID, sampleName.Data(), "Nominal"), "update");
//
//      std::map<TString, TH1F*> mxaodCutflows;
//      int nFiles = 0;
//      for(auto f : files){
//        if(f.find((sampleName+"_").Data()) == std::string::npos) continue;
//        if(f.find(camp.Data()) == std::string::npos) continue;
//        if(specificSysSet!="" && f.find(specificSysSet.Data()) == std::string::npos) continue;
//        TString filepath = f.data();
//        TFile *f1 = TFile::Open(f.data()); cout<<f.data()<<endl;
//        if(nFiles == 0){
//          for(auto k : *f1->GetListOfKeys()){
//            TKey *key = static_cast<TKey*>(k);
//            TClass *cl = gROOT->GetClass(key->GetClassName());
//            if (!cl->InheritsFrom("TH1")) continue;
//            TH1 *h = key->ReadObject<TH1>(); cout<<h->GetName()<<endl;
//            mxaodCutflows[h->GetName()] = (TH1F*) h->Clone((TString)h->GetName());
//          }
//        }else if(nFiles > 0){
//          for(auto k : *f1->GetListOfKeys()){
//            TKey *key = static_cast<TKey*>(k);
//            TClass *cl = gROOT->GetClass(key->GetClassName());
//            if (!cl->InheritsFrom("TH1")) continue;
//            TH1 *h = key->ReadObject<TH1>(); cout<<h->GetName()<<endl;
//            mxaodCutflows[h->GetName()]->Add(h);
//          }
//        }
//        nFiles++;
//      }
//
//      f_nom->cd();
//      for(auto h : mxaodCutflows){
//        h.second->Write();
//      }
//      f_nom->Close();
//      delete f_nom;
///******/

    }//camp
  }// sample

  for(auto file : failSysList){
    cout<<endl<<file.first<<endl;
    for(auto sys : file.second){
      cout<<sys<<"\t";
    }
  }

}
