#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
using namespace std;

/*
    Real-Time Operating System (RTOS)
    Earliest Deadline First (EDF) Scheduler Simulation
*/

struct Task {
    int id;
    int period;             // Task period
    int execution_time;     // WCET (Worst Case Execution Time)
    int relative_deadline;  // Deadline relative to release time
};

struct Job {
    int task_id;
    int release_time;
    int absolute_deadline;
    int remaining_time;
    int job_number;

    bool operator>(const Job& other) const {
        return absolute_deadline > other.absolute_deadline;
    }
};

class EDFScheduler {
private:
    vector<Task> tasks;
    priority_queue<Job, vector<Job>, greater<Job>> ready_queue;
    int hyperperiod;
    int current_time;
    int deadline_misses;

public:
    EDFScheduler(vector<Task> t, int sim_time)
        : tasks(t), hyperperiod(sim_time), current_time(0), deadline_misses(0) {}

    void releaseJobs() {
        for (auto &task : tasks) {
            if (current_time % task.period == 0) {
                Job new_job;
                new_job.task_id = task.id;
                new_job.release_time = current_time;
                new_job.absolute_deadline = current_time + task.relative_deadline;
                new_job.remaining_time = task.execution_time;
                new_job.job_number = current_time / task.period;

                ready_queue.push(new_job);

                cout << "Time " << current_time 
                     << ": Task " << task.id 
                     << " released (Deadline: " 
                     << new_job.absolute_deadline << ")\n";
            }
        }
    }

    void checkDeadlineMisses() {
        vector<Job> temp;
        while (!ready_queue.empty()) {
            Job job = ready_queue.top();
            ready_queue.pop();

            if (current_time >= job.absolute_deadline && job.remaining_time > 0) {
                cout << "⚠ Deadline Missed! Task " 
                     << job.task_id 
                     << " at time " << current_time << "\n";
                deadline_misses++;
            } else {
                temp.push_back(job);
            }
        }

        for (auto &job : temp)
            ready_queue.push(job);
    }

    void run() {
        cout << "===== EDF Scheduler Simulation =====\n\n";

        while (current_time < hyperperiod) {

            releaseJobs();
            checkDeadlineMisses();

            if (!ready_queue.empty()) {
                Job current_job = ready_queue.top();
                ready_queue.pop();

                cout << "Time " << current_time 
                     << ": Running Task " 
                     << current_job.task_id << "\n";

                current_job.remaining_time--;

                if (current_job.remaining_time > 0) {
                    ready_queue.push(current_job);
                } else {
                    cout << "Time " << current_time + 1 
                         << ": Task " << current_job.task_id 
                         << " completed\n";
                }

            } else {
                cout << "Time " << current_time 
                     << ": CPU Idle\n";
            }

            current_time++;
            cout << "---------------------------------\n";
        }

        cout << "\n===== Simulation Complete =====\n";
        cout << "Total Deadline Misses: " 
             << deadline_misses << endl;
    }
};
