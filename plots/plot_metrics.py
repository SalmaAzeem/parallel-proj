import pandas as pd
import matplotlib.pyplot as plt
import os

# Ensure the plots directory exists
if not os.path.exists("plots"):
    os.makedirs("plots")

# 1. Load the data
csv_path = "data/metrics_log.csv"
if not os.path.exists(csv_path):
    print(f"Error: {csv_path} not found. Run the automation script first.")
    exit()

df = pd.read_csv(csv_path)

# Set style for professional-looking plots
plt.style.use("seaborn-v0_8-muted")

# 2. Create Latency vs. Time Plot
plt.figure(figsize=(10, 6))
plt.plot(df['Timestamp(s)'], df['Latency(ms)'], label='Latency per Request', alpha=0.6)

# Annotate likely failure/recovery points based on project requirements
plt.axvline(x=30, color='r', linestyle='--', label='Failure Injection (t=30s)')
plt.axvline(x=60, color='g', linestyle='--', label='Recovery Trigger (t=60s)')

plt.title("Latency vs. Time (Resilience Analysis)")
plt.xlabel("Time (seconds)")
plt.ylabel("Latency (ms)")
plt.legend()
plt.grid(True, which='both', linestyle='--', alpha=0.5)
plt.savefig("plots/latency_vs_time.png", dpi=300)
print("Generated: plots/latency_vs_time.png")

# 3. Calculate and Plot Throughput vs. Time
# Round timestamps to nearest second to group them for throughput
df['Second'] = df['Timestamp(s)'].round()
throughput_df = df[df['Success'] == 1].groupby('Second').size().reset_index(name='RequestsPerSec')

plt.figure(figsize=(10, 6))
plt.step(throughput_df['Second'], throughput_df['RequestsPerSec'], where='post', 
         label='Throughput (Successful Req/s)', color='teal', linewidth=2)

plt.axvline(x=30, color='r', linestyle='--', label='Failure Point')
plt.axvline(x=60, color='g', linestyle='--', label='Recovery Point')

plt.title("Throughput vs. Time")
plt.xlabel("Time (seconds)")
plt.ylabel("Throughput (Req/sec)")
plt.legend()
plt.grid(True, which='both', linestyle='--', alpha=0.5)
plt.savefig("plots/throughput_vs_time.png", dpi=300)
print("Generated: plots/throughput_vs_time.png")

# 4. Summary Statistics for Report
print("\n--- Mandatory Metrics Summary ---")
baseline = df[df['Timestamp(s)'] < 30]
failure_period = df[(df['Timestamp(s)'] >= 30) & (df['Timestamp(s)'] < 60)]

print(f"Baseline Latency (p95): {baseline['Latency(ms)'].quantile(0.95):.2f} ms")
print(f"Failure Latency (p95):  {failure_period['Latency(ms)'].quantile(0.95):.2f} ms")
print(f"Recovery Time:          Observed between t=60s and stabilized throughput.")