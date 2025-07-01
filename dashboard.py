import json
import os
import pandas as pd
import matplotlib.pyplot as plt
import glob

# Diretórios
JSON_DIR = "outputs/json"
GRAFICOS_DIR = "outputs/graficos"
os.makedirs(GRAFICOS_DIR, exist_ok=True)

# Arquivos JSON
fcfs_files = sorted(glob.glob(os.path.join(JSON_DIR, "fcfs_simulator_output_*.json")))
rr_files   = sorted(glob.glob(os.path.join(JSON_DIR, "rr_simulator_output_*.json")))
rrd_files  = sorted(glob.glob(os.path.join(JSON_DIR, "rr_dynamic_simulator_output_*.json")))

# IDs únicos
ids = sorted(set(
    int(os.path.basename(f).split("_")[-1].split(".")[0])
    for f in fcfs_files + rr_files + rrd_files
))

todos_dados = []
for id in ids:
    try:
        with open(f"{JSON_DIR}/fcfs_simulator_output_{id}.json") as f1, \
             open(f"{JSON_DIR}/rr_simulator_output_{id}.json")   as f2, \
             open(f"{JSON_DIR}/rr_dynamic_simulator_output_{id}.json") as f3:
            d1, d2, d3 = json.load(f1), json.load(f2), json.load(f3)
            for proc_fcfs, proc_rr, proc_rrd in zip(d1, d2, d3):
                for alg, proc in [("FCFS", proc_fcfs), ("RR", proc_rr), ("RR Dinâmico", proc_rrd)]:
                    todos_dados.append({
                        "ID": id,
                        "Algoritmo": alg,
                        "WaitingTime": int(proc.get("Waiting", 0)),
                        "Preempcoes": int(proc.get("Preemptions", proc.get("Preempcoes", 0))) if alg != "FCFS" else 0,
                        "Turnaround": int(proc.get("Turnaround", 0)),
                        "Priority": int(proc.get("Priority", 0))
                    })
    except FileNotFoundError:
        continue

# DataFrame
if todos_dados:
    df = pd.DataFrame(todos_dados)
else:
    df = pd.DataFrame()

# Converter tipos
for col in ["WaitingTime", "Preempcoes", "Turnaround", "Priority"]:
    if col in df.columns:
        df[col] = pd.to_numeric(df[col], errors="coerce")

if df.empty:
    print("Nenhum dado para gerar relatórios.")
    exit()

# CSV completo
df.to_csv("relatorio_completo.csv", index=False)

# Médias por Input e Algoritmo
medias = (
    df.groupby(["ID", "Algoritmo"])[["WaitingTime", "Preempcoes", "Turnaround"]]
      .mean()
      .reset_index()
)
medias.to_csv("relatorio_medias_por_algoritmo.csv", index=False)

# 1. Gráficos originais (barras com rótulos)
for metrica in ["WaitingTime", "Preempcoes", "Turnaround"]:
    for id in ids:
        subset = df[df["ID"] == id]
        pivot = subset.pivot_table(index="Algoritmo", values=metrica, aggfunc="mean")
        if pivot.empty: continue
        ax = pivot.plot(kind="bar", figsize=(8,5), title=f"{metrica} Médio - Input {id}")
        ax.set_ylabel(metrica)
        ax.set_xlabel("Algoritmo")
        ax.grid(True)
        # adicionar rótulos de valor
        for container in ax.containers:
            ax.bar_label(container, fmt='%d', padding=3)
        plt.tight_layout()
        plt.savefig(os.path.join(GRAFICOS_DIR, f"media_{metrica.lower()}_input_{id}.png"))
        plt.close()

# 2. Scatter Turnaround vs Priority
plt.figure(figsize=(8,5))
for alg in df["Algoritmo"].unique():
    grp = df[df["Algoritmo"] == alg]
    plt.scatter(grp["Priority"], grp["Turnaround"], label=alg)
plt.title("Turnaround vs Priority")
plt.xlabel("Priority")
plt.ylabel("Turnaround")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(GRAFICOS_DIR, "turnaround_vs_priority.png"))
plt.close()

# 3. Scatter WaitingTime vs Priority
plt.figure(figsize=(8,5))
for alg in df["Algoritmo"].unique():
    grp = df[df["Algoritmo"] == alg]
    plt.scatter(grp["Priority"], grp["WaitingTime"], label=alg)
plt.title("Waiting Time vs Priority")
plt.xlabel("Priority")
plt.ylabel("Waiting Time")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig(os.path.join(GRAFICOS_DIR, "waiting_vs_priority.png"))
plt.close()

# 4. Histograma WaitingTime
plt.figure(figsize=(8,5))
for alg in df["Algoritmo"].unique():
    plt.hist(df[df["Algoritmo"] == alg]["WaitingTime"], bins=20, alpha=0.5, label=alg)
plt.title("Distribuição de Waiting Time")
plt.xlabel("Waiting Time")
plt.ylabel("Frequência")
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(GRAFICOS_DIR, "hist_waiting_distribution.png"))
plt.close()

# 5. Histograma Turnaround
plt.figure(figsize=(8,5))
for alg in df["Algoritmo"].unique():
    plt.hist(df[df["Algoritmo"] == alg]["Turnaround"], bins=20, alpha=0.5, label=alg)
plt.title("Distribuição de Turnaround Time")
plt.xlabel("Turnaround Time")
plt.ylabel("Frequência")
plt.legend()
plt.tight_layout()
plt.savefig(os.path.join(GRAFICOS_DIR, "hist_turnaround_distribution.png"))
plt.close()

# 6. Boxplot Preempções
plt.figure(figsize=(8,5))
df.boxplot(column="Preempcoes", by="Algoritmo")
plt.title("Distribuição de Preempções por Algoritmo")
plt.suptitle("")
plt.ylabel("Preempções")
plt.tight_layout()
plt.savefig(os.path.join(GRAFICOS_DIR, "box_preemptions.png"))
plt.close()

# 7. Jain's Fairness (Waiting) - barras com rótulos
fairness = []
for alg in df["Algoritmo"].unique():
    w = df[df["Algoritmo"] == alg]["WaitingTime"].values
    if len(w) > 0:
        idx = w.sum()**2 / (len(w) * (w**2).sum())
        fairness.append({"Algoritmo": alg, "Jain_Waiting": idx})
fair_df = pd.DataFrame(fairness)
ax = fair_df.plot(kind="bar", x="Algoritmo", y="Jain_Waiting", figsize=(6,4), legend=False)
plt.title("Jain's Fairness (Waiting)")
plt.ylabel("Fairness Index")
for container in ax.containers:
    ax.bar_label(container, fmt='%.2f', padding=3)
plt.tight_layout()
plt.savefig(os.path.join(GRAFICOS_DIR, "jain_waiting.png"))
plt.close()

# 8 & 9. Média Ponderada por Input e Algoritmo
for id in ids:
    sub = df[df["ID"] == id]
    if sub.empty: continue
    # Turnaround ponderado
    wt = sub.groupby("Algoritmo").apply(lambda g: (g["Turnaround"] * g["Priority"]).sum() / g["Priority"].sum())
    wt = wt.reset_index(name="W_Turnaround")
    ax = wt.plot(kind="bar", x="Algoritmo", y="W_Turnaround", figsize=(6,4), legend=False,
                 title=f"Turnaround Médio Ponderado - Input {id}")
    plt.ylabel("Valor")
    for container in ax.containers:
        ax.bar_label(container, fmt='%.2f', padding=3)
    plt.tight_layout()
    plt.savefig(os.path.join(GRAFICOS_DIR, f"ponderado_turnaround_input_{id}.png"))
    plt.close()

    # WaitingTime ponderado
    ww = sub.groupby("Algoritmo").apply(lambda g: (g["WaitingTime"] * g["Priority"]).sum() / g["Priority"].sum())
    ww = ww.reset_index(name="W_WaitingTime")
    ax = ww.plot(kind="bar", x="Algoritmo", y="W_WaitingTime", figsize=(6,4), legend=False,
                 title=f"Waiting Time Médio Ponderado - Input {id}")
    plt.ylabel("Valor")
    for container in ax.containers:
        ax.bar_label(container, fmt='%.2f', padding=3)
    plt.tight_layout()
    plt.savefig(os.path.join(GRAFICOS_DIR, f"ponderado_waiting_input_{id}.png"))
    plt.close()

print("[✓] Todos os gráficos gerados.")
