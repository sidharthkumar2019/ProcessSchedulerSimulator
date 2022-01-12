#include<iostream>
#include<queue>
#include <fstream>
using namespace std;

class Process {
    public: 

    int pid, arrival_time, burst_time, completion_time, tot_turn_around_time,
        waiting_time, response_time;
    static int process_count;

    Process (int mx_at, int mx_bt) {    
        pid = ++process_count;
        arrival_time = rand()%(mx_at-0 + 1) + 0;
        burst_time = rand()%(mx_bt-1 + 1) + 1;
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
        process_list = new Process* [num_process];
        for (int i=0 ; i<num_process ; i++)
            process_list[i] = new Process(mx_at, mx_bt);

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

class compare_FCFS {
    public:
    bool operator() (Process *A, Process *B) {
        return A->arrival_time > B->arrival_time;
    }
};

class compare_SJF {
    public:
    bool operator() (Process *A, Process *B) {
        return A->burst_time > B->burst_time;
    }
};

class Simulator{
    int simulation_time, time_quantum; // in ms
    int num_process;
    char scheduling_algo;
    /*
        'f' -> FCFS
        's' -> SJF
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

            priority_queue<Process*, vector<Process*>, compare_FCFS> ready_queue;
            for (int i=0 ; i<num_process ; i++)
                ready_queue.push( process_list[i] );

            ofstream f;
            f.open("status.txt");
            int i = 0;
            while (!ready_queue.empty())    {
                Process *t = ready_queue.top();

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

                ready_queue.pop();
            }
            f.close();

            generate_report(process_list, num_process);
        }

        else if ( scheduling_algo == 's' ) {
            // Shortest Job First

            priority_queue<Process*, vector<Process*>, compare_SJF> ready_queue;
            for (int i=0 ; i<num_process ; i++)
                ready_queue.push( process_list[i] );

            ofstream f;
            f.open("status.txt");
            int i = 0;
            while (!ready_queue.empty())    {
                Process *t = ready_queue.top();
                // t->print_info();

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

                ready_queue.pop();
            }
            f.close();

            generate_report(process_list, num_process);
        }

        else if ( scheduling_algo == 'r' ) {
            // Round Robin

            priority_queue<Process*, vector<Process*>, compare_FCFS> temp;
            for (int i=0 ; i<num_process ; i++)
                temp.push( process_list[i] );

            queue<Process *> ready_queue;
            while (!temp.empty())   {
                ready_queue.push( temp.top() );
                temp.pop();
            }
            
            ofstream f;
            f.open("status.txt");
            int i = 0;
            while (!ready_queue.empty())    {
                Process *t = ready_queue.front();
                ready_queue.pop();
                // t->print_info();

                if ( t->arrival_time >= i ) {
                    i = t->arrival_time;
                    t->response_time = 0;
                }
                else t->response_time = i - t->arrival_time;

                f << "Process with pid: " << t->pid << ", " << " arrived at " << t->arrival_time << "\n";
                f << "Process with pid: " << t->pid << ", " << " executing at " << i << "\n";

                if ( t->burst_time > time_quantum ) {
                    i += time_quantum;
                    t->burst_time -= time_quantum;
                    ready_queue.push(t);
                }
                else {
                    i += t->burst_time;
                    f << "Process with pid: " << t->pid << ", " << " exited at " << i << "\n";
                    t->completion_time = i;
                    t->tot_turn_around_time = t->completion_time - t->arrival_time;
                    t->waiting_time = t->tot_turn_around_time - t->burst_time;
                }                
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
        for (int i=0 ; i<size ; i++) 
            f << process_list[i]->pid << "\t|\t" << process_list[i]->arrival_time << "\t|\t" 
            << process_list[i]->burst_time << "\t|\t" << process_list[i]->completion_time 
            << "\t|\t" << process_list[i]->tot_turn_around_time << "\t|\t" 
            << process_list[i]->waiting_time << "\t|\t" << process_list[i]->response_time << "\n";
        f.close();
    }
};

int Process::process_count = 0;

int main () {
    cout << "Enter simulation time(in seconds): ";
    int sim_time; cin >> sim_time;
    cout << "Enter the type of scheduling algo that you would like to simulate: ";
    cout << "Options: \n\t'f' -> FCFS\n\t's' -> SJF\n\t'r' -> RR\n<-";
    char input; cin >> input;
    if ( input == 'f' or input == 's'){
        Simulator s(sim_time, input);
        s.start();
    }
    else if (input == 'r') {
        cout << "Enter the time quantum(in ms): ";
        int tq; cin >> tq;
        Simulator s(sim_time, input, tq);
        s.start();
    }    
    return 0;
}