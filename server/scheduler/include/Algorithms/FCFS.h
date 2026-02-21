#pragma once
#include "Common.h"          // ← gets Task + GanttEntry from here
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// ── FCFS-specific result ──────────────────────────────────────
struct FCFSResult {
    std::vector<Task>       tasks;
    std::vector<GanttEntry> gantt;
    int totalClockTime   = 0;
    int totalBusyTime    = 0;
    int contextSwitches  = 0;
    int deadlineMisses   = 0;
};

// ── FCFS Scheduler ────────────────────────────────────────────
class FCFSScheduler {
public:

    explicit FCFSScheduler(int csPenalty = 1)
        : csPenalty_(csPenalty) {}

    FCFSResult run(std::vector<Task> tasks) {
        FCFSResult result;

        std::sort(tasks.begin(), tasks.end(),
            [](const Task& a, const Task& b){
                return a.arrivalTime < b.arrivalTime;
            });

        int clock      = 0;
        int prevTaskId = -1;

        std::cout << "\n+--------------------------------------------------+\n";
        std::cout << "|  FCFS Scheduler  (CS Penalty = "
                  << csPenalty_ << " tick)           |\n";
        std::cout << "+--------------------------------------------------+\n";

        for (auto& t : tasks) {

            // Idle gap
            if (clock < t.arrivalTime) {
                std::cout << "  [IDLE]  " << clock << " -> " << t.arrivalTime << "\n";
                result.gantt.push_back({"IDLE", clock, t.arrivalTime});
                clock = t.arrivalTime;
            }

            // Context switch penalty
            if (prevTaskId != -1 && prevTaskId != t.id) {
                int csEnd = clock + csPenalty_;
                std::cout << "  [CS]    " << clock << " -> " << csEnd << "\n";
                result.gantt.push_back({"CS", clock, csEnd});
                clock = csEnd;
                ++result.contextSwitches;
            }

            t.startTime   = clock;
            int execStart = clock;

            std::cout << "  [RUN]   " << t.name
                      << "  start=" << clock
                      << "  burst=" << t.burstTime
                      << "  deadline=" << t.hardDeadline << "\n";

            // Tick-by-tick execution with deadline check
            for (int i = 0; i < t.burstTime; ++i) {
                ++clock;
                --t.remainingTime;
                if (!t.deadlineMissed && clock > t.hardDeadline) {
                    t.deadlineMissed = true;
                    std::cout << "  !! DEADLINE VIOLATION  " << t.name
                              << "  clock=" << clock
                              << "  deadline=" << t.hardDeadline << "\n";
                }
            }

            result.gantt.push_back({t.name, execStart, clock});
            result.totalBusyTime += t.burstTime;

            t.completionTime = clock;
            t.turnaroundTime = t.completionTime - t.arrivalTime;
            t.waitingTime    = t.turnaroundTime - t.burstTime;
            if (t.deadlineMissed) ++result.deadlineMisses;

            std::cout << "  [DONE]  " << t.name
                      << "  completion=" << t.completionTime
                      << "  TAT=" << t.turnaroundTime
                      << "  WT=" << t.waitingTime
                      << (t.deadlineMissed ? "  !! MISSED" : "  OK") << "\n";

            prevTaskId = t.id;
        }

        result.tasks          = tasks;
        result.totalClockTime = clock;
        std::cout << "\n  FCFS done. Total clock = " << clock << " ticks.\n";
        return result;
    }

    static void printGantt(const FCFSResult& res) {
        std::cout << "\n  Gantt Chart [FCFS]\n  ";
        for (const auto& e : res.gantt) {
            int w = (e.end - e.start) * 4;
            for (int i = 0; i < w; ++i) std::cout << '-';
            std::cout << '+';
        }
        std::cout << "\n  |";
        for (const auto& e : res.gantt) {
            int w    = (e.end - e.start) * 4 - 1;
            int lLen = (int)e.label.size();
            int padL = (w - lLen) / 2;
            int padR = w - lLen - padL;
            for (int i = 0; i < padL; ++i) std::cout << ' ';
            std::cout << e.label;
            for (int i = 0; i < padR; ++i) std::cout << ' ';
            std::cout << '|';
        }
        std::cout << "\n  ";
        for (const auto& e : res.gantt) {
            int w = (e.end - e.start) * 4;
            for (int i = 0; i < w; ++i) std::cout << '-';
            std::cout << '+';
        }
        std::cout << "\n  ";
        int prev = -1;
        for (const auto& e : res.gantt) {
            std::string ts = std::to_string(e.start);
            if (e.start != prev) { std::cout << ts; prev = e.start; }
            int w = (e.end - e.start) * 4 + 1 - (int)ts.size();
            for (int i = 0; i < w; ++i) std::cout << ' ';
        }
        if (!res.gantt.empty()) std::cout << res.gantt.back().end;
        std::cout << "\n";
    }

    static void printMetrics(const FCFSResult& res) {
        std::cout << "\n  Per-Task Metrics [FCFS]\n";
        std::cout << "  " << std::string(68, '-') << "\n";
        std::cout << std::left
                  << "  " << std::setw(6)  << "Task"
                  << std::setw(9)  << "Arrival"
                  << std::setw(8)  << "Burst"
                  << std::setw(11) << "Deadline"
                  << std::setw(12) << "Completion"
                  << std::setw(12) << "Turnaround"
                  << std::setw(9)  << "Waiting"
                  << "Missed\n";
        std::cout << "  " << std::string(68, '-') << "\n";

        double totalTAT = 0, totalWT = 0;
        for (const auto& t : res.tasks) {
            std::cout << std::left
                      << "  " << std::setw(6)  << t.name
                      << std::setw(9)  << t.arrivalTime
                      << std::setw(8)  << t.burstTime
                      << std::setw(11) << t.hardDeadline
                      << std::setw(12) << t.completionTime
                      << std::setw(12) << t.turnaroundTime
                      << std::setw(9)  << t.waitingTime
                      << (t.deadlineMissed ? "YES !!" : "No") << "\n";
            totalTAT += t.turnaroundTime;
            totalWT  += t.waitingTime;
        }
        std::cout << "  " << std::string(68, '-') << "\n";
        int n = (int)res.tasks.size();
        double util = res.totalClockTime > 0
                      ? 100.0 * res.totalBusyTime / res.totalClockTime : 0.0;
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  Avg Turnaround  : " << totalTAT / n << " ticks\n"
                  << "  Avg Waiting     : " << totalWT  / n << " ticks\n"
                  << "  Deadline Misses : " << res.deadlineMisses << "\n"
                  << "  Context Switches: " << res.contextSwitches << "\n"
                  << "  CPU Utilization : " << util << "%\n"
                  << "  Total Clock     : " << res.totalClockTime << " ticks\n";
    }

private:
    int csPenalty_;
};
