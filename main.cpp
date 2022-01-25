#include<iostream>
#include<queue>
#include<algorithm>
#include <fstream>
using namespace std;

class Process {
    public: 

    int pid, arrival_time, burst_time, completion_time, tot_turn_around_time,
        waiting_time, response_time, copy_burst_time;
    static int process_count;

    Process (int mx_at, int mx_bt) {    
        pid = ++process_count;
        arrival_time = rand()%(mx_at-0 + 1) + 0;
        burst_time = rand()%(mx_bt-1 + 1) + 1;
        copy_burst_time = burst_time; // so that the original BT is not lost
        completion_time = tot_turn_around_time = waiting_time = response_time = -1;
    }   

    void print_info () {
        cout << "{";
        cout << "pid : " << pid;
        cout << ",  AT : " << arrival_time;
        cout << ",  BT : " << burst_time;
        cout << ",  CT : " << completion_time;
        cout << ",  TAT : " << tot_turn_around_time;
        cout << ",  WT : " << waiting_time;
        cout << ",  RT : " << response_time;
        cout << "}\n";
    }
};

int Process::process_count = 0;

bool compare_process(Process *a, Process *b) {
    // for sorting the process list wrt arrival time
    return a->arrival_time < b->arrival_time;
}

class ProcessCreator {
    public:
    Process **process_list;
    int mx_at, mx_bt, num_process;

    ProcessCreator (int sim_time=1000, int mp=20) {
        int practical_sim_time = sim_time / 2;
        num_process = mp;
        mx_at = practical_sim_time;
        mx_bt = practical_sim_time / num_process;
    }

    Process **create () {
        process_list = new Process* [num_process];  // array of pointers to 'Process' objects
        for (int i=0 ; i<num_process ; i++)
            process_list[i] = new Process(mx_at, mx_bt);
        
        // sort the list wrt AT
        sort(process_list, process_list+num_process, compare_process);

        return process_list;
    }

    void print_process_list () {
        for (int i=0 ; i<num_process ; i++)
            process_list[i]->print_info();
    }

    ~ProcessCreator () {
        delete [] process_list;
    }
};

class compare_SRTF {
    public:
    bool operator() (Process *A, Process *B) {
        return A->burst_time > B->burst_time;
    }
};

bool compare_PID (Process *a, Process *b) {
    return a->pid < b->pid;
}

class Simulator{
    int simulation_time, time_quantum; // in ms
    int num_process;
    char scheduling_algo;
    /*
        'f' -> FCFS
        's' -> SRTF
        'r' -> RR
    */
    public:
    Simulator () {}

    Simulator (int st, char sa='f', int tq=-1, int mp=20) {
        simulation_time = st*1000;
        scheduling_algo = sa;
        time_quantum = tq;
        num_process = mp;
    }

    void set_sim_time (int st) {
        simulation_time = st;
    }
    void set_sched_algo (char sa) {
        scheduling_algo = sa;
    }
    void set_time_quant (int tq) {
        time_quantum = tq;
    }

    void start () {
        ProcessCreator pc(simulation_time, num_process);
        Process **process_list = pc.create();

        if ( scheduling_algo == 'f' ) {
            // First come first serve

            // priority_queue<Process*, vector<Process*>, compare_FCFS> ready_queue;
            // for (int i=0 ; i<num_process ; i++)
            //     ready_queue.push( process_list[i] );

            ofstream f;
            f.open("status.txt");
            int i = 0, process_ptr = 0;
            while ( process_ptr<num_process )    {
                Process *t = process_list[process_ptr];

                if ( t->arrival_time >= i ) {
                    i = t->arrival_time;
                    t->response_time = 0;
                }
                else t->response_time = i - t->arrival_time;

                f << "Process with pid: " << t->pid << ", " << " arrived at " << t->arrival_time << "\n";
                f << "Process with pid: " << t->pid << ", " << " executing at " << i << "\n";
                i += t->burst_time;
                f << "Process with pid: " << t->pid << ", " << " exited at " << i << "\n";
                t->completion_time = i;
                t->tot_turn_around_time = t->completion_time - t->arrival_time;
                t->waiting_time = t->tot_turn_around_time - t->burst_time;

                process_ptr ++;
            }
            f.close();

            generate_report(process_list, num_process);
        }

        else if ( scheduling_algo == 's' ) {
            // Shortest Remaining Time First

            priority_queue<Process*, vector<Process*>, compare_SRTF> ready_queue;
            // processes will be pushed in the ready queue as they arrive

            ofstream f;
            f.open("status.txt");
            int process_list_ptr = 0;

            for ( int i=0 ; i <= simulation_time ; ) {
                while ( (process_list_ptr < num_process) and 
                            process_list[process_list_ptr]->arrival_time == i) {
                    f << "Process with pid: " << process_list[process_list_ptr]->pid << ", " << " arrived at " << process_list[process_list_ptr]->arrival_time << "\n";    
                    ready_queue.push(process_list[process_list_ptr]);

                    process_list_ptr ++;
                }

                if ( !ready_queue.empty() ) {
                    // some processes are there in the ready queue
                    Process *t = ready_queue.top();
                    ready_queue.pop();
                    if ( t->burst_time > 0 )
                        f << "Process with pid: " << t->pid << ", " << " executing at " << i << "\n";

                    if (t->response_time == -1)
                        t->response_time = i - t->arrival_time;

                    if (t->burst_time == 0) {
                        f << "Process with pid: " << t->pid << ", " << " completed execution at " << i << "\n";
                        t->completion_time = i;
                        t->tot_turn_around_time = t->completion_time - t->arrival_time;
                        t->waiting_time = t->tot_turn_around_time - t->copy_burst_time;
                        t->burst_time = t->copy_burst_time; // for priting in the result
                    }
                    else {
                        // BT is decremented in else because take for example, a process
                        // with BT = 1 and arrives at 0, then it will execute at time=0 
                        // and finish executing at t=1. If we decremented BT before the 
                        // 'if ( t->burst_time == 0 )' condition then CT would have been
                        // 0 and hence, TAT = 0 and WT = -1, which would be wrong.
                        t->burst_time --;
                        ready_queue.push(t);
                    }                        
                }   
                else 
                    f << "Ready queue is empty (at " << i << ") ...\n";

                i++;
            }
            f.close();

            generate_report(process_list, num_process);
        }

        else if ( scheduling_algo == 'r' ) {
            // Round Robin

            // processes will be pushed in the ready queue as they arrive
            queue<Process *> ready_queue;   
            
            ofstream f;
            f.open("status.txt");
            int process_list_ptr = 0;

            for ( int i=0 ; i <= simulation_time ; ) {
                while ( (process_list_ptr < num_process) and 
                            process_list[process_list_ptr]->arrival_time <= i) {
                    f << "Process with pid: " << process_list[process_list_ptr]->pid << ", " << " arrived at " << process_list[process_list_ptr]->arrival_time << "\n";    
                    ready_queue.push(process_list[process_list_ptr]);

                    process_list_ptr ++;
                }

                if ( !ready_queue.empty() ) {
                    // some processes are there in the ready queue
                    Process *t = ready_queue.front();
                    ready_queue.pop();
                    if ( t->burst_time > 0 )
                        f << "Process with pid: " << t->pid << ", " << " executing at " << i << "\n";

                    if (t->response_time == -1)
                        t->response_time = i - t->arrival_time;

                    if (t->burst_time == 0) {
                        f << "Process with pid: " << t->pid << ", " << " completed execution at " << i << "\n";
                        t->completion_time = i;
                        t->tot_turn_around_time = t->completion_time - t->arrival_time;
                        t->waiting_time = t->tot_turn_around_time - t->copy_burst_time;
                        t->burst_time = t->copy_burst_time; // for priting in the result
                    }
                    else {
                        // BT is decremented in else because take for example, a process
                        // with BT = 1 and arrives at 0, then it will execute at time=0 
                        // and finish executing at t=1. If we decremented BT before the 
                        // 'if ( t->burst_time == 0 )' condition then CT would have been
                        // 0 and hence, TAT = 0 and WT = -1, which would be wrong.
                        if ( t->burst_time >= time_quantum ) {
                            i += (time_quantum-1);
                            t->burst_time -= time_quantum;
                        }
                        else {
                            i += (t->burst_time -1);
                            t->burst_time = 0;
                        }

                        while ( (process_list_ptr < num_process) and 
                                    process_list[process_list_ptr]->arrival_time <= i) {
                            f << "Process with pid: " << process_list[process_list_ptr]->pid << ", " << " arrived at " << process_list[process_list_ptr]->arrival_time << "\n";    
                            ready_queue.push(process_list[process_list_ptr]);

                            process_list_ptr ++;
                        }

                        ready_queue.push(t);
                    }                        
                }   
                else {
                    if (process_list_ptr == num_process and ready_queue.empty())
                        break;
                    else f << "Ready Queue is empty (at " << i << ") ...\n";
                } 
                i++;
            }            
            f.close();

            generate_report(process_list, num_process);
        }

        else cout << "Not a valid scheduling algo.\n";
    }

    void generate_report (Process **process_list, int size) {
        // generating the TAT, WT, RT table
        // cout << "SIZE = " << size << "\n";

        ofstream f;
        f.open ("result.txt");
        f << "pid\t|\tAT\t|\tBT\t|\tCT\t|\tTAT\t|\tWT\t|\tRT\n";
        f << "___________________________________________________\n";

        sort( process_list, process_list+size, compare_PID );

        for (int i=0 ; i<size ; i++) {
            if ( scheduling_algo == 's' )
                process_list[i]->burst_time = process_list[i]->copy_burst_time;

            f << process_list[i]->pid << "\t|\t" << process_list[i]->arrival_time << "\t|\t" 
            << process_list[i]->burst_time << "\t|\t" << process_list[i]->completion_time 
            << "\t|\t" << process_list[i]->tot_turn_around_time << "\t|\t" 
            << process_list[i]->waiting_time << "\t|\t" << process_list[i]->response_time << "\n";
        }
            
        f.close();
    }
};

int main () {
    cout << "Enter simulation time(in seconds): ";
    int time; cin >> time;
    cout << "Enter the type of scheduling algo that you would like to simulate: ";
    cout << "Options: \n\t'f' -> FCFS\n\t's' -> SRTF\n\t'r' -> RR\n<- ";
    char input; cin >> input;
    if ( input == 'f' or input == 's'){
        Simulator s(time, input);
        s.start();
    }
    else if (input == 'r') {
        cout << "Enter the time quantum(in ms): ";
        int tq; cin >> tq;
        Simulator s(time, input, tq);
        s.start();
    }    
    return 0;
}