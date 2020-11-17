#ifndef BRANCH_TRACING_H
#define BRANCH_TRACING_H


#include <unordered_map>
#include "json/json.h"

struct branch_track_t {
  // count how often the branch was executed
  uint64_t executions;
  // count how often the branch is taken
  uint64_t taken;
  // count the number of miss predictions
  uint64_t miss_pred;
  // branch type
  uint8_t branch_type;
  // branch target
  uint64_t branch_target;
  //
  uint8_t taken_last_time;

  struct tage_info_t {
    uint64_t tage_miss, sc_miss, loop_miss;
    uint64_t tage_pred, sc_pred, loop_pred;
    uint64_t n_alloc, n_useful_entries;
    uint64_t n_entries_alloc, hit_bank;
    uint64_t utilization, max_util; 

    tage_info_t()
      : tage_miss{0}, sc_miss{0}, loop_miss{0},
      tage_pred{0}, sc_pred{0}, loop_pred{0},
      n_alloc{0}, n_useful_entries{0},
      n_entries_alloc{0}, hit_bank{0},
      utilization{0}, max_util{0}
    {}
  } tage_info;

  std::string print() {
    // const char *fmt = "N_exe: %i; N_mispred: %i; br. typ: %i; br. target: %#x";
    // std::string s = str( format("%2% %2% %1%\n") % "world" % "hello" );
    return "N_exe: " + to_string(executions) + "; N_mispred: " + to_string(miss_pred) + "; br. typ: " + to_string(branch_type) + "; br. target: " + to_string(branch_target);
  }

  Json::Value dumpJSON() {
    Json::Value val;
    val["type"] = branch_type;
    val["target"] = branch_target;
    val["N_EXE"] = executions;
    val["N_TAKEN"] = taken;
    val["N_MISS"] = miss_pred;

    Json::Value tage_val;
    tage_val["tage_miss"] = tage_info.tage_miss;
    tage_val["sc_miss"] = tage_info.sc_miss;
    tage_val["loop_miss"] = tage_info.loop_miss;

    tage_val["tage_pred"] = tage_info.tage_pred;
    tage_val["sc_pred"] = tage_info.sc_pred;
    tage_val["loop_pred"] = tage_info.loop_pred;
 
    tage_val["utilization"] = tage_info.utilization;
    tage_val["max_util"] = tage_info.max_util;
   
    tage_val["n_alloc"] = tage_info.n_alloc;
    tage_val["n_useful_entries"] = tage_info.n_useful_entries;
    tage_val["n_entries_alloc"] = tage_info.n_entries_alloc;
    tage_val["hit_bank"] = tage_info.hit_bank;
    val["tage_info"] = tage_val;
    return val;
  }
};


#endif // BRANCH_TRACING_H
