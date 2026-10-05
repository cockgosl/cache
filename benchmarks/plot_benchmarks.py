import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

def create_plots():
    csv_path = 'benchmarks/benchmarks.csv'
    out_dir = 'benchmarks/plots'

    if not os.path.exists(csv_path):
        print(f"Error: {csv_path} not found.")
        return

    os.makedirs(out_dir, exist_ok=True)
    df = pd.read_csv(csv_path)

    sns.set_theme(style="whitegrid")

    # 1. Сравнение Single Caches
    single_df = df[df['Mode'] == 'Single']
    plt.figure(figsize=(12, 6))
    sns.barplot(data=single_df, x='Workload', y='Hit_Ratio_Pct', hue='Algorithm')
    plt.title('Single Cache Hit Ratio by Workload')
    plt.ylabel('Hit Ratio (%)')
    plt.xticks(rotation=45)
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, 'single_cache_comparison.png'))
    plt.close()

    # 2. Сравнение Multi Caches (Exclusive vs Inclusive)
    multi_df = df[df['Mode'] != 'Single'].copy()
    multi_df['Algo_Mode'] = multi_df['Algorithm'] + " (" + multi_df['Mode'] + ")"

    workloads = multi_df['Workload'].unique()
    for wl in workloads:
        wl_df = multi_df[multi_df['Workload'] == wl]
        plt.figure(figsize=(14, 7))
        sns.barplot(data=wl_df, x='Algorithm', y='Hit_Ratio_Pct', hue='Mode', palette='viridis')
        plt.title(f'Multi-Level Cache Hit Ratio: {wl} (L1=10, L2=20)')
        plt.ylabel('Hit Ratio (%)')
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig(os.path.join(out_dir, f'multi_cache_{wl.lower()}.png'))
        plt.close()

    print(f"Plots successfully generated and saved to {out_dir}/")

if __name__ == "__main__":
    create_plots()
