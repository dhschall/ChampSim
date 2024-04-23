/*
 * This file implements a basic Branch Target Buffer (BTB) structure, a Return Address Stack (RAS), and an indirect target branch prediction.
 */

#include <iostream>

#include "ooo_cpu.h"
#include "../basic_btb.h"
#include "../ittage_64KB.h"
#include "../ras.h"

#include <fmt/core.h>

//BasicBTB<8192, 8> btb[NUM_CPUS];
BasicBTB<2048, 8> btb[NUM_CPUS];
//BasicBTB<256, 4> l1_btb[NUM_CPUS];
//BasicBTB<4096, 8> l2_btb[NUM_CPUS];
my_predictor *ittage[NUM_CPUS];
RAS<64, 4096> ras[NUM_CPUS];

std::pair<uint64_t, uint8_t> interim_result;
std::array<uint64_t, 8> btb_misses;

void O3_CPU::initialize_btb()
{
  std::cout << "BTB:" << std::endl;
  btb[cpu].initialize();
  std::cout << "Indirect:" << std::endl;
  ittage[cpu] = new my_predictor();
  std::cout << "RAS:" << std::endl;
  ras[cpu].initialize();
}

std::pair<uint64_t, uint8_t> O3_CPU::btb_prediction(uint64_t ip)
{
  auto btb_pred = btb[cpu].predict(ip);
  interim_result = std::make_pair(0, false);
  if (btb_pred.first == 0 && btb_pred.second == BRANCH_INFO_ALWAYS_TAKEN) {
    // no prediction for this IP
  }
  else if (btb_pred.second == BRANCH_INFO_INDIRECT) {
    interim_result = std::make_pair(ittage[cpu]->predict_brindirect(ip), true);
  } else if (btb_pred.second == BRANCH_INFO_RETURN) {
    interim_result = std::make_pair(ras[cpu].predict(), true);
  } else {
    interim_result = std::make_pair(btb_pred.first, btb_pred.second != BRANCH_INFO_CONDITIONAL);
  }
  return interim_result;
}

void O3_CPU::update_btb(uint64_t ip, uint64_t branch_target, uint8_t taken, uint8_t branch_type)
{
  ittage[cpu]->update_brindirect(ip, branch_type, taken, branch_target);
  ittage[cpu]->fetch_history_update(ip, branch_type, taken, branch_target);
  ras[cpu].update(ip, branch_target, taken, branch_type);
  btb[cpu].update(ip, branch_target, taken, branch_type);

  if (branch_target != interim_result.first) {
    btb_misses[branch_type]++;
  }

}


void O3_CPU::btb_final_stats(int inst)
{

constexpr std::array<std::pair<std::string_view, std::size_t>, 6> types{
      {std::pair{"BRANCH_DIRECT_JUMP", BRANCH_DIRECT_JUMP}, std::pair{"BRANCH_INDIRECT", BRANCH_INDIRECT},
       std::pair{"BRANCH_CONDITIONAL", BRANCH_CONDITIONAL},
       std::pair{"BRANCH_DIRECT_CALL", BRANCH_DIRECT_CALL}, std::pair{"BRANCH_INDIRECT_CALL", BRANCH_INDIRECT_CALL},
       std::pair{"BRANCH_RETURN", BRANCH_RETURN}}};


    // brpred->PrintStat(inst);
    // delete brpred;
    // printf("ZZZ Num_unique_taken_branches %lu\n", takenPCs.size());


    for (auto [str, idx] : types) {
      fmt::print("ZZZ {}_BTB_MISS {}\n", str, btb_misses[idx]);
      fmt::print("ZZZ {}_BTB_MPKI {:.3}\n", str, btb_misses[idx] * 1000.0 / (double)inst);
    }

}