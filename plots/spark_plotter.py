import pandas as pd
import matplotlib.pyplot as plt
import argparse
import os

def load_metrics(csv_path: str) -> pd.DataFrame:
    df = pd.read_csv(csv_path)
    df['latency_ms'] = df['latency_ms'].astype(float)
    df['calc_time_ms'] = df['calc_time_ms'].astype(float)
    df['success'] = df['success'].astype(int)
    return df

def plot_metrics(df: pd.DataFrame, output_dir: str):
    os.makedirs(output_dir, exist_ok=True)
    workers = df['worker'].unique()
    colors = plt.cm.tab10.colors
    
    fig, ax = plt.subplots(figsize=(12, 5))
    for i, worker in enumerate(workers):
        worker_df = df[df['worker'] == worker].sort_values('frame_id')
        ax.plot(worker_df['frame_id'], worker_df['latency_ms'], 
                marker='o', markersize=4, label=worker, color=colors[i % len(colors)])
    ax.set_xlabel('Frame ID')
    ax.set_ylabel('Latency (ms)')
    ax.set_title('Round-Trip Latency per Frame (by Worker)')
    ax.legend(title='Worker')
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/latency_per_frame.png', dpi=150)
    plt.close()
    
    fig, ax = plt.subplots(figsize=(12, 5))
    for i, worker in enumerate(workers):
        worker_df = df[df['worker'] == worker].sort_values('frame_id')
        ax.plot(worker_df['frame_id'], worker_df['calc_time_ms'], 
                marker='o', markersize=4, label=worker, color=colors[i % len(colors)])
    ax.set_xlabel('Frame ID')
    ax.set_ylabel('Calculation Time (ms)')
    ax.set_title('Server-Side Calculation Time per Frame (by Worker)')
    ax.legend(title='Worker')
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(f'{output_dir}/calc_time_per_frame.png', dpi=150)
    plt.close()
    
    summary = df.groupby('worker').agg({
        'latency_ms': ['mean', 'std', 'min', 'max'],
        'calc_time_ms': ['mean', 'std'],
        'frame_id': 'count'
    }).round(2)
    summary.columns = ['avg_latency', 'std_latency', 'min_latency', 'max_latency',
                       'avg_calc_time', 'std_calc_time', 'request_count']
    
    fig, axes = plt.subplots(1, 2, figsize=(12, 5))
    
    summary['avg_latency'].plot(kind='bar', ax=axes[0], color=colors[:len(workers)], edgecolor='black')
    axes[0].set_ylabel('Avg Latency (ms)')
    axes[0].set_title('Average Round-Trip Latency per Worker')
    axes[0].set_xticklabels(summary.index, rotation=45, ha='right')
    
    summary['avg_calc_time'].plot(kind='bar', ax=axes[1], color=colors[:len(workers)], edgecolor='black')
    axes[1].set_ylabel('Avg Calc Time (ms)')
    axes[1].set_title('Average Calculation Time per Worker')
    axes[1].set_xticklabels(summary.index, rotation=45, ha='right')
    
    plt.tight_layout()
    plt.savefig(f'{output_dir}/worker_comparison.png', dpi=150)
    plt.close()
    
    fig, ax = plt.subplots(figsize=(7, 7))
    summary['request_count'].plot(kind='pie', ax=ax, autopct='%1.1f%%', 
                                   colors=colors[:len(workers)], startangle=90)
    ax.set_ylabel('')
    ax.set_title('Request Distribution Across Workers')
    plt.tight_layout()
    plt.savefig(f'{output_dir}/request_distribution.png', dpi=150)
    plt.close()
    
    print(f"Plots saved to: {output_dir}/")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Plot Spark gRPC metrics by worker")
    parser.add_argument("--input", type=str, default="data/spark_metrics.csv",
                        help="Path to the metrics CSV file")
    parser.add_argument("--output", type=str, default="plots/spark",
                        help="Output directory for plots")
    
    args = parser.parse_args()
    
    df = load_metrics(args.input)
    plot_metrics(df, args.output)
