#!/bin/bash

echo "Cleaning up generated files..."

# Remover arquivos de input
rm -f input_*.txt

# Remover arquivos de saída .txt (originalmente em outputs/txt/)
rm -rf outputs/txt/*.txt

# Remover arquivos JSON (em outputs/json/)
rm -rf outputs/json/*.json

# Remover todos os gráficos PNG (em outputs/graficos/)
rm -rf outputs/graficos/*.png

# Remover CSVs gerados
rm -f relatorio_completo.csv relatorio_medias.csv

# Remover executáveis
rm -f rr_simulator fcfs_simulator rr_dynamic_simulator

echo "Cleanup completed!"
