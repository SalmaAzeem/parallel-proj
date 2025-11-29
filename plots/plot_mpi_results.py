import pandas as pd
import matplotlib.pyplot as plt
import os

if not os.path.exists("plots"):
    os.makedirs("plots")

plt.style.use("seaborn-v0_8-colorblind")

def plot_mpi_results(csv_path, prefix):
    if not os.path.exists(csv_path):
        print(f"{csv_path} not found.")
        return
    df = pd.read_csv(csv_path)
    sizes = df["ImageSize"].unique()

    # Speedup plot
    plt.figure(figsize=(8,5))
    for s in sizes:
        sub = df[df["ImageSize"] == s].sort_values("Threads")
        plt.plot(sub["Threads"], sub["Speedup"], 'o-', label=f"{s}x{s}")
    max_ranks = df["Threads"].max()
    plt.plot([1, max_ranks], [1, max_ranks], 'k--', label="Ideal")
    plt.title(f"MPI Speedup ({prefix})")
    plt.xlabel("Ranks")
    plt.ylabel("Speedup")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"plots/{prefix}_speedup.png", dpi=300)

    # Efficiency plot
    plt.figure(figsize=(8,5))
    for s in sizes:
        sub = df[df["ImageSize"] == s].sort_values("Threads")
        plt.plot(sub["Threads"], sub["Efficiency"], 'o-', label=f"{s}x{s}")
    plt.title(f"MPI Efficiency ({prefix})")
    plt.xlabel("Ranks")
    plt.ylabel("Efficiency (%)")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"plots/{prefix}_efficiency.png", dpi=300)

    # Execution time plot
    plt.figure(figsize=(8,5))
    for s in sizes:
        sub = df[df["ImageSize"] == s].sort_values("Threads")
        plt.plot(sub["Threads"], sub["Parallel"], 'o-', label=f"{s}x{s}")
    plt.title(f"MPI Execution Time ({prefix})")
    plt.xlabel("Ranks")
    plt.ylabel("Time (s)")
    plt.legend()
    plt.yscale("log")
    plt.grid(True)
    plt.savefig(f"plots/{prefix}_time.png", dpi=300)

    print(f"Plots generated for {csv_path} in plots/ directory.")

# Plot for fixed size
plot_mpi_results("data/mpi_fixedsize.csv", "fixedsize")
# Plot for scaling size
plot_mpi_results("data/mpi_scaledsize.csv", "scaledsize")