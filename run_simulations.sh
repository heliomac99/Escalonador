#!/bin/bash

# --- Entradas diferentes ---
# --- Entradas diferentes ---
# --- Entradas diferentes ---
INPUTS=(
# 5 processos — números 1-200
"5
137 42 158 63 32
98 176 57 199 10
12 88 143 117 22
185 29 61 154 11
72 134 96 10 199"

# 10 processos — números 1-200
"10
144 7 123 55 1
31 187 192 68 10
199 84 46 173 78
63 57 179 15 90
9 158 22 120 23
118 143 101 198 199
175 30 167 92 23
88 196 35 161 3
142 66 186 27 13
56 110 78 139 161"

# 15 processos — números 1-200
"15
121 47 138 91 23
4 162 180 66 3 
189 109 51 130 4 
57 11 197 145 5
32 123 95 187 6
173 70 28 101 90
140 199 75 20 3
93 182 64 41 12
154 36 118 178 34
24 160 131 83 199
65 173 97 10 23
186 58 149 134 76
117 8 168 45 79
38 127 55 156 41
102 94 184 71 34"

# 20 processos — números 1-200
"20
13 159 56 178 42
194 34 132 60 57
81 197 25 122 97
169 11 188 93 124
47 72 145 170 94
158 63 84 137 10
120 186 39 5 34
96 112 174 44 85
183 20 68 151 27
27 99 190 65 75
141 52 119 16 42
6 171 48 129 58
175 86 33 115 111
70 142 104 198 90
155 91 167 29 23
53 124 14 161 32
100 35 181 82 11
59 147 126 107 32
129 2 109 140 33
176 80 96 38 44"
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
