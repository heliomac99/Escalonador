#include <stdio.h>
#include <stdlib.h>

/*
 *  MLFQ com conjunto ativo + Round‑Robin adaptado
 *  ------------------------------------------------
 *  • Seleciona ±1/3 dos processos PRONTOS (maior priority).
 *  • PRIORIDADE inicial = io_time; aging: +1 a cada tick esperando.
 *  • Conjunto ativo: cada processo deve executar **ao menos 5 ticks** antes
 *    de o conjunto ser reconstruído.
 *  • Quantum para Round‑Robin é **fixo**: igual ao menor cpu_time entre todos
 *    os processos (constante ao longo de toda a simulação).
 *  • Execução tick‑a‑tick; global_time sempre ++ por iteração.
 */

#define MAX_PROCESSES 50
#define MIN(a,b) ((a)<(b)?(a):(b))

typedef struct {
    int pid, arrival_time, cpu_time, io_time, cycles;
    /* estado */
    int current_cycle, remaining_cpu, blocked_until, finished;
    int quantum_left;          /* restante do quantum corrente */
    /* métricas */
    int start_time, end_time, total_cpu_time, total_io_time;
    int waiting_time, preemptions;
    /* seleção */
    int priority, active, starting_priority;
} PCB;

static int all_done(PCB *p, int n) {
    for (int i = 0; i < n; ++i)
        if (!p[i].finished) return 0;
    return 1;
}

static int ready(const PCB *p, int now) {
    return (!p->finished && p->arrival_time <= now && p->blocked_until <= now);
}

int main() {
    PCB proc[MAX_PROCESSES];
    int n;
    if (scanf("%d", &n) != 1 || n < 1 || n > MAX_PROCESSES) return 0;

    int quantum_global = 0; /* menor cpu_time global */

    for (int i = 0; i < n; i++) {
        proc[i].pid = i + 1;
        scanf("%d%d%d%d%d", &proc[i].arrival_time, &proc[i].cpu_time,
              &proc[i].io_time, &proc[i].cycles, &proc[i].priority);

        proc[i].current_cycle   = 0;
        proc[i].remaining_cpu   = proc[i].cpu_time;
        proc[i].blocked_until   = -1;
        proc[i].finished        = 0;
        proc[i].quantum_left    = 0;
        proc[i].starting_priority = proc[i].priority;
        proc[i].start_time      = -1;
        proc[i].end_time        = -1;
        proc[i].total_cpu_time  = 0;
        proc[i].total_io_time   = 0;
        proc[i].waiting_time    = 0;
        proc[i].preemptions     = 0;

        proc[i].active          = 0;

        if (quantum_global == 0 || proc[i].cpu_time < quantum_global)
            quantum_global = proc[i].cpu_time;
    }

    const int ACTIVE_CAP = (n + 2) / 3; /* ~1/3 arredondado */
    int global_time = 0;
    int rr_idx      = -1;
    int active_ticks[MAX_PROCESSES] = {0};
    int need_refresh               = 1;

    while (!all_done(proc, n)) {
        /* 1) Aging + waiting */
        for (int i = 0; i < n; i++) {
            if (ready(&proc[i], global_time)) {
                proc[i].priority++;
                if (i != rr_idx) proc[i].waiting_time++;
            }
        }

        /* 2) (Re)construção do conjunto ativo se necessário */
        if (need_refresh) {
            for (int i = 0; i < n; i++) {
                proc[i].active      = 0;
                active_ticks[i]     = 0;
            }
            for (int k = 0; k < ACTIVE_CAP; k++) {
                int best = -1;
                for (int i = 0; i < n; i++) {
                    if (!ready(&proc[i], global_time) || proc[i].active) continue;
                    if (best == -1 || proc[i].priority > proc[best].priority)
                        best = i;
                }
                if (best != -1) proc[best].active = 1;
            }
            for (int i = 0; i < n; i++) {
                if (proc[i].active) {
                    proc[i].quantum_left = MIN(proc[i].remaining_cpu, quantum_global);
                }
            }
            rr_idx      = -1;
            need_refresh = 0;
        }

        /* 3) Escolhe próximo ativo pronto via RR */
        int chosen = -1;
        for (int step = 1; step <= n; step++) {
            int idx = (rr_idx + step) % n;
            if (proc[idx].active && ready(&proc[idx], global_time)) {
                chosen = idx;
                break;
            }
        }

        if (chosen != -1) {
            PCB *p = &proc[chosen];
            rr_idx    = chosen;

            if (p->start_time == -1)
                p->start_time = global_time;

            /* executa 1 tick */
            p->remaining_cpu--;
            p->quantum_left--;
            p->total_cpu_time++;
            p->priority = 0;
            active_ticks[chosen]++;

            if (p->remaining_cpu == 0) {
                p->current_cycle++;
                if (p->current_cycle >= p->cycles) {
                    p->finished   = 1;
                    p->end_time   = global_time + 1;
                    p->active     = 0;
                } else {
                    p->remaining_cpu = p->cpu_time;
                    p->blocked_until = global_time + 1 + p->io_time;
                    p->total_io_time += p->io_time;
                    p->quantum_left  = 0;
                    p->active        = 0;
                }
            } else if (p->quantum_left == 0) {
                p->preemptions++;
                p->quantum_left = MIN(p->remaining_cpu, quantum_global);
            }
        } else {
            rr_idx = -1;
        }

        /* 4) Verifica se todos ativos executaram >=5 ticks ou não estão prontos */
        int done_cycle = 1;
        for (int i = 0; i < n; i++) {
            if (proc[i].active) {
                if (active_ticks[i] < 5 && ready(&proc[i], global_time)) {
                    done_cycle = 0;
                    break;
                }
            }
        }
        if (done_cycle) need_refresh = 1;

        global_time++;
    }

    /* relatório */
    for (int i = 0; i < n; i++) {
        int tat = proc[i].end_time - proc[i].arrival_time;
        printf("Process %d Report:\n", proc[i].pid);
        printf("Arrival Time: %d\nStart Time: %d\nEnd Time: %d\n", proc[i].arrival_time, proc[i].start_time, proc[i].end_time);
        printf("Turnaround Time: %d\nTotal CPU Time: %d\nTotal I/O Time: %d\n", tat, proc[i].total_cpu_time, proc[i].total_io_time);
        printf("Waiting Time: %d\nNumber of Preemptions: %d\nCycles Completed: %d\nPriority: %d\n\n", proc[i].waiting_time, proc[i].preemptions, proc[i].current_cycle, proc[i].starting_priority);
    }

    return 0;
}
