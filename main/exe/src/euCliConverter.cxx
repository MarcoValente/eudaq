//draft by Yi LIU, Hamburg, 06-12-2019, no warranty
//Desync additions by Emma Buchanan 11/02/2020

#include "eudaq/OptionParser.hh"
#include "eudaq/DataConverter.hh"
#include "eudaq/FileWriter.hh"
#include "eudaq/FileReader.hh"
#include "eudaq/StandardEvent.hh"
#include "eudaq/Event.hh"
#include "eudaq/StdEventConverter.hh"
#include <iostream>
#include <deque>
//#include "TEST.hh"
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

//================================================
//git clone https://github.com/eudaq/eudaq
//cp euCliConverter.cc eudaq/main/exe/src
//mkdir eudaq/build
//cmake .. -DUSER_TLU_BUILD=ON -DUSER_EUDET_BUILD=ON -DUSER_ITKSTRIP_BUILD=ON -DUSER_STCONTROL_BUILD=ON
//make install
//
//=================================================


int main(int /*argc*/, const char **argv) {
  std::vector<int> BCID;
  std::vector<int> TTC;
  std::vector<int> Shift;
  cout << "---------Running parray.cxx to get the lookup table"  << endl;
  std::system("./p.out"); //run parray.cxx stand alone code to create the lookup table 
  cout << "---------lookup table created" << endl;
  //--------read in txt file loop up tables 
  //reading in the RAWBCID
  ifstream infile1;
  infile1.open("txt_BCID_events.txt"); //needs to be changed to your local directory 
  if (!infile1) {
    cout << "Unable to open file: txt_BCID_events.txt ";
    exit(1); // terminate with error
  }
  if (infile1) {
    cout << "reading in txt_BCID_events.txt" << endl;
    double value;
    while ( infile1 >> value ) {
      BCID.push_back(value);
    }
  }
  infile1.close();
  
  //reading in the TTCBCID                                                                                         
  ifstream infile2;
  infile2.open("txt_TTC_events.txt"); //needs to be charged to your local directory 
  if (!infile2) {
    cout << "Unable to open file: txt_TTC_events.txt ";
    exit(1); // terminate with error
  }
  if (infile2) {
    cout << "reading in txt_TTC_events.txt" << endl;
    double value;
    while ( infile2 >> value ) {
      TTC.push_back(value);
    }
  }
  infile2.close();
  

  //make sure that the TTC and BCID vectors are the same length 
  if(TTC.size() == BCID.size()){
    std::cout << "TTC and BCID txt file length match" << std::endl;
    std::cout << TTC.size() << "\t" << BCID.size() <<std::endl;
  }


  eudaq::OptionParser op("EUDAQ2 command line", "0.0001", "A resync tool");
  eudaq::Option<std::string> file_input(op, "i", "input", "", "string", "input file (eg. /home/testbeam/run000001.raw)");
  eudaq::Option<std::string> file_output(op, "o", "output", "", "string", "output file (eg. /home/testbeam/sync000001.raw)");
  eudaq::OptionFlag iprint(op, "ip", "iprint", "enable print of input Event");
  eudaq::OptionFlag oprint(op, "op", "iprint", "enable print of output Event");
  try{
    op.Parse(argv);
  }
  catch (...) {
    return op.HandleMainException();
  }

  std::string infile_path = file_input.Value();
  std::string outfile_path = file_output.Value();
  bool flag_print_input  = iprint.Value();
  bool flag_print_output  = oprint.Value();
  if(infile_path.empty() || infile_path.empty()){
    std::cerr<<"option --help to get help"<<std::endl;
    return 1;
  }

  std::string type_out = outfile_path.substr(outfile_path.find_last_of(".")+1);
  if(type_out=="raw")  type_out = "native";

  auto reader = eudaq::Factory<eudaq::FileReader>::MakeUnique(eudaq::str2hash("native"), infile_path);
  auto writer = eudaq::Factory<eudaq::FileWriter>::MakeUnique(eudaq::str2hash(type_out), outfile_path);


  //Identifying the name of each sub event information stored within an Event 
  static const std::string dsp_abc("ITS_ABC");
  static const std::string dsp_ttc("ITS_TTC");
  static const std::string dsp_tel("NiRawDataEvent");
  static const std::string dsp_tlu("TluRawDataEvent");
  static const std::string dsp_ref("USBPIXI4"); //or USBPIXI4 ?


  //creating a vector queue that will store in a queue each of the subevent information 
 
  std::deque<eudaq::EventSPC> que_abc;
  std::deque<eudaq::EventSPC> que_ttc;
  std::deque<eudaq::EventSPC> que_tel;
  std::deque<eudaq::EventSPC> que_tlu;
  std::deque<eudaq::EventSPC> que_ref;

  std::deque<int> TTC_events;
  std::deque<int> BCID_events;
  cout << "creating the new synced events " << endl;

  int n_events = 0;
  while(1){
    eudaq::EventSPC ev_in = reader->GetNextEvent();
    
    if(!ev_in){
      std::cout<< "No more Event. End of file."<<std::endl;
      break;
    }
    if(flag_print_input){
      ev_in->Print(std::cout);
    }
    uint32_t ev_n = ev_in->GetEventN();

    bool BCID_bool;

    std::vector<int>::iterator BCID_it;
    BCID_it = std::find(BCID.begin(), BCID.end(), ev_n);
    if (BCID_it != BCID.end()){
      BCID_bool = true; //event found
    }
    else{
      BCID_bool = false;
    }

    bool TTC_bool;
    std::vector<int>::iterator TTC_it;
    TTC_it = std::find(TTC.begin(), TTC.end(), ev_n);
    if (TTC_it != TTC.end()){
      TTC_bool = true;
    }
    else{
      TTC_bool = false;
    }
    
    
    //assigning the sub events to eacch of the queues depending on the event description 
    std::vector<eudaq::EventSPC> subev_col = ev_in->GetSubEvents();
    
    for(auto &subev : subev_col ){
      std::string dsp = subev->GetDescription();
      if((dsp==dsp_ttc) && TTC_bool) { que_ttc.push_back(subev); continue; }
      if((dsp==dsp_tel) && TTC_bool) { que_tel.push_back(subev); continue; }
      if((dsp==dsp_tlu) && TTC_bool) { que_tlu.push_back(subev); continue; }
      if((dsp==dsp_ref) && TTC_bool) { que_ref.push_back(subev); continue; }
      if((dsp==dsp_abc) && BCID_bool){ que_abc.push_back(subev); continue; }
      
    }

    //if each queue has at least one Event, go to this loop.
    if(que_abc.size()> 0 && que_ttc.size() >0){
      auto ev_front_abc = que_abc.front();
      auto ev_front_ttc = que_ttc.front();
      auto ev_front_tel = que_tel.front();
      auto ev_front_tlu = que_tlu.front();
      auto ev_front_ref = que_ref.front();
      
      auto ev_sync =  eudaq::Event::MakeUnique("syncEvent");
      ev_sync->SetFlagPacket();
      ev_sync->AddSubEvent(ev_front_abc); 
      ev_sync->AddSubEvent(ev_front_ttc);
      ev_sync->AddSubEvent(ev_front_tel);    
      ev_sync->AddSubEvent(ev_front_tlu);
      ev_sync->AddSubEvent(ev_front_ref);      
      ev_sync->SetTriggerN(ev_front_tlu->GetTriggerN());
      //      ev_sync->SetEventN(ev_n); //use this if you want to use the original event number  
      ev_sync->SetEventN(n_events); //this assigned a new event number to the events


      que_abc.pop_front();
      que_ttc.pop_front();
      que_tel.pop_front();
      que_tlu.pop_front();
      que_ref.pop_front();

      if(writer){      
	writer->WriteEvent(std::move(ev_sync));
      }
      n_events++;
    }
    
    
  }
  
  //std::system("mv run000*.slcio /afs/cern.ch/work/e/ebuchana/EU_DATA/DESYNC_CORRECTED"); //move the slcio file to desired directory 
  

  return 0;
}


