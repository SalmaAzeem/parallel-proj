import pandas as pd
import matplotlib.pyplot as plt
import os

# Ensure the plots directory exists
if not os.path.exists("plots"):
    os.makedirs("plots")

# 1. Load the data
csv_path = "data/metrics_log.csv"
if not os.path.exists(csv_path):
    print(f"Error: {csv_path} not found.Run the automation script first.")
    exit()

# C++ Columns: Timestamp, Status, Latency_ms, Condition
df = pd.read_csv(csv_path, names=['Timestamp', 'Status', 'Latency_ms', 'Condition'])

# Convert Timestamp and calculate relative time
df['Timestamp'] = pd.to_datetime(df['Timestamp'])
start_time = df['Timestamp'].min()
df['TimeDelta'] = (df['Timestamp'] - start_time).dt.total_seconds()

# Set professional style
plt.style.use("seaborn-v0_8-muted")

# --- 2. Create Latency Plot ---
plt.figure(figsize=(12, 6))

# Plot the actual data points
plt.scatter(df['TimeDelta'], df['Latency_ms'], 
            c=df['Status'].map({'SUCCESS': 'blue', 'FAILURE': 'red'}), 
            alpha=0.5, s=15, label='Requests')

# --- 3. ADD THE THREE CRITICAL LINES ---

# Line 1: Network Disruption (Likely when Condition changes to DISRUPTED)
disrupt_start = df[df['Condition'] == 'DISRUPTED']['TimeDelta'].min()
if pd.notnull(disrupt_start):
    plt.axvline(x=disrupt_start, color='orange', linestyle='--', linewidth=2)
    plt.text(disrupt_start, plt.ylim()[1]*0.9, ' Network Disruption', color='orange', fontweight='bold')

# Line 2: Failure Injected (When status first turns to FAILURE)
failure_start = df[df['Status'] == 'FAILURE']['TimeDelta'].min()
if pd.notnull(failure_start):
    plt.axvline(x=failure_start, color='red', linestyle='--', linewidth=2)
    plt.text(failure_start, plt.ylim()[1]*0.7, ' Failure Injected', color='red', fontweight='bold')

# Line 3: Recovery (First success AFTER a failure)
if pd.notnull(failure_start):
    recovery_point = df[(df['TimeDelta'] > failure_start) & (df['Status'] == 'SUCCESS')]['TimeDelta'].min()
    if pd.notnull(recovery_point):
        plt.axvline(x=recovery_point, color='green', linestyle='-', linewidth=2)
        plt.text(recovery_point, plt.ylim()[1]*0.5, ' Recovery Point', color='green', fontweight='bold')

plt.title("Fractal Service Resilience Analysis")
plt.xlabel("Time (seconds)")
plt.ylabel("Latency (ms)")
plt.grid(True, which='both', linestyle='--', alpha=0.5)
plt.tight_layout()
plt.savefig("plots/latency_resilience.png", dpi=300)
print("Generated: plots/latency_resilience.png with 3 event lines.")

# --- 4. Summary Statistics ---
print("\n--- Mandatory Resilience Metrics ---")
if pd.notnull(disrupt_start) and pd.notnull(failure_start):
    disruption_zone = df[(df['TimeDelta'] >= disrupt_start) & (df['TimeDelta'] < failure_start)]
    print(f"Avg Latency during Network Disruption: {disruption_zone['Latency_ms'].mean():.2f} ms")

if pd.notnull(failure_start) and pd.notnull(recovery_point):
    recovery_time = recovery_point - failure_start
    print(f"System Recovery Time (MTTR): {recovery_time:.2f} seconds")