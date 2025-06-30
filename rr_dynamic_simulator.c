#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_PROCESSES 50
#define QUEUE_LEVELS 3
#define QUANTUMS {5, 10, 20}  // Quanta para as 3 filas

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
    int preemptions;
    int waiting_time;
    int current_queue;
} SimulatedProcess;

void simulate_cpu(int seconds) {}
void simulate_io(int seconds) {}

int main() {
    SimulatedProcess processes[MAX_PROCESSES];
    int n, global_time = 0;
    int quanta[] = QUANTUMS;

    scanf("%d", &n);
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        scanf("%d %d %d %d", &processes[i].arrival_time, &processes[i].cpu_time,
              &processes[i].io_time, &processes[i].cycles);
        processes[i].current_cycle = 0;
        processes[i].remaining_cpu = processes[i].cpu_time;
        processes[i].finished = 0;
        processes[i].start_time = -1;
        processes[i].end_time = -1;
        processes[i].total_cpu_time = 0;
        processes[i].total_io_time = 0;
        processes[i].preemptions = 0;
        processes[i].waiting_time = 0;
        processes[i].current_queue = 0; // Começa na fila de maior prioridade
    }

    while (1) {
        int all_finished = 1;
        for (int level = 0; level < QUEUE_LEVELS; level++) {
            for (int i = 0; i < n; i++) {
                SimulatedProcess *proc = &processes[i];
                if (proc->finished || proc->arrival_time > global_time || proc->current_queue != level)
                    continue;

                all_finished = 0;
                if (proc->start_time == -1) proc->start_time = global_time;

                int exec_time = (proc->remaining_cpu < quanta[level]) ? proc->remaining_cpu : quanta[level];

                simulate_cpu(exec_time);
                global_time += exec_time;
                proc->remaining_cpu -= exec_time;
                proc->total_cpu_time += exec_time;

                for (int j = 0; j < n; j++) {
                    if (i != j && !processes[j].finished && processes[j].arrival_time <= global_time)
                        processes[j].waiting_time += exec_time;
                }

                if (proc->remaining_cpu <= 0) {
                    simulate_io(proc->io_time);
                    global_time += proc->io_time;
                    proc->total_io_time += proc->io_time;
                    proc->current_cycle++;
                    if (proc->current_cycle >= proc->cycles) {
                        proc->finished = 1;
                        proc->end_time = global_time;
                    } else {
                        proc->remaining_cpu = proc->cpu_time;
                        proc->current_queue = 0; // Reinicia na fila de alta prioridade
                    }
                } else {
                    proc->preemptions++;
                    if (level + 1 < QUEUE_LEVELS)
                        proc->current_queue++; // Demove de fila
                }

                goto next_cycle; // Processa um por vez
            }
        }

        if (all_finished) break;
        global_time++;
    next_cycle:;
    }

    for (int i = 0; i < n; i++) {
        int tat = processes[i].end_time - processes[i].arrival_time;
        printf("\nProcess %d Report:\n", processes[i].pid);
        printf("Arrival Time: %d\n", processes[i].arrival_time);
        printf("Start Time: %d\n", processes[i].start_time);
        printf("End Time: %d\n", processes[i].end_time);
        printf("Turnaround Time: %d\n", tat);
        printf("Total CPU Time: %d\n", processes[i].total_cpu_time);
        printf("Total I/O Time: %d\n", processes[i].total_io_time);
        printf("Waiting Time: %d\n", processes[i].waiting_time);
        printf("Number of Preemptions: %d\n", processes[i].preemptions);
        printf("Cycles Completed: %d\n", processes[i].current_cycle);
    }

    return 0;
}
