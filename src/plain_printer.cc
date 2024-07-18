/*
 *    Copyright 2023 The ChampSim Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <numeric>
#include <sstream>
#include <utility>
#include <vector>

#include "stats_printer.h"
#include <fmt/core.h>
#include <fmt/ostream.h>

void champsim::plain_printer::print(O3_CPU::stats_type stats)
{
  constexpr std::array<std::pair<std::string_view, std::size_t>, 8> types{
      {std::pair{"BRANCH_DIRECT_JUMP", BRANCH_DIRECT_JUMP}, std::pair{"BRANCH_INDIRECT", BRANCH_INDIRECT}, std::pair{"BRANCH_CONDITIONAL", BRANCH_CONDITIONAL},
       std::pair{"BRANCH_DIRECT_CALL", BRANCH_DIRECT_CALL}, std::pair{"BRANCH_INDIRECT_CALL", BRANCH_INDIRECT_CALL},
       std::pair{"BRANCH_RETURN", BRANCH_RETURN}, std::pair{"BRANCH_OTHER", BRANCH_OTHER}, std::pair{"NOT_BRANCH", NOT_BRANCH}}};

  auto total_branch = std::ceil(
      std::accumulate(std::begin(types), std::end(types), 0ll, [tbt = stats.total_branch_types](auto acc, auto next) { return acc + tbt[next.second]; }));
  auto total_mispredictions = std::ceil(
      std::accumulate(std::begin(types), std::end(types), 0ll, [btm = stats.branch_type_misses](auto acc, auto next) { return acc + btm[next.second]; }));

  fmt::print(stream, "\n{} cumulative IPC: {:.4g} instructions: {} cycles: {}\n", stats.name, std::ceil(stats.instrs()) / std::ceil(stats.cycles()),
             stats.instrs(), stats.cycles());
  fmt::print(stream, "ZZZ IPC: {:.5g}\n", std::ceil(stats.instrs()) / std::ceil(stats.cycles()));
  fmt::print(stream, "ZZZ instructions: {}\n", stats.instrs());
  fmt::print(stream, "ZZZ cycles: {}\n", stats.cycles());


  fmt::print(stream, "{} Branch Prediction Accuracy: {:.4g}% MPKI: {:.4g} Average ROB Occupancy at Mispredict: {:.4g}\n", stats.name,
             (100.0 * std::ceil(total_branch - total_mispredictions)) / total_branch, (1000.0 * total_mispredictions) / std::ceil(stats.instrs()),
             std::ceil(stats.total_rob_occupancy_at_branch_mispredict) / total_mispredictions);

  std::vector<double> mpkis;
  std::transform(std::begin(stats.branch_type_misses), std::end(stats.branch_type_misses), std::back_inserter(mpkis),
                 [instrs = stats.instrs()](auto x) { return 1000.0 * std::ceil(x) / std::ceil(instrs); });

  fmt::print(stream, "Branch type MPKI\n");
  for (auto [str, idx] : types)
    fmt::print(stream, "{}: {:.3}\n", str, mpkis[idx]);
  fmt::print(stream, "\n");

  uint64_t total_branches = 0;
  uint64_t total_misp = 0;
  for (const auto& [str, idx] : types) {
    fmt::print(stream, "{} Branch Type: {}\n", stats.name, str);
    fmt::print(stream, "ZZZ {} {}\n", str, stats.total_branch_types[idx]);
    fmt::print(stream, "ZZZ {}_MISP {}\n", str, stats.branch_type_misses[idx]);
    fmt::print(stream, "ZZZ {}_MPKI {:.4g}\n", str, mpkis[idx]);
    total_branches += stats.total_branch_types[idx];
    total_misp += stats.branch_type_misses[idx];
  }

  fmt::print(stream, "ZZZ target_misses {}\n", stats.target_misses);
  fmt::print(stream, "ZZZ cond_branch_misses {}\n", stats.cond_branch_misses);

  fmt::print(stream, "Total:\n");
  fmt::print(stream, "ZZZ TOTAL_BR_EXEC {}\n", total_branches);
  fmt::print(stream, "ZZZ TOTAL_BR_MISP {}\n", total_misp);

  fmt::print(stream, "ZZZ IFETCH_BUFF_OCC {:.4g}\n", std::ceil(stats.total_ifetch_buffer_occupancy) / stats.cycles());
  fmt::print(stream, "ZZZ DECODE_BUFF_OCC {:.4g}\n", std::ceil(stats.total_decode_buffer_occupancy) / stats.cycles());
  fmt::print(stream, "ZZZ DISPATCH_BUFF_OCC {:.4g}\n", std::ceil(stats.total_dispatch_buffer_occupancy) / stats.cycles());
  fmt::print(stream, "ZZZ L1I_B_OCC {:.4g}\n", std::ceil(stats.total_l1ib_occupancy) / stats.cycles());

  fmt::print(stream, "ZZZ RETIRED {:.4g}, CLK {:.4g}, RETIRED_CLK {:.4g}\n",
                      std::ceil(stats.total_retired) / stats.cycles(),
                      std::ceil(stats.cycle_retired) / stats.cycles(),
                      std::ceil(stats.total_retired) / std::ceil(stats.cycle_retired));
  fmt::print(stream, "ZZZ EXECUTED {:.4g}, CLK {:.4g}, EXECUTED_CLK {:.4g}\n",
                      std::ceil(stats.total_executed) / stats.cycles(),
                      std::ceil(stats.cycle_executed) / stats.cycles(),
                      std::ceil(stats.total_executed) / std::ceil(stats.cycle_executed));
  fmt::print(stream, "ZZZ SCHEDULED {:.4g}, CLK {:.4g}, SCHEDULED_CLK {:.4g}\n",
                      std::ceil(stats.total_scheduled) / stats.cycles(),
                      std::ceil(stats.cycle_scheduled) / stats.cycles(),
                      std::ceil(stats.total_scheduled) / std::ceil(stats.cycle_scheduled));
  fmt::print(stream, "ZZZ DISPATCHED {:.4g}, CLK {:.4g}, DISPATCHED_CLK {:.4g}\n",
                      std::ceil(stats.total_dispatched) / stats.cycles(),
                      std::ceil(stats.cycle_dispatched) / stats.cycles(),
                      std::ceil(stats.total_dispatched) / std::ceil(stats.cycle_dispatched));
  fmt::print(stream, "ZZZ DECODED {:.4g}, CLK {:.4g}, DECODED_CLK {:.4g}\n",
                      std::ceil(stats.total_decoded) / stats.cycles(),
                      std::ceil(stats.cycle_decoded) / stats.cycles(),
                      std::ceil(stats.total_decoded) / std::ceil(stats.cycle_decoded));
  fmt::print(stream, "ZZZ FETCHED {:.4g}, CLK {:.4g}, FETCHED_CLK {:.4g}\n",
                      std::ceil(stats.total_fetched) / stats.cycles(),
                      std::ceil(stats.cycle_fetched) / stats.cycles(),
                      std::ceil(stats.total_fetched) / std::ceil(stats.cycle_fetched));
  fmt::print(stream, "ZZZ INST_FETCHED {:.4g}, INST_PER_FETCH {:.4g} INST_FETCHED_CLK {:.4g}\n",
                      std::ceil(stats.total_inst_per_fetch) / stats.cycles(),
                      std::ceil(stats.total_inst_per_fetch) / std::ceil(stats.total_fetched),
                      std::ceil(stats.total_inst_per_fetch) / std::ceil(stats.cycle_fetched));

  fmt::print(stream, "ZZZ PROMOTED {:.4g}, CLK {:.4g}, PROMOTED_CLK {:.4g}\n",
                      std::ceil(stats.total_promoted) / stats.cycles(),
                      std::ceil(stats.cycle_promoted) / stats.cycles(),
                      std::ceil(stats.total_promoted) / std::ceil(stats.cycle_promoted));
  fmt::print(stream, "ZZZ IREAD {:.4g}\n", std::ceil(stats.total_iread) / stats.cycles());
  fmt::print(stream, "ZZZ DREAD {:.4g}\n", std::ceil(stats.total_dread) / stats.cycles());

  fmt::print(stream, "ZZZ STALL_RETIRE {:.4g}\n", std::ceil(stats.stall_retire) / stats.cycles());
  fmt::print(stream, "ZZZ STALL_DISPATCH {:.4g}\n", std::ceil(stats.stall_dispatch) / stats.cycles());
  fmt::print(stream, "ZZZ STALL_DECODE {:.4g}\n", std::ceil(stats.stall_decode) / stats.cycles());
  fmt::print(stream, "ZZZ STALL_FETCH {:.4g}\n", std::ceil(stats.stall_fetch) / stats.cycles());
  fmt::print(stream, "ZZZ STALL_L1I {:.4g}\n", std::ceil(stats.stall_l1i) / stats.cycles());
  fmt::print(stream, "ZZZ STALL_L1I_HEAD {:.4g}\n", std::ceil(stats.stall_l1i_head) / stats.cycles());
  fmt::print(stream, "ZZZ STALL_L1I_HEAD_LAT {:.4g}\n", std::ceil(stats.total_icache_stall_latency) / std::ceil(stats.total_icache_stall_count));
  fmt::print(stream, "ZZZ IFB_INFLIGHT {:.4g}\n", std::ceil(stats.total_ifb_inflight) / stats.cycles());
  fmt::print(stream, "ZZZ IFB_NEED_FETCH {:.4g}\n", std::ceil(stats.total_ifb_need_fetch) / stats.cycles());
  fmt::print(stream, "ZZZ IFB_FETCHED {:.4g}\n", std::ceil(stats.total_ifb_fetched) / stats.cycles());
  fmt::print(stream, "ZZZ IFB_FETCHED_HEAD {:.4g}\n", std::ceil(stats.total_ifb_fetched_head) / stats.cycles());
  fmt::print(stream, "ZZZ IFB_CHECKED {:.4g}\n", std::ceil(stats.total_ifb_checked) / stats.cycles());
  fmt::print(stream, "ZZZ INST_PRED {:.4g}, INST_PRED_CLK {:.4g}, INST_STOP_PRED {:.4g}, INST_STOP_PRED_CLK {:.4g}\n",
                      std::ceil(stats.total_inst_fetched) / std::ceil(stats.total_inst_fetched_cycle),
                      std::ceil(stats.total_inst_fetched_cycle) / stats.cycles(),
                      std::ceil(stats.total_inst_fetched_stop_br) / std::ceil(stats.total_inst_fetched_stop_br_cycle),
                      std::ceil(stats.total_inst_fetched_stop_br_cycle) / std::ceil(stats.total_inst_fetched_cycle)
                      );
    fmt::print(stream, "ZZZ INST_FE_CLK {:.4g}, STOP: [BR {:.4g}, FULL {:.4g}, STALL {:.4g}], STOP_EMPTY {:.4g}, STOP_FULL {:.4g}, STOP_STALL {:.4g}, \n",
                      std::ceil(stats.total_inst_fetched_cycle) / stats.cycles(),
                      std::ceil(stats.total_inst_fetched_stop_br_cycle) / std::ceil(stats.total_inst_fetched_cycle),
                      std::ceil(stats.total_inst_fetched_full2_cycle) / std::ceil(stats.total_inst_fetched_cycle),
                      std::ceil(stats.total_inst_fetched_stall2_cycle) / std::ceil(stats.total_inst_fetched_cycle),
                      std::ceil(stats.total_inst_fetched_stop_ie_cycle) / stats.cycles(),
                      std::ceil(stats.total_inst_fetched_full_cycle) / stats.cycles(),
                      std::ceil(stats.total_inst_fetched_stall_cycle) / stats.cycles()
                      );

  fmt::print(stream, "ROB Occup\n{}", stats.rob_occupancy_hist.print(true,true));
  fmt::print(stream, "Reg dep\n{}", stats.inst_dep_hist.print(true,true));
  fmt::print(stream, "Retire\n{}", stats.ret_inst.print(true,true));
  fmt::print(stream, "Complete\n{}", stats.complete_inst.print(true,true));
  fmt::print(stream, "Exec\n{}", stats.exec_inst.print(true,true));
  fmt::print(stream, "Sched\n{}", stats.sched_inst.print(true,true));
  fmt::print(stream, "LSQ\n{}", stats.lsq_inst.print(true,true));
  fmt::print(stream, "Disp\n{}", stats.disp_inst.print(true,true));
  fmt::print(stream, "Dec\n{}", stats.dec_inst.print(true,true));
  fmt::print(stream, "Fetch\n{}", stats.fetch_inst.print(true,true));
  fmt::print(stream, "LQ\n{}", stats.lq_if.print(true,true));
  fmt::print(stream, "SQ\n{}", stats.lq_if.print(true,true));

}

void champsim::plain_printer::print(CACHE::stats_type stats, int inst)
{
  constexpr std::array<std::pair<std::string_view, std::size_t>, 5> types{
      {std::pair{"LOAD", champsim::to_underlying(access_type::LOAD)}, std::pair{"RFO", champsim::to_underlying(access_type::RFO)},
       std::pair{"PREFETCH", champsim::to_underlying(access_type::PREFETCH)}, std::pair{"WRITE", champsim::to_underlying(access_type::WRITE)},
       std::pair{"TRANSLATION", champsim::to_underlying(access_type::TRANSLATION)}}};

  for (std::size_t cpu = 0; cpu < NUM_CPUS; ++cpu) {
    uint64_t TOTAL_HIT = 0, TOTAL_MISS = 0;
    for (const auto& type : types) {
      TOTAL_HIT += stats.hits.at(type.second).at(cpu);
      TOTAL_MISS += stats.misses.at(type.second).at(cpu);
    }

    fmt::print(stream, "{} TOTAL        ACCESS: {:10d} HIT: {:10d} MISS: {:10d} MPKI: {:.4g}\n", stats.name, TOTAL_HIT + TOTAL_MISS, TOTAL_HIT, TOTAL_MISS,
               (1000.0 * TOTAL_MISS) / inst);
    for (const auto& type : types) {
      fmt::print(stream, "{} {:<12s} ACCESS: {:10d} HIT: {:10d} MISS: {:10d} MPKI: {:.4g}\n", stats.name, type.first,
                 stats.hits[type.second][cpu] + stats.misses[type.second][cpu], stats.hits[type.second][cpu], stats.misses[type.second][cpu],
                 (1000.0 * stats.misses[type.second][cpu]) / inst);
    }

    fmt::print(stream, "{} PREFETCH REQUESTED: {:10} ISSUED: {:10} USEFUL: {:10} USELESS: {:10}\n", stats.name, stats.pf_requested, stats.pf_issued,
               stats.pf_useful, stats.pf_useless);

    fmt::print(stream, "{} AVERAGE MISS LATENCY: {:.4g} cycles\n", stats.name, stats.avg_miss_latency);

    fmt::print(stream, "ZZZ {}_TOTAL_MISS {}\n", stats.name, TOTAL_MISS);
    fmt::print(stream, "ZZZ {}_LOAD_MISS {}\n", stats.name, stats.misses.at(champsim::to_underlying(access_type::LOAD)).at(cpu));
    // fmt::print(stream, "ZZZ {}_LOAD_MPKI {}\n", stats.name, TOTAL_MISS);

  }
}

void champsim::plain_printer::print(champsim::cache_queue_stats stats)
{
  fmt::print(stream, "{} RQ: ACCESS: {:10} FULL: {:10} MERGED: {:10}\n", stats.name, stats.RQ_ACCESS, stats.RQ_FULL, stats.RQ_MERGED);
  fmt::print(stream, "{} WQ: ACCESS: {:10} FULL: {:10} MERGED: {:10}\n", stats.name, stats.WQ_ACCESS, stats.WQ_FULL, stats.WQ_MERGED);
  fmt::print(stream, "{} PQ: ACCESS: {:10} FULL: {:10} MERGED: {:10}\n", stats.name, stats.PQ_ACCESS, stats.PQ_FULL, stats.PQ_MERGED);
}


void champsim::plain_printer::print(DRAM_CHANNEL::stats_type stats)
{
  fmt::print(stream, "{}\n", stats.name);
  fmt::print(stream, "RQ: ROW_BUFFER_HIT: {:10}\n    ROW_BUFFER_MISS: {:10}\n    FULL: {:10}\n", stats.RQ_ROW_BUFFER_HIT, stats.RQ_ROW_BUFFER_MISS, stats.RQ_FULL);
  if (stats.dbus_count_congested > 0)
    fmt::print(stream, "     DBUS CONGESTED: N:{} AVG CYCLEs: {:.4g}\n", stats.dbus_cycle_congested, std::ceil(stats.dbus_cycle_congested) / std::ceil(stats.dbus_count_congested));
  else
    fmt::print(stream, "     DBUS CONGESTED: N:0 AVG CYCLEs: -\n");
  fmt::print(stream, "WQ: ROW_BUFFER_HIT: {:10}\n    ROW_BUFFER_MISS: {:10}\n    FULL: {:10}\n", stats.WQ_ROW_BUFFER_HIT, stats.WQ_ROW_BUFFER_MISS, stats.WQ_FULL);
}

void champsim::plain_printer::print(champsim::phase_stats& stats)
{
  fmt::print(stream, "=== {} ===\n", stats.name);

  int i = 0;
  for (auto tn : stats.trace_names)
    fmt::print(stream, "CPU {} runs {}", i++, tn);

  if (NUM_CPUS > 1) {
    fmt::print(stream, "\nTotal Simulation Statistics (not including warmup)\n");

    for (const auto& stat : stats.sim_cpu_stats)
      print(stat);

    for (const auto& stat : stats.sim_cache_stats)
      print(stat);
  }

  fmt::print(stream, "\nRegion of Interest Statistics\n");

  for (uint j = 0; j < stats.roi_cpu_stats.size(); ++j) {
    print(stats.roi_cpu_stats[j]);
  }

  for (uint j = 0; j < stats.roi_cache_stats.size(); ++j) {
    print(stats.roi_cache_stats[j], stats.roi_cpu_stats[0].instrs());
  }

  fmt::print(stream, "\nDRAM Statistics\n");
  for (const auto& stat : stats.roi_dram_stats)
    print(stat);

  fmt::print(stream, "\nQueue Statistics\n");
  for (const auto& stat : stats.roi_queue_stats)
    print(stat);
}

void champsim::plain_printer::print(std::vector<phase_stats>& stats)
{
  for (auto p : stats)
    print(p);
}
