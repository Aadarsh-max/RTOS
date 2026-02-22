#include <vector>
using namespace std;

struct Task {
    int id;
    int arrival_time;
    int execution_time;
    int period;   // Used as priority (smaller = higher priority)
};

/*
    Rate Monotonic Scheduling Function
    Parameters:
        tasks  -> vector of tasks
        n      -> number of tasks
    Returns:
        vector<int> -> execution order (task IDs per time unit)
*/

vector<int> RM_Schedule(vector<Task> tasks, int n) {

    vector<int> execution_order;
    vector<int> remaining(n);
    vector<bool> completed(n, false);

    // Initialize remaining time
    for (int i = 0; i < n; i++)
        remaining[i] = tasks[i].execution_time;

    int time = 0;
    int completed_tasks = 0;

    while (completed_tasks < n) {

        int highest_priority = -1;
        int min_period = 1e9;

        // Select task with smallest period (highest priority)
        for (int i = 0; i < n; i++) {
            if (tasks[i].arrival_time <= time &&
                !completed[i] &&
                remaining[i] > 0 &&
                tasks[i].period < min_period) {

                min_period = tasks[i].period;
                highest_priority = i;
            }
        }

        if (highest_priority != -1) {
            execution_order.push_back(tasks[highest_priority].id);
            remaining[highest_priority]--;

            if (remaining[highest_priority] == 0) {
                completed[highest_priority] = true;
                completed_tasks++;
            }
        } else {
            execution_order.push_back(-1);  // CPU Idle
        }

        time++;
    }

    return execution_order;
}
