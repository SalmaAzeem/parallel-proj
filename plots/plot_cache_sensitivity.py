import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("data/results.csv")

schedules = df['Schedule'].unique()
image_sizes = sorted(df['ImageSize'].unique())
thread_counts = sorted(df['Threads'].unique())

plt.figure(figsize=(8, 6))
for sched in schedules:
    data = df[(df['Schedule'] == sched) & (df['Threads'] == 8)]
    plt.plot(data['ImageSize'], data['Speedup'], marker='^', label=f'{sched} (8 threads)')
plt.xlabel("Image Size")
plt.ylabel("Speedup")
plt.title("Speedup vs Image Size (Cache Sensitivity Trend)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

plt.figure(figsize=(8, 6))
for sched in schedules:
    data = df[(df['Schedule'] == sched) & (df['Threads'] == 8)]
    plt.plot(data['ImageSize'], data['Efficiency'], marker='d', label=f'{sched} (8 threads)')
plt.xlabel("Image Size")
plt.ylabel("Efficiency (%)")
plt.title("Efficiency vs Image Size (Cache Sensitivity Trend)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
