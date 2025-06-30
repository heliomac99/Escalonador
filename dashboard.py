import json
import os
import pandas as pd
import matplotlib.pyplot as plt
import glob

# Diretórios onde estão armazenados os JSONs
JSON_DIR = "outputs/json"
# Diretório para salvar gráficos
GRAFICOS_DIR = "outputs/graficos"

# Buscar todos os arquivos JSON disponíveis
fcfs_files = sorted(glob.glob(os.path.join(JSON_DIR, "fcfs_simulator_output_*.json")))
rr_files = sorted(glob.glob(os.path.join(JSON_DIR, "rr_simulator_output_*.json")))
rrd_files = sorted(glob.glob(os.path.join(JSON_DIR, "rr_dynamic_simulator_output_*.json")))

# Extrair os IDs únicos a partir dos nomes de arquivo
ids = sorted(set(
    int(os.path.basename(f).split("_")[-1].split(".")[0])
    for f in fcfs_files + rr_files + rrd_files
))

todos_dados = []

for id in ids:
    try:
        with open(os.path.join(JSON_DIR, f"fcfs_simulator_output_{id}.json")) as f1, \
             open(os.path.join(JSON_DIR, f"rr_simulator_output_{id}.json")) as f2, \
             open(os.path.join(JSON_DIR, f"rr_dynamic_simulator_output_{id}.json")) as f3:

            fcfs_data = json.load(f1)
            rr_data   = json.load(f2)
            rrd_data  = json.load(f3)

            if not fcfs_data or not rr_data or not rrd_data:
                print(f"[!] Input {id} com dados incompletos ou vazios. Pulando.")
                continue

            for proc_fcfs, proc_rr, proc_rrd in zip(fcfs_data, rr_data, rrd_data):
                todos_dados.append({
                    "ID": id,
                    "Algoritmo": "FCFS",
                    "WaitingTime": int(proc_fcfs.get("Waiting", 0)),
                    "Preempcoes": 0,
                    "Turnaround": int(proc_fcfs.get("Turnaround", 0))
                })
                todos_dados.append({
                    "ID": id,
                    "Algoritmo": "RR",
                    "WaitingTime": int(proc_rr.get("Waiting", 0)),
                    "Preempcoes": int(proc_rr.get("Preemptions", proc_rr.get("Preempcoes", 0))),
                    "Turnaround": int(proc_rr.get("Turnaround", 0))
                })
                todos_dados.append({
                    "ID": id,
                    "Algoritmo": "RR Dinâmico",
                    "WaitingTime": int(proc_rrd.get("Waiting", 0)),
                    "Preempcoes": int(proc_rrd.get("Preemptions", proc_rrd.get("Preempcoes", 0))),
                    "Turnaround": int(proc_rrd.get("Turnaround", 0))
                })

    except FileNotFoundError:
        print(f"[!] Arquivo faltando para o input {id}. Pulando...")

# Montar DataFrame
if todos_dados:
    df = pd.DataFrame(todos_dados)
else:
    df = pd.DataFrame()

print("\nColunas detectadas:", df.columns if not df.empty else "nenhuma")
print("\nConteúdo do DataFrame:")
print(df)

# Converter colunas para numérico
if not df.empty:
    if all(col in df.columns for col in ["WaitingTime", "Preempcoes", "Turnaround"]):
        df[["WaitingTime", "Preempcoes", "Turnaround"]] = \
            df[["WaitingTime", "Preempcoes", "Turnaround"]].apply(pd.to_numeric, errors="coerce")
    else:
        print("[!] As colunas esperadas não estão presentes no DataFrame.")
else:
    print("[!] DataFrame vazio. Nenhum dado foi carregado.")

# Salvar CSV detalhado (por processo)
if not df.empty:
    df.to_csv("relatorio_completo.csv", index=False)

    # Calcular médias por (Input, Algoritmo)
    medias_det = df.groupby(["ID", "Algoritmo"])[["Preempcoes", "Turnaround", "WaitingTime"]].mean().reset_index()
    medias_det.rename(columns={
        "Preempcoes": "Preempcoes_Media",
        "Turnaround": "Turnaround_Medio",
        "WaitingTime": "WaitingTime_Medio"
    }, inplace=True)

    print("\nMétricas médias por Input e Algoritmo:")
    print(medias_det)

    # Salvar CSV com médias detalhadas
    medias_det.to_csv("relatorio_medias_por_algoritmo.csv", index=False)

    # Criar pasta de gráficos
    os.makedirs(GRAFICOS_DIR, exist_ok=True)

    # Gerar gráficos de médias por algoritmo para cada input
    metricas = ["WaitingTime", "Preempcoes", "Turnaround"]
    for metrica in metricas:
        for id in ids:
            subset = df[df["ID"] == id]
            pivot = subset.pivot_table(index="Algoritmo", values=metrica, aggfunc="mean")

            if pivot.dropna().empty:
                print(f"[!] Sem dados válidos para {metrica} - Input {id}. Pulando gráfico.")
                continue

            ax = pivot.plot(
                kind="bar",
                title=f"{metrica} Médio por Algoritmo - Input {id}",
                figsize=(8, 5)
            )
            plt.ylabel(metrica)
            plt.xlabel("Algoritmo")
            plt.tight_layout()
            plt.grid(True)

            for container in ax.containers:
                ax.bar_label(container, fmt='%d', padding=3)

            plt.savefig(os.path.join(GRAFICOS_DIR, f"grafico_{metrica.lower()}_input_{id}.png"))
            plt.close()

    print("\n[✓] Relatório CSV e gráficos gerados com sucesso.")
else:
    # Se DataFrame vazio, apenas garantir que pasta de gráficos exista
    os.makedirs(GRAFICOS_DIR, exist_ok=True)
    print("\nNenhum dado para gerar relatórios.")
