#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_PROCESSES 20

typedef struct {
    int pid;
    int arrival_time;
    int cpu_time;
    int io_time;
    int cycles;
    int current_cycle;
    int finished;
    int priority;
    int start_time;
    int end_time;
    int total_cpu_time;
    int total_io_time;
    int waiting_time;
} SimulatedProcess;

void simulate_cpu(int seconds) {
    // sleep(seconds);
}

void simulate_io(int seconds) {
    // sleep(seconds);
}

void sort_by_arrival_time(SimulatedProcess processes[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                SimulatedProcess temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
}

int main() {
    SimulatedProcess processes[MAX_PROCESSES];
    int n;
    int global_time = 0;

    printf("Enter number of processes (max %d):\n", MAX_PROCESSES);
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        printf("Process %d - Arrival time:\n", processes[i].pid);
        scanf("%d", &processes[i].arrival_time);
        printf("Process %d - CPU time per cycle:\n", processes[i].pid);
        scanf("%d", &processes[i].cpu_time);
        printf("Process %d - I/O time per cycle:\n", processes[i].pid);
        scanf("%d", &processes[i].io_time);
        printf("Process %d - Number of cycles:\n", processes[i].pid);
        scanf("%d", &processes[i].cycles);
        printf("Process %d - Priority:\n", processes[i].pid);
        scanf("%d", &processes[i].priority);

        processes[i].current_cycle = 0;
        processes[i].finished = 0;
        processes[i].start_time = -1;
        processes[i].end_time = 0;
        processes[i].total_cpu_time = 0;
        processes[i].total_io_time = 0;
        processes[i].waiting_time = 0;
    }

    sort_by_arrival_time(processes, n);

    printf("\nStarting FCFS simulation with Arrival Times\n\n");

    for (int i = 0; i < n; i++) {
        // Aguarda chegada do processo, se necessário
        if (global_time < processes[i].arrival_time) {
            global_time = processes[i].arrival_time;
        }

        processes[i].start_time = global_time;
        processes[i].waiting_time = processes[i].start_time - processes[i].arrival_time;

        for (int c = 0; c < processes[i].cycles; c++) {
            printf("[Time %d] Process %d executing cycle %d for %d s\n",
                   global_time, processes[i].pid, c + 1, processes[i].cpu_time);

            simulate_cpu(processes[i].cpu_time);
            global_time += processes[i].cpu_time;
            processes[i].total_cpu_time += processes[i].cpu_time;

            printf("[Time %d] Process %d finished cycle %d, performing I/O for %d s\n",
                   global_time, processes[i].pid, c + 1, processes[i].io_time);

            simulate_io(processes[i].io_time);
            global_time += processes[i].io_time;
            processes[i].total_io_time += processes[i].io_time;

            processes[i].current_cycle++;
        }

        processes[i].end_time = global_time;
        processes[i].finished = 1;

        printf("[Time %d] Process %d finished all cycles.\n", global_time, processes[i].pid);
    }

    printf("\nAll processes completed.\n");

    printf("\n==== Execution Report ====\n");
    for (int i = 0; i < n; i++) {
        int turnaround_time = processes[i].end_time - processes[i].arrival_time;
        printf("\nProcess %d Report:\n", processes[i].pid);
        printf("Arrival Time: %d\n", processes[i].arrival_time);
        printf("Start Time: %d\n", processes[i].start_time);
        printf("End Time: %d\n", processes[i].end_time);
        printf("Turnaround Time: %d\n", turnaround_time);
        printf("Waiting Time: %d\n", processes[i].waiting_time);
        printf("Total CPU Time: %d\n", processes[i].total_cpu_time);
        printf("Total I/O Time: %d\n", processes[i].total_io_time);
        printf("Cycles Completed: %d\n", processes[i].current_cycle);
        printf("Priority: %d\n", processes[i].priority);
    }

    return 0;
}
