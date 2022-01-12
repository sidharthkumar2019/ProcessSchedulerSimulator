#include<iostream>
#include<queue>
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
        for (int i=0 ; i<20 ; i++)
            process_list[i]->print_info();
    }

    ~ProcessCreator () {
        delete [] process_list;
    }
};

class compare_FCFS {
    public:
    bool operator() (Process &A, Process &B) {
        if ( A.arrival_time > B.arrival_time ) return true;
        else if ( A.arrival_time == B.arrival_time )  {
            return A.pid > B.pid;
        }
        return false;
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

            priority_queue<Process, vector<Process>, compare_FCFS> ready_queue;
            for (int i=0 ; i<num_process ; i++)
                ready_queue.push( *process_list[i] );

            int i = 0;
            while (!ready_queue.empty())    {
                Process t = ready_queue.top();
                t.print_info();


                i = t.arrival_time;
                

                ready_queue.pop();
            }
            

            // int i = 1;
            // while (i<=simulation_time) {
            //     cout << "After " << i << " ms, \n";
            //     i++;

            //     // Also print the status in status.txt after each meaning full iteration
            // }
        }
    }

    void generate_report () {
        // generating the TAT, WT, RT table
    }

    ~Simulator() {
        cout << "Generated the status.txt file";
    }
};

int Process::process_count = 0;

int main () {
    Simulator s(1, 'f');
    s.start();

    // ProcessCreator pc(1000);
    // Process **arr = pc.create();
    // pc.print_process_list();
}