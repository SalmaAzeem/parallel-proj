import pandas as pd
import matplotlib.pyplot as plt
import os

# Ensure the plots directory exists
if not os.path.exists("plots"):
    os.makedirs("plots")

# 1. Load the data
csv_path = "data/metrics_log.csv"
if not os.path.exists(csv_path):
    print(f"Error: {csv_path} not found.")
    exit()

# Load CSV and clean columns
df = pd.read_csv(csv_path)
df.columns = df.columns.str.strip()

# Convert Timestamp and calculate relative time in seconds
df['Timestamp'] = pd.to_datetime(df['Timestamp'])
start_time = df['Timestamp'].min()
df['TimeDelta'] = (df['Timestamp'] - start_time).dt.total_seconds()

# Identify critical event timestamps
# Network Disruption: When the condition first becomes 'DISRUPTED'
disrupt_start = df[df['Condition'] == 'DISRUPTED']['TimeDelta'].min()

# Service Stop: When the status first becomes 'FAILURE'
stop_start = df[df['Status'] == 'FAILURE']['TimeDelta'].min()

# Recovery Point: The first 'SUCCESS' after the service was stopped
recovery_point = None
if pd.notnull(stop_start):
    recovery_point = df[(df['TimeDelta'] > stop_start) & (df['Status'] == 'SUCCESS')]['TimeDelta'].min()

plt.style.use("seaborn-v0_8-muted")

def apply_annotations(ax):
    """Adds labeled vertical lines for each failure stage."""
    ylim = ax.get_ylim()[1]
    
    if pd.notnull(disrupt_start):
        ax.axvline(x=disrupt_start, color='orange', linestyle='--', linewidth=1.5)
        ax.text(disrupt_start, ylim * 0.9, ' Network Disruption', color='orange', fontweight='bold')
    
    if pd.notnull(stop_start):
        ax.axvline(x=stop_start, color='red', linestyle='--', linewidth=1.5)
        ax.text(stop_start, ylim * 0.7, ' Worker Stopped', color='red', fontweight='bold')
        
    if pd.notnull(recovery_point):
        ax.axvline(x=recovery_point, color='green', linestyle='-', linewidth=2)
        ax.text(recovery_point, ylim * 0.5, ' Recovery Point', color='green', fontweight='bold')

# --- Graph 1: Latency vs. Time ---
plt.figure(figsize=(12, 6))
plt.scatter(df['TimeDelta'], df['Latency_ms'], 
            c=df['Status'].map({'SUCCESS': 'royalblue', 'FAILURE': 'red'}), 
            alpha=0.6, s=20, label='Requests (Blue=Success, Red=Fail)')

apply_annotations(plt.gca())
plt.title("Latency vs. Time (Resilience Analysis)")
plt.xlabel("Time (seconds)")
plt.ylabel("Latency (ms)")
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend(loc='upper right')
plt.savefig("latency_vs_time.png", dpi=300)
print("Generated: plots/latency_vs_time.png")

# --- Graph 2: Throughput vs. Time ---
# Group successful requests into 1-second bins
df['Second'] = df['TimeDelta'].astype(int)
throughput = df[df['Status'] == 'SUCCESS'].groupby('Second').size()

plt.figure(figsize=(12, 6))
plt.plot(throughput.index, throughput.values, color='teal', linewidth=2, label='Successful Req/sec')
plt.fill_between(throughput.index, throughput.values, color='teal', alpha=0.1)

apply_annotations(plt.gca())
plt.title("Throughput vs. Time")
plt.xlabel("Time (seconds)")
plt.ylabel("Throughput (Req/sec)")
plt.grid(True, linestyle=':', alpha=0.6)
plt.legend(loc='upper right')
plt.savefig("throughput_vs_time.png", dpi=300)
print("Generated: plots/throughput_vs_time.png")