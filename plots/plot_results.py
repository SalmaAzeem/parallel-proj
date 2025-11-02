import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("data/results.csv")
schedules = df["Schedule"].unique()

plt.style.use("seaborn-v0_8-colorblind")

# time comparison with sequential
plt.figure(figsize=(8,5))
for sched in schedules:
    subset = df[df["Schedule"] == sched]
    plt.plot(subset["Threads"], subset["Sequential"], '--', label=f'{sched} Sequential (constant)')
    plt.plot(subset["Threads"], subset["Parallel"], 'o-', label=f'{sched} Parallel')
plt.title("Sequential vs Parallel Execution Time")
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (s)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("plots/time_comparison.png", dpi=300)
plt.show()

# speedup
plt.figure(figsize=(8,5))
for sched in schedules:
    subset = df[df["Schedule"] == sched]
    plt.plot(subset["Threads"], subset["Speedup"], marker='o', label=sched)
plt.plot([1, max(df["Threads"])], [1, max(df["Threads"])], 'k--', label="Ideal Speedup")
plt.title("Speedup vs Number of Threads")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup (Sequential / Parallel)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("plots/speedup.png", dpi=300)
plt.show()

# efficiency 
plt.figure(figsize=(8,5))
for sched in schedules:
    subset = df[df["Schedule"] == sched]
    plt.plot(subset["Threads"], subset["Efficiency"], marker='o', label=sched)
plt.title("Efficiency vs Number of Threads")
plt.xlabel("Number of Threads")
plt.ylabel("Efficiency (%)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("plots/efficiency.png", dpi=300)
plt.show()

# time comparison
plt.figure(figsize=(8,5))
for sched in schedules:
    subset = df[df["Schedule"] == sched]
    plt.plot(subset["Threads"], subset["Parallel"], marker='o', label=sched)
plt.title("Parallel Execution Time by Schedule")
plt.xlabel("Number of Threads")
plt.ylabel("Parallel Time (s)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("plots/parallel_time.png", dpi=300)
plt.show()

# speedup vs efficiency
plt.figure(figsize=(7,5))
for sched in schedules:
    subset = df[df["Schedule"] == sched]
    plt.scatter(subset["Speedup"], subset["Efficiency"], label=sched, s=70)
plt.title("Speedup vs Efficiency")
plt.xlabel("Speedup")
plt.ylabel("Efficiency (%)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("plots/speedup_vs_efficiency.png", dpi=300)
plt.show()
