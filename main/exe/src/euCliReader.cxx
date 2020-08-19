#include "eudaq/OptionParser.hh"
#include "eudaq/FileReader.hh"
#include "eudaq/StdEventConverter.hh"
#include "eudaq/Event.hh"

#include <iostream>
#include <fstream> 

#include <iostream>

int main(int /*argc*/, const char **argv) {
  std::ofstream outfile_BCID ("BCID.txt");
  std::ofstream outfile_TTC ("TTC.txt");
  std::ofstream outfile_event ("event_numbers.txt");
  std::ofstream outfile_event_skip ("event_skip.txt");

  //  std::ofstream outfile_L0ID ("L0ID.txt"); //not currently needed for desync
  //  std::ofstream outfile_L0ID_ev ("L0ID_ev.txt"); //not currently needed for desync 

  eudaq::OptionParser op("EUDAQ Command Line FileReader modified for TLU", "2.1", "EUDAQ FileReader (TLU)");
  eudaq::Option<std::string> file_input(op, "i", "input", "", "string", "input file");
  eudaq::Option<uint32_t> eventl(op, "e", "event", 0, "uint32_t", "event number low");
  eudaq::Option<uint32_t> eventh(op, "E", "eventhigh", 0, "uint32_t", "event number high");
  eudaq::Option<uint32_t> triggerl(op, "tg", "trigger", 0, "uint32_t", "trigger number low");
  eudaq::Option<uint32_t> triggerh(op, "TG", "triggerhigh", 0, "uint32_t", "trigger number high");
  eudaq::Option<uint32_t> timestampl(op, "ts", "timestamp", 0, "uint32_t", "timestamp low");
  eudaq::Option<uint32_t> timestamph(op, "TS", "timestamphigh", 0, "uint32_t", "timestamp high");
  eudaq::OptionFlag stat(op, "s", "statistics", "enable print of statistics");
  eudaq::OptionFlag stdev(op, "std", "stdevent", "enable converter of StdEvent");

  op.Parse(argv);

  std::string infile_path = file_input.Value();
  std::string type_in = infile_path.substr(infile_path.find_last_of(".")+1);
  if(type_in=="raw")
    type_in = "native";

  bool stdev_v = stdev.Value();


  uint32_t eventl_v = eventl.Value();
  uint32_t eventh_v = eventh.Value();
  uint32_t triggerl_v = triggerl.Value();
  uint32_t triggerh_v = triggerh.Value();
  uint32_t timestampl_v = timestampl.Value();
  uint32_t timestamph_v = timestamph.Value();
  bool not_all_zero = eventl_v||eventh_v||triggerl_v||triggerh_v||timestampl_v||timestamph_v;

  eudaq::FileReaderUP reader;
  reader = eudaq::Factory<eudaq::FileReader>::MakeUnique(eudaq::str2hash(type_in), infile_path);
  
  //  int yes_L0ID=0; //not currently needed for desync
  //  int no_L0ID=0;  //not currently needed for desync


  while(1){
    auto ev = reader->GetNextEvent();
    if(!ev)
      break;
    bool in_range_evn = false;
    if(eventl_v!=0 || eventh_v!=0){
      uint32_t ev_n = ev->GetEventN();
      if(ev_n >= eventl_v && ev_n < eventh_v){
        in_range_evn = true;
      }
    }
    else
      in_range_evn = true;

    bool in_range_tgn = false;
    if(triggerl_v!=0 || triggerh_v!=0){
      uint32_t tg_n = ev->GetTriggerN();
      if(tg_n >= triggerl_v && tg_n < triggerh_v){
        in_range_tgn = true;
      }
    }
    else
      in_range_tgn = true;

    bool in_range_tsn = false;
    if(timestampl_v!=0 || timestamph_v!=0){
      uint32_t ts_beg = ev->GetTimestampBegin();
      uint32_t ts_end = ev->GetTimestampEnd();
      if(ts_beg >= timestampl_v && ts_end <= timestamph_v){
        in_range_tsn = true;
      }
    }
    else
      in_range_tsn = true;
  
    //Desync edits start here
    //    if((in_range_evn && in_range_tgn && in_range_tsn) && not_all_zero){
    //
    //      if(stdev_v){

    auto evstd = eudaq::StandardEvent::MakeShared();
    eudaq::StdEventConverter::Convert(ev, evstd, nullptr);

    uint32_t ev_n = ev->GetEventN();
    int BCID  = 100;
    int TTCBCID =100;
    //    int L0ID =-1; //not currently needed for desync 

    BCID = evstd->GetTag("RAWBCID", BCID); //ABCStar   
    TTCBCID = evstd->GetTag("TTC.BCID", TTCBCID); //ITSDAQ
    //    L0ID = evstd->GetTag("RAWL0ID", L0ID); //ABCSTAR //not currently needed for desycn 
    //    std::cout << L0ID << std::endl;
    
    //if there is both a RAWBCID and TTCBCID for an event then keep the event otherwise ignore the event 
    if(BCID<100 && TTCBCID<100){
      outfile_BCID << BCID << std::endl;
      outfile_TTC << TTCBCID << std::endl;
      outfile_event << ev_n << std::endl;    
    }      


    /*
    if(L0ID!=-1){      
      outfile_L0ID << L0ID <<std::endl;
      outfile_L0ID_ev << ev_n <<std::endl;
    }
    */
    
    //txt file with the events that have been skipped, good for debugging 
    if(BCID>80 || TTCBCID>80){
      outfile_event_skip << ev_n << std::endl;
    }
    
  
    
   //    event_count ++;
  }
  //  outfile1.close();
  outfile_BCID.close();
  outfile_TTC.close();
  outfile_event.close();
  outfile_event_skip.close();


  //  outfile_L0ID.close();

    





  return 0;
}


