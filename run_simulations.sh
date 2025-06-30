#!/bin/bash

# --- Entradas diferentes ---
INPUTS=(
# 5 processos: diversos perfis
"5
0 8 3 4
1 10 2 3
2 2 6 6
3 5 1 2
4 12 4 2"

# 10 processos: misto de CPU-bound e IO-bound
"10
0 15 2 3
1 3 6 4
2 8 2 5
3 10 4 2
4 5 8 1
5 6 5 3
6 12 1 6
7 4 9 2
8 9 3 3
9 7 5 2"

# 15 processos: carga moderada, variado
"15
0 4 3 5
1 10 2 4
2 2 1 2
3 8 4 3
4 3 2 6
5 7 5 3
6 9 2 4
7 6 6 2
8 12 2 2
9 13 3 3
10 1 7 1
11 11 4 3
12 5 5 2
13 14 3 2
14 15 4 1"

# 20 processos: carga alta, alternância
"20
0 6 2 3
1 9 1 2
2 4 5 3
3 8 3 2
4 7 2 3
5 12 4 2
6 1 7 1
7 5 2 4
8 3 1 3
9 10 2 2
10 6 3 2
11 11 1 2
12 2 6 2
13 13 3 3
14 14 2 2
15 0 5 2
16 15 4 1
17 16 2 2
18 17 1 1
19 18 3 3"
)

# Diretórios de saída
TXT_DIR="outputs/txt"
JSON_DIR="outputs/json"
mkdir -p "$TXT_DIR" "$JSON_DIR"

# Arquivos fonte e binários
PROGRAMS=("rr_simulator.c" "fcfs_simulator.c" "rr_dynamic_simulator.c")
BINARIES=("rr_simulator" "fcfs_simulator" "rr_dynamic_simulator")

# Compilar simuladores
for i in "${!PROGRAMS[@]}"; do
  if [[ ! -f "${BINARIES[$i]}" ]]; then
    gcc "${PROGRAMS[$i]}" -o "${BINARIES[$i]}"
    chmod +x "${BINARIES[$i]}"
  fi
done

# Função para gerar JSON
generate_json() {
    local output_file=$1
    local json_file=$2
    local type=$3

    echo "[" > "$json_file"

    awk -v type="$type" '
    BEGIN { first = 1 }
    /^Process [0-9]+ Report:/ {
        pid = $2
        getline; split($0, a, ": "); arrival = a[2]
        getline; split($0, a, ": "); start = a[2]
        getline; split($0, a, ": "); end = a[2]
        getline; split($0, a, ": "); tat = a[2]
        getline; split($0, a, ": "); cpu = a[2]
        getline; split($0, a, ": "); io = a[2]
        waiting = tat - cpu - io

        if (type == "FCFS") {
            getline; split($0, a, ": "); cycles = a[2]
            preempt = "null"
        } else {
            getline; split($0, a, ": "); preempt = a[2]
            getline; split($0, a, ": "); cycles = a[2]
        }

        if (first == 0) printf(",\n") >> outfile
        first = 0

        printf("{\"Process\": %s, \"Arrival\": %s, \"Start\": %s, \"End\": %s, \"Turnaround\": %s, \"CPU\": %s, \"IO\": %s, \"Waiting\": %s, ", pid, arrival, start, end, tat, cpu, io, waiting) >> outfile

        if (type == "FCFS") {
            printf("\"Cycles\": %s}", cycles) >> outfile
        } else {
            printf("\"Preemptions\": %s, \"Cycles\": %s}", preempt, cycles) >> outfile
        }
    }
    ' outfile="$json_file" "$output_file"

    echo "]" >> "$json_file"
}

# Loop pelos inputs
for idx in "${!INPUTS[@]}"; do
    input_file="input_$((idx+1)).txt"
    echo -e "${INPUTS[$idx]}" > "$input_file"

    for bin in "${BINARIES[@]}"; do
        output_file="$TXT_DIR/${bin%.c}_output_$((idx+1)).txt"
        json_file="$JSON_DIR/${bin%.c}_output_$((idx+1)).json"
        exe="./${bin}"

        echo "Executando $exe com entrada $input_file..."
        $exe < "$input_file" > "$output_file"

        if [[ "$bin" == "fcfs_simulator" ]]; then
            generate_json "$output_file" "$json_file" "FCFS"
        else
            generate_json "$output_file" "$json_file" "RR"
        fi
    done
done

echo "✅ Todas as simulações e conversões JSON foram concluídas."
