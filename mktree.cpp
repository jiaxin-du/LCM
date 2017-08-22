///////////////////////////////////////////////////////////////
//
// This program demonstrates how to read the output of LCM and 
//   put them in a ROOT file
//  
//  ROOT is free data analysis framework built on C++.
//
//  See http://root.cern.ch for more information about ROOT 
//
//  You need both the ROOT library and the LCM source code
//    to compile the program.
//
//   Assuming you have the LCM source code in directory "src" 
//     and a working root software installed, then you can compile
//     the program using the following command
//
//  g++ -o mktree mktree.cpp src/*.cpp -O2 $(root-config --cflags --glibs)
//
//  Usage:
//    ./mktree volt.dat volt.root
//
//  where "volt.dat" is the input file, which is the LCM output file, 
//    and "volt.root" is the root file to be created.
//  The third argument is optional. If not given, the root file 
//    would be the same as the input file with the extension 
//    changed to ".root"
/////////////////////////////////////////////////////////////

#include <cstdio>
#include <iostream>
#include <string>
#include <map>

#include <omp.h>

#include "TNamed.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TVirtualFFT.h"

#include "src/simulation.h"

using namespace std;
using std::string;
using std::map;

//get the directory of the given file 
string get_dir(const string& fpath, const string& path_sep = "/\\")
{
    std::size_t found = fpath.find_last_of(path_sep);
    if (found == string::npos) {
        return string("."); //current directory
    }
    else {
        return(fpath.substr(0, found));
    }
}

string get_name(const string& fpath, const string& path_sep = "/\\")
{
    string fname(fpath);

    std::size_t found = fpath.find_last_of(path_sep);
    if (found != string::npos) {
        fname = fpath.substr(found + 1, string::npos);
    }

    found = fname.find_last_of(".");

    if (found != string::npos) {
        return fname.substr(0, found);
    }
    else {
        return fname;
    }
}

Int_t main(Int_t argc, char* argv[])
{

    if (argc != 2 & argc != 3) {
        cerr << "usage: " << argv[0] << " volt_data_file" << endl;
        exit(-1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        cerr << "ERROR! failed to open file '" << argv[1] << "' !" << endl;
        exit(-1);
    }

    map<string, string> fileInfo;
    //read and processing the header
    {
        char *buff = new char[1024];

        fread(buff, sizeof(char), 1024, fp);

        string str = buff;
        vector<string> parts;

        strsplit(str, "\n", parts);

        Int_t pos1, pos2;
        string paraName, paraVal;
        for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it) {
            *it = strtrim(remove_comments(*it));
            if (it->size() == 0) continue;
            pos1 = it->find("=");
            pos2 = it->find(";");
            if (pos1 > pos2) {
                cerr << "ERROR! cannnot recognise the line " << *it << endl;
                fclose(fp);
                exit(-1);
            }
            paraName = upperstr(strtrim(it->substr(0, pos1)));
            paraVal = upperstr(strtrim(it->substr(pos1 + 1, pos2 - pos1 - 1)));
            fileInfo[paraName] = paraVal;
        }
        delete[] buff;
    }

    Int_t ng_num, elmt_num;
    Int_t section_num;
    Int_t cfg_pos, cfg_len;
    Int_t num_size, block_size, block_num;
    Int_t data_pos, data_len;

    //proccessing the header
    {
        if ((!str2int(fileInfo["NEUR_NUM"], ng_num)) || ng_num < 0) {
            if ((!str2int(fileInfo["NG_NUM"], ng_num)) || ng_num < 0) {
                cerr << "ERROR! cannot recognise the value for 'NEUR_NUM' or 'NG_NUM': NEUR_NUM = '" <<
                    fileInfo["NEUR_NUM"] << "', NG_NUM = " << fileInfo["NG_NUM"] << "!" << endl;
                fclose(fp);
                exit(-1);
            }
        }
        if ((!str2int(fileInfo["ELMT_NUM"], elmt_num)) || elmt_num < 0) {
            cerr << "ERROR! cannot recognise the value for 'ELMT_NUM': '" << fileInfo["ELMT_NUM"] << "'!" << endl;
            fclose(fp);
            exit(-1);
        }

        if ((!str2int(fileInfo["CFG_POS"], cfg_pos)) || cfg_pos < 0) {
            cerr << "ERROR! cannot recognise the value for 'CFG_POS': '" << fileInfo["CFG_POS"] << "'!" << endl;
            fclose(fp);
            exit(-1);
        }

        if ((!str2int(fileInfo["CFG_LEN"], cfg_len)) || cfg_len < 0) {
            cerr << "ERROR! cannot recognise the value for 'CFG_LEN': '" << fileInfo["CFG_LEN"] << "'!" << endl;
            fclose(fp);
            exit(-1);
        }

        if ((!str2int(fileInfo["NUM_SIZE"], num_size)) || num_size < 0) {
            cerr << "ERROR! cannot recognise the value for 'NUM_SIZE': '" << fileInfo["NUM_SIZE"] << "'!" << endl;
            fclose(fp);
            exit(-1);
        }

        if (num_size != sizeof(float)) {
            cerr << "ERROR! Data type is not right. Only float type data are supported! num_size = " << num_size << endl;
            fclose(fp);
            exit(-1);
        }
        if ((!str2int(fileInfo["BLOCK_SIZE"], block_size)) || block_size < 0) {
            cerr << "ERROR! cannot recognise the value for 'BLOCK_SIZE': '" << fileInfo["BLOCK_SIZE"] << "'!" << endl;
            fclose(fp);
            exit(-1);
        }
        if ((!str2int(fileInfo["BLOCK_NUM"], block_num)) || block_num < 0) {
            cerr << "ERROR! cannot recognise the value for 'BLOCK_NUM': '" << fileInfo["BLOCK_NUM"] << "'!" << endl;
            fclose(fp);
            exit(-1);
        }

        if ((!str2int(fileInfo["DATA_POS"], data_pos)) || (data_pos < 0)) {
            cerr << "ERROR! cannot recognise the valure for 'DATA_POS': " << fileInfo["DATA_POS"] << "! " << endl;
            fclose(fp);
            exit(-1);
        }

        if ((!str2int(fileInfo["DATA_LEN"], data_len)) || (data_len < 0)) {
            cerr << "ERROR! cannot recognise the valure for 'DATA_LEN': " << fileInfo["DATA_LEN"] << "! " << endl;
            fclose(fp);
            exit(-1);
        }

        Int_t curr_pos = ftell(fp); // save current position

        fseek(fp, 0, SEEK_END); // go the end of the file
        Int_t end_pos = ftell(fp); // length of the file

        if ((end_pos - data_pos + 2) / block_size < block_num) {
            block_num = (end_pos - data_pos + 2) / block_size;
            cerr << "WARNING! it seems the data blocks in '" << argv[1] << "' the file is smaller than it claims!" << endl;
            cerr << "  " << block_num << " blocks are found in the file. data_pos = " << data_pos << ", file_size = " << end_pos << "." << endl;
            fclose(fp);
            exit(-1);
        }
        fseek(fp, curr_pos, SEEK_SET);
    }

    //
    //
    //The following section parse the parameter configuration section
    //and save the parameter name and value pair to th variable modelInfo
    LCM lcm;
    map<string, string> modelInfo;
    //read the parameter section, and use that to initialise a LCM model
    {
        char *cfg = new char[cfg_len + 1];

        std::fill(cfg, cfg + cfg_len + 1, '\0');

        fseek(fp, cfg_pos, SEEK_SET);
        fread(cfg, sizeof(char), cfg_len, fp);

        read_param(cfg, modelInfo);
        map<string, string> paraList = modelInfo;

        for (map<string, string>::iterator it = paraList.begin(); it != paraList.end(); ++it) {
            if ((it->first).substr(0, 5) == string("SIMU.")) {
                paraList.erase(it);
                it = paraList.begin();
            }
        }

        lcm.set_param(paraList);

        if (!lcm.init()) {
            cerr << "ERROR! Initialising LCM failed!" << endl;
            exit(-1);
        }

        if (lcm.ng_num() != ng_num || lcm.elmt_num() != elmt_num) {
            cout << "ERROR! Info in header section is not consistent with that in config section!" << endl;
            exit(-1);
        }

        //cout<<lcm.print()<<endl;
        delete[] cfg;
    }

    string fpath;
    if (argc == 3) {
        fpath.assign(argv[2]);
    }
    else {
        fpath.assign(argv[1]);

#if defined(_WIN32) || defined(_WIN64)
        fpath = get_dir(fpath) + ("\\") + get_name(fpath) + (".root");
#else
        fpath = get_dir(fpath) + ("/") + get_name(fpath) + (".root");
#endif
    }
    cout << "INFO: save data to " << fpath << endl;

    TFile *hfile = new TFile(fpath.c_str(), "RECREATE", "Neuronal firing rates from LCM", 5);
    if (hfile->IsZombie()) {
        cout << "ERROR: cannot create file '" << fpath.c_str() << "' for writting." << endl;
        exit(-1);
    }
    //
    //The following section save the content in modelInfo  
    //to the root file
    TNamed *pList = new TNamed[modelInfo.size()];

    hfile->mkdir("param");
    hfile->cd("param");

    Int_t idx = 0;
    for (map<string, string>::iterator it = modelInfo.begin(); it != modelInfo.end(); (++it), (++idx)) {
        pList[idx].SetNameTitle((it->first).c_str(), (it->second).c_str());
        pList[idx].Write();
    }
    hfile->cd("/");

    /////////////////////////////////////////////////////
    //
    //In the following section, a tree is created to
    //store information about the model and filled with
    //only one entry
    //
    Int_t nrow = lcm.grid_row();

    Int_t ncol = elmt_num / nrow;

    Int_t volt_num = ng_num * elmt_num;

    Int_t mid_elmt = (nrow / 2) * ncol + ncol / 2;

    Int_t *idx_neur = new Int_t[volt_num];
    Int_t *idx_elmt = new Int_t[volt_num];

    for (Int_t idx = 0; idx < volt_num; ++idx) {
        idx_neur[idx] = idx % ng_num;
        idx_elmt[idx] = idx / ng_num;
    }

    TString neur_name;
    Float_t *neur_density = new Float_t[ng_num];
    Int_t *neur_layer = new Int_t[ng_num];
    Int_t *neur_type = new Int_t[ng_num];

    for (Int_t idx = 0; idx < ng_num; ++idx) {
        if (idx != 0) {
            neur_name += TString(";");
        }
        neur_name += (TString::Format("%d-", idx) + TString(lcm.neur_group(idx).name()));
        neur_density[idx] = lcm.neur_group(idx).density();
        neur_layer[idx] = lcm.neur_group(idx).layer();

        neur_type[idx] = lcm.neur_group(idx).type();
    }

    Int_t layer_num = lcm.layer_num();
    TString layer_name;

    for (Int_t idx = 0; idx < layer_num; ++idx) {
        if (idx != 0) {
            layer_name += TString(";");
        }
        layer_name += (TString::Format("%d-", idx) + TString(lcm.layer(idx).name()));
    }

    TString excit_inhib = TString::Format("%d-EXCIT;%d-INHIB", cEXCIT, cINHIB);

    TTree *tr1 = new TTree("model_info", "LCM information");

    tr1->Branch("nneur", &ng_num, "nneur/I");
    tr1->Branch("ncol", &ncol, "ncol/I");
    tr1->Branch("nrow", &nrow, "nrow/I");
    tr1->Branch("nelmt", &elmt_num, "nelmt/I");
    tr1->Branch("nvolt", &volt_num, "nvolt/I"); // volt_num == ng_num * elmt_num;
    tr1->Branch("nlayer", &layer_num, "nlayer/I");
    tr1->Branch("mid_elmt", &mid_elmt, "mid_elmt/I");

    tr1->Branch("layer_name", (void *)layer_name.Data(), "layer_name/C", 1024);
    tr1->Branch("neur_name", (void *)neur_name.Data(), "neur_name/C", 1024);
    tr1->Branch("excit_inhib", (void *)excit_inhib.Data(), "excit_inhib/C", 1024);

    tr1->Branch("neur_density", neur_density, "neur_density[nneur]/F");
    tr1->Branch("neur_layer", neur_layer, "neur_layer[nneur]/I");
    tr1->Branch("neur_type", neur_type, "neur_type[nneur]/I");

    tr1->Branch("idx_neur", idx_neur, "idx_neur[nvolt]/I");
    tr1->Branch("idx_elmt", idx_elmt, "idx_elmt[nvolt]/I");

    tr1->Fill();

    tr1->Write();

    /////////////////////////////////////////////////////////
    //
    Float_t nneur_E = 0.;
    Float_t nneur_I = 0.;

    for (Int_t idx = 0; idx < ng_num; ++idx) {
        if (neur_type[idx] == cEXCIT) {
            nneur_E += neur_density[idx];
        }
        else {
            nneur_I += neur_density[idx];
        }
    }

    //convert to percentage
    for (Int_t idx = 0; idx < ng_num; ++idx) {
        if (neur_type[idx] == cEXCIT) {
            neur_density[idx] /= nneur_E;
        }
        else {
            neur_density[idx] /= nneur_I;
        }
    }
    /////////////////////////////////////////////////////////
    //
    //In the following section, the voltage information is read
    //and convert into firing rates, both the voltage and FR 
    //information are stored in a tree
    //
    //
    char *pos;
    Float_t tau;

    Float_t *volt = new Float_t[volt_num];
    Float_t *FR = new Float_t[volt_num];

    Float_t *mFR_E = new Float_t[elmt_num];
    Float_t *mFR_I = new Float_t[elmt_num];

    TTree *tr2 = new TTree("neur_volts", "neuron voltages and firing rates");

    tr2->Branch("tau", &tau, "tau/F"); //branch for evolution time
    tr2->Branch("nvolt", &volt_num, "nvolt/I");
    tr2->Branch("nelmt", &elmt_num, "nelmt/I");
    tr2->Branch("volt",   volt,     "volt[nvolt]/F");
    tr2->Branch("FR", FR, "FR[nvolt]/F");
    tr2->Branch("mFR_E", mFR_E, "mFR_E[nelmt]/F");
    tr2->Branch("mFR_I", mFR_I, "mFR_I[nelmt]/F");

    Int_t Nproc = omp_get_num_procs() / 2;//get the number of available processors without hyper-threading.
    if (Nproc > 10) Nproc = 10;

    omp_set_num_threads(Nproc);//set the number of processors.
    omp_set_nested(0);
    omp_set_dynamic(1);

    char ch;
    Int_t ineur, ibgn, ielmt;

    ibgn = mid_elmt * ng_num;

    for (Int_t iblk = 0; iblk < block_num; ++iblk) {

        fseek(fp, data_pos + iblk*block_size, SEEK_SET);
        fread(&tau, num_size, 1, fp); //read tau
        fread(volt, num_size, volt_num, fp); //read voltage data
        fread(&ch, sizeof(char), 1, fp); //read ending '\0'

        if (ch != '\0') {
            cerr << "ERROR! A inappropriately ended data block found!" << endl;
            cerr << " block# = " << iblk << ", pos = " << ftell(fp) << endl;
            break;
        }

        for (ielmt = 0; ielmt < elmt_num; ++ielmt) {
            mFR_E[ielmt] = 0.;
            mFR_I[ielmt] = 0.;
        }

#pragma omp parallel for 
        for (Int_t ielmt = 0; ielmt < elmt_num; ++ielmt) {
            Int_t idx = ielmt * ng_num;

            for (Int_t ineur = 0; ineur < ng_num; ++ineur) {
                FR[idx] = lcm.neur_group(ineur).eqn_firing(volt[idx]);

                if (neur_type[ineur] == cEXCIT) {
                    mFR_E[ielmt] += (FR[idx] * neur_density[ineur]);
                }
                else {
                    mFR_I[ielmt] += (FR[idx] * neur_density[ineur]);
                }

                ++idx;
            }
        }

        tr2->Fill();
    }

    tr2->Write();
    hfile->Close();
    delete hfile;

    delete[] pList;
    delete[] idx_neur;
    delete[] idx_elmt;
    delete[] neur_density;
    delete[] neur_layer;
    delete[] neur_type;

    delete[] volt;
    delete[] FR;
    delete[] mFR_E;
    delete[] mFR_I;

    fclose(fp);
}
