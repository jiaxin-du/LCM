//-------------------------------------------------
//
//          Laminar cortex model
//
// Developed by Jiaxin Du under the supervision of
//    Prof. David Reutens and Dr. Viktor Vegh
//
//       Centre for Advanced Imaging (CAI),
//   The University of Queensland (UQ), Australia
//
//        jiaxin.du@uqconnect.edu.au
//
// Reference:
//  Du J, Vegh V, & Reutens DC,
//                PLOS Compt Biol 8(10): e1002733.
//              & NeuroImage 94: 1-11.
//
// See README for software copyright statements.
//-------------------------------------------------
#include <iomanip>
#include "src/simulation.h"

using namespace std;

//return the command formate
string cmd_format(string cmd)
{
   return string("\nformat: ") + cmd + \
      string(" -p prefix -f para_file -o dat_file -l log_file\n\n" \
      "  -p prefix\t specify the directory for all files (default: NONE).\n" \
      "  -f para_file\t specify parameter configuration file (default: para.cfg).\n" \
      "  -o dat_file\t specify voltage data output file (default: voltage_<time_stamp>.dat).\n" \
      "  -l log_file\t specify the runing log output file (default: run_<time_stamp>.log).\n\n" \
      "if a prefix is specified, it will add to all file names " \
      "that does not contain a '\\' or '/'. \n\n" \
      "for example :\n\n  ") + cmd + \
      string(" -p run_01 \n\n" \
      "will run LCM using parameter file 'run_01/para.cfg', and write voltage " \
      "data to 'run_01/voltage_20150101_120101.dat' and runing log to " \
      "'run_01/run_20150101_120101.log', where '20150101_120101' is the time stamp.\n\n  ") + cmd + \
      string(" -p run_01 -f para.cfg -o ./voltage.dat -l run.log \n\n" \
      "will run LCM using parameter file 'run_01/para.cfg, and " \
      "write output voltage information to './voltage.dat' and " \
      "runing log to 'run_01/run.log'.\n\n");
}

//return a banner
string banner()
{
   return string(
      "//--------------------------------------------------\n"
      "//         Laminar Cortex Model (LCM)\n"
      "//\n"
      "//  by Jiaxin Du, Viktor Vegh and David Reutens\n"
      "//\n"
      "//         jiaxin.du@uqconnect.edu.au\n"
      "//\n"
      "//      Centre for Advanced Imaging (CAI), \n"
      "// The University of Queensland (UQ), Australia\n"
      "//\n"
      "// Reference: Du J, Vegh V & Reutens DC,\n"
      "//                PLOS Comput Biol 8(10): e102733\n"
      "//              & NeuroImage 94: 1-11.\n"
      "//--------------------------------------------------\n"
      );
}

int main(int argc, char **argv)
{
#ifdef _OPENMP
#pragma message "INFO: OpenMP is switched on, good."
#endif

#ifndef NDEBUG
#pragma message "WARNING: debugging option is switched on, the program will be slow!"
#endif
   //get the time stamp
   time_t raw_tm, bgn_tm;
   time(&raw_tm);
   char time_stamp[32];
   strftime(time_stamp, 31, "%Y%m%d_%H%M%S", localtime(&raw_tm));

   //decide the file names
   string prefix = "";
   string para_file = "para.cfg";
   string dat_file = string("volt_") + time_stamp + string(".dat");
   string log_file = string("run_") + time_stamp + string(".log");

   //deal with command argument
   for (TInt idx = 1; idx < argc; idx += 2){
      if (strcmp(argv[idx], "-f") == 0) {
         para_file = strtrim(argv[idx + 1]);
      }
      else if (strcmp(argv[idx], "-p") == 0){
         prefix = strtrim(argv[idx + 1]);
      }
      else if (strcmp(argv[idx], "-o") == 0){
         dat_file = strtrim(argv[idx + 1]);
      }
      else if (strcmp(argv[idx], "-l") == 0){
         log_file = strtrim(argv[idx + 1]);
      }
      else{
         cerr << "ERROR: unrecognised option '" << argv[idx] << "'." << endl;
         cerr << cmd_format(argv[0]) << endl;
         cerr.flush();
         exit(-1);
      }
   }

   //add prefix to file names
   if (!prefix.empty()){

      if (para_file.find_first_of("/\\") == string::npos){
         if (prefix[prefix.size() - 1] == FILE_PATH_SEP){
            para_file = prefix + para_file;
         }
         else{
            para_file = prefix + FILE_PATH_SEP + para_file;
         }
      }

      if (dat_file.find_first_of("/\\") == string::npos){
         if (prefix[prefix.size() - 1] == FILE_PATH_SEP){
            dat_file = prefix + dat_file;
         }
         else{
            dat_file = prefix + FILE_PATH_SEP + dat_file;
         }
      }

      if (log_file.find_first_of("/\\") == string::npos){
         if (prefix[prefix.size() - 1] == FILE_PATH_SEP){
            log_file = prefix + log_file;
         }
         else{
            log_file = prefix + FILE_PATH_SEP + log_file;
         }
      }

   }

   //open log file
   ofstream flog;
   flog.open(log_file.c_str(), ios::out);
   if (!flog.good()){
      cerr << "ERROR: failed to open file '" << log_file << "' for writing." << endl;
      flog << "ERROR: failed to open file '" << log_file << "' for writing." << endl;
      cerr.flush();
      flog.close();
      exit(-1);
   }

   //log the command
   flog << "//INFO: command = \'" << argv[0];
   for (TInt idx = 1; idx < argc; ++idx) flog << ' ' << argv[idx];
   flog << "'" << endl << endl;

   //print the banner on the screen and into the log file
   cout << banner() << endl;
   flog << banner() << endl;

   //output compiler information
   cout << "INFO: Compiled by " << cpp_ver() << endl;
   flog << "//INFO: Compiled by " << cpp_ver() << endl;

#ifdef _OPENMP
   cout << "INFO: OpenMP ver " << _OPENMP << " is used." << endl;
   flog << "//INFO: OpenMP ver " << _OPENMP << " is used." << endl;
#endif

   //print out input and output file information
   cout << "INFO: use parameter file '" << para_file << "'." << endl;
   cout << "INFO: write voltage data to '" << dat_file << "'." << endl;
   cout << "INFO: redirect runing log to file '" << log_file << "'." << endl;
   cout << endl;

   //log output and input file information
   flog << "//INFO: use parameter file '" << para_file << "'." << endl;
   flog << "//INFO: write voltage data to '" << dat_file << "'." << endl;
   flog << "//INFO: redirect runing log to file '" << log_file << "'." << endl;
   flog << endl;

   //Create a simulation object
   Simulation simu;

   //load the parameter values from the paramter file
   simu.load_from_file(para_file);

#ifdef _OPENMP
   TInt Nproc = omp_get_num_procs();//get the number of available processors without hyper-threading.
   if (Nproc > 25) Nproc = 25;
   if (Nproc > simu.thread_num() && simu.thread_num() > 0) Nproc = simu.thread_num();

   omp_set_num_threads(Nproc);//set the number of processors.
   omp_set_nested(0);
   omp_set_dynamic(1);

   cout << "INFO: Program is running on " << omp_get_max_threads() << " threads." << endl << endl;
   flog << "//INFO: Program is running on " << omp_get_max_threads() << " threads." << endl << endl;
#endif

   //log the basic infomation of the simulation
   flog << "//INFO: number of neuron groups = " << simu.ng_num() << endl;
   flog << "//INFO: number of layers = " << simu.layer_num() << endl;
   flog << "//INFO: number of receptor = " << simu.rcpt_num() << endl;
   flog << "//INFO: number of external source = " << simu.exsrc_num() << endl;
   flog << "//INFO: number of stimulator = " << simu.stim_num() << endl;
   flog << "//INFO: number of synaptic connection = " << SynpConn::count() << endl << endl;

   //log the parameter settings
   flog << "//--------------- parameter settings ---------------" << endl;
   flog << simu.get_cfg() << endl;
   flog << "//------------- parameter settings end -------------" << endl << endl;

   //print voltage info on the screen every 1 sec
   TInt print_dt = static_cast<TInt>(1000. / simu.time_step());
   TInt print_step = print_dt;

   //voltage data file
   ofstream fout(dat_file.c_str(), std::ofstream::binary);
   if (!fout.good()){
      cerr << "ERROR: open output file '" << dat_file << "'!" << endl;
      flog << "ERROR: open output file '" << dat_file << "'!" << endl;
      cerr.flush();
      flog.close();
      exit(-1);
   }

   //Voltage data file structure
   //|<--------------->|<--------------------->|<----------------->|
   //    info (1024)     configure (cfg_len)     data (data_len)
   //|<--------------- header ---------------->|

   vector<char> buff; //data buffer

   simu.get_data_header(buff); //get data header

   fout.write(&(buff.front()), buff.size()); //write the header to file

   time(&raw_tm);
   strftime(time_stamp, 31, "%Y-%m-%d %H:%M:%S", localtime(&raw_tm));
   flog << "//INFO: simulation started at " << time_stamp << "." << endl;
   flog.flush();

   double sec_elapsed;

   time(&bgn_tm); //receord the beginning time

   TInt ctr_pnt = simu.elmt_num() / 2 - simu.grid_row() / 2; //cntr of simulated area

   while (simu.evlt_step() != simu.total_step()){

      simu.advance();

      //print out voltage info to the screen regularly
      if (simu.evlt_step() == print_step){
         cout << "time = " << simu.evlt_time() << " sec" << endl;
         for (TInt ineur = 0; ineur < simu.ng_num(); ++ineur){
            cout << "  " << simu.neur_name(ineur) << ":\t" << setw(5) \
               << simu.Volt(ctr_pnt, ineur) << endl;
         }

         time(&raw_tm);
         sec_elapsed = difftime(raw_tm, bgn_tm);
         cout << endl << sec2str(sec_elapsed) << " has elapsed, " \
            << sec2str(sec_elapsed * (simu.total_time() - simu.evlt_time())\
            / simu.evlt_time()) << " to finish." << endl << endl;

         print_step += print_dt;
         cout.flush();
      }

      //write voltage info to the output file
      if (simu.is_out()){ //the state of is_out is updated in simu.advance()
         buff.clear();
         //get voltage info
         simu.get_data_block(buff);
         //write voltage info
         fout.write(&(buff.front()), buff.size());
      }
   }

   fout.close(); //close the file

   time(&raw_tm);
   sec_elapsed = difftime(raw_tm, bgn_tm); //calculate elapsed time
   strftime(time_stamp, 31, "%Y-%m-%d %H:%M:%S", localtime(&raw_tm));

   cout << endl << "INFO: total running time = " << sec2str(sec_elapsed) << "." << endl;
   flog << "//INFO: simulation finished at " << time_stamp << "." << endl;
   flog << "//INFO: total running time = " << sec2str(sec_elapsed) << "." << endl;

   flog.close();
   //char ch;
   //cin>>ch;
}
