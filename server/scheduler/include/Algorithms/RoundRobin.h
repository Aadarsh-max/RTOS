#pragma once
#include "Common.h"          // ← gets Task + GanttEntry from here
#include <vector>
#include <deque>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

// ── RR-specific result ────────────────────────────────────────
struct RRResult {
    std::vector<Task>       tasks;
    std::vector<GanttEntry> gantt;
    int totalClockTime   = 0;
    int totalBusyTime    = 0;
    int contextSwitches  = 0;
    int deadlineMisses   = 0;
};

// ── Round Robin Scheduler ─────────────────────────────────────
class RRScheduler {
public:

    RRScheduler(int quantum, int csPenalty = 1)
        : quantum_(quantum), csPenalty_(csPenalty) {}

    RRResult run(std::vector<Task> tasks) {
        RRResult result;

        std::sort(tasks.begin(), tasks.end(),
            [](const Task& a, const Task& b){
                return a.arrivalTime < b.arrivalTime;
            });

        const int       N = (int)tasks.size();
        std::deque<int> readyQ;
        int clock      = 0;
        int nextArrive = 0;
        int completed  = 0;
        int prevTaskId = -1;

        std::cout << "\n+--------------------------------------------------+\n";
        std::cout << "|  Round Robin  (Q=" << quantum_
                  << "  CS Penalty=" << csPenalty_ << " tick)        |\n";
        std::cout << "+--------------------------------------------------+\n";

        auto enqueue = [&]() {
            while (nextArrive < N && tasks[nextArrive].arrivalTime <= clock) {
                readyQ.push_back(nextArrive);
                std::cout << "  [ARR]   " << tasks[nextArrive].name
                          << " arrived at tick " << tasks[nextArrive].arrivalTime << "\n";
                ++nextArrive;
            }
        };

        if (N > 0) clock = tasks[0].arrivalTime;
        enqueue();

        while (completed < N) {

            if (readyQ.empty()) {
                if (nextArrive < N) {
                    int idleEnd = tasks[nextArrive].arrivalTime;
                    std::cout << "  [IDLE]  " << clock << " -> " << idleEnd << "\n";
                    result.gantt.push_back({"IDLE", clock, idleEnd});
                    clock = idleEnd;
                    enqueue();
                }
                continue;
            }

            int   idx = readyQ.front(); readyQ.pop_front();
            Task& t   = tasks[idx];

            // Context switch
            if (prevTaskId != -1 && prevTaskId != t.id) {
                int csEnd = clock + csPenalty_;
                std::cout << "  [CS]    " << clock << " -> " << csEnd
                          << "  (switch to " << t.name << ")\n";
                result.gantt.push_back({"CS", clock, csEnd});
                clock = csEnd;
                ++result.contextSwitches;
            }

            if (t.startTime == -1) t.startTime = clock;

            int slice      = std::min(quantum_, t.remainingTime);
            int sliceStart = clock;

            std::cout << "  [RUN]   " << t.name
                      << "  clock=" << clock
                      << "  slice=" << slice
                      << "  remaining=" << t.remainingTime
                      << "  deadline=" << t.hardDeadline << "\n";

            for (int i = 0; i < slice; ++i) {
                ++clock; --t.remainingTime;
                if (!t.deadlineMissed && clock > t.hardDeadline) {
                    t.deadlineMissed = true;
                    std::cout << "  !! DEADLINE VIOLATION  " << t.name
                              << "  clock=" << clock
                              << "  deadline=" << t.hardDeadline << "\n";
                }
            }

            result.gantt.push_back({t.name, sliceStart, clock});
            result.totalBusyTime += slice;
            prevTaskId = t.id;

            enqueue();

            if (t.remainingTime == 0) {
                t.completionTime = clock;
                t.turnaroundTime = t.completionTime - t.arrivalTime;
                t.waitingTime    = t.turnaroundTime - t.burstTime;
                if (t.deadlineMissed) ++result.deadlineMisses;
                ++completed;
                std::cout << "  [DONE]  " << t.name
                          << "  completion=" << t.completionTime
                          << "  TAT=" << t.turnaroundTime
                          << "  WT=" << t.waitingTime
                          << (t.deadlineMissed ? "  !! MISSED" : "  OK") << "\n";
            } else {
                readyQ.push_back(idx);
                std::cout << "  [PRE]   " << t.name
                          << "  preempted, remaining=" << t.remainingTime << "\n";
            }
        }

        result.tasks          = tasks;
        result.totalClockTime = clock;
        std::cout << "\n  Round Robin done. Total clock = " << clock << " ticks.\n";
        return result;
    }

    static void printGantt(const RRResult& res) {
        std::cout << "\n  Gantt Chart [Round Robin]\n  ";
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

    static void printMetrics(const RRResult& res) {
        std::cout << "\n  Per-Task Metrics [Round Robin]\n";
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
    int quantum_;
    int csPenalty_;
};
