#include <stdio.h>
#include <stdlib.h>

#define MAX_PROCESSES 20
#define QUANTUM 10

typedef struct {
    int pid;
    int arrival_time;
    int cpu_time;
    int io_time;
    int cycles;
    int current_cycle;
    int remaining_cpu;
    int finished;

    int start_time;
    int end_time;
    int total_cpu_time;
    int total_io_time;
    int waiting_time;
    int preemptions;
    int blocked_until;
} SimulatedProcess;

int main() {
    SimulatedProcess processes[MAX_PROCESSES];
    int n;
    int global_time = 0;

    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        scanf("%d", &processes[i].arrival_time);
        scanf("%d", &processes[i].cpu_time);
        scanf("%d", &processes[i].io_time);
        scanf("%d", &processes[i].cycles);

        processes[i].current_cycle = 0;
        processes[i].remaining_cpu = processes[i].cpu_time;
        processes[i].finished = 0;
        processes[i].start_time = -1;
        processes[i].end_time = -1;
        processes[i].total_cpu_time = 0;
        processes[i].total_io_time = 0;
        processes[i].waiting_time = 0;
        processes[i].preemptions = 0;
        processes[i].blocked_until = -1;
    }

    while (1) {
        int executed = 0;
        int all_finished = 1;

        for (int i = 0; i < n; i++) {
            if (processes[i].finished)
                continue;

            if (processes[i].arrival_time > global_time ||
                processes[i].blocked_until > global_time) {
                all_finished = 0;
                continue;
            }

            all_finished = 0;
            executed = 1;

            if (processes[i].start_time == -1)
                processes[i].start_time = global_time;

            int exec_time = (processes[i].remaining_cpu < QUANTUM) ? processes[i].remaining_cpu : QUANTUM;

            global_time += exec_time;
            processes[i].remaining_cpu -= exec_time;
            processes[i].total_cpu_time += exec_time;

            if (processes[i].remaining_cpu <= 0) {
                processes[i].total_io_time += processes[i].io_time;
                processes[i].current_cycle++;

                if (processes[i].current_cycle >= processes[i].cycles) {
                    processes[i].finished = 1;
                    processes[i].end_time = global_time;
                } else {
                    processes[i].remaining_cpu = processes[i].cpu_time;
                    processes[i].blocked_until = global_time + processes[i].io_time;
                }
            } else {
                processes[i].preemptions++;
            }
        }

        if (all_finished) break;
        if (!executed) global_time++;
    }

    // Relatório compatível com o generate_json (texto)
    for (int i = 0; i < n; i++) {
        int turnaround = processes[i].end_time - processes[i].arrival_time;
        int waiting = turnaround - processes[i].total_cpu_time - processes[i].total_io_time;
        processes[i].waiting_time = waiting;

        printf("Process %d Report:\n", processes[i].pid);
        printf("Arrival Time: %d\n", processes[i].arrival_time);
        printf("Start Time: %d\n", processes[i].start_time);
        printf("End Time: %d\n", processes[i].end_time);
        printf("Turnaround Time: %d\n", turnaround);
        printf("Total CPU Time: %d\n", processes[i].total_cpu_time);
        printf("Total I/O Time: %d\n", processes[i].total_io_time);
        printf("Waiting Time: %d\n", waiting);
        printf("Number of Preemptions: %d\n", processes[i].preemptions);
        printf("Cycles Completed: %d\n", processes[i].current_cycle);
    }

    return 0;
}
