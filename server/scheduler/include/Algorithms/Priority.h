#pragma once
#include <vector>
#include <algorithm>

using namespace std;

struct Task {
    int id;
    int arrivalTime;
    int burstTime;
    int priority;
    int completionTime;
    int waitingTime;
    int turnaroundTime;
    bool completed = false;
};

void priorityScheduling(vector<Task>& tasks) {

    int currentTime = 0;
    int completedTasks = 0;
    int n = tasks.size();

    while (completedTasks < n) {

        int idx = -1;
        int highestPriority = 1e9;

        for (int i = 0; i < n; i++) {
            if (!tasks[i].completed && tasks[i].arrivalTime <= currentTime) {

                if (tasks[i].priority < highestPriority) {
                    highestPriority = tasks[i].priority;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            currentTime += tasks[idx].burstTime;

            tasks[idx].completionTime = currentTime;
            tasks[idx].turnaroundTime =
                tasks[idx].completionTime - tasks[idx].arrivalTime;
            tasks[idx].waitingTime =
                tasks[idx].turnaroundTime - tasks[idx].burstTime;

            tasks[idx].completed = true;
            completedTasks++;
        }
        else {
            currentTime++;
        }
    }
}
