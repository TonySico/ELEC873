# Anthony Sicoie (20214793)

import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv(
    "results.csv", header=None, names=["processes", "threads", "array_size", "avg_time"]
)

df["time_ms"] = df["avg_time"] * 1000

# Plot avg_time vs array_size for each (processes, threads) configuration
for (p, t), subset in df.groupby(["processes", "threads"]):
    plt.plot(
        subset["array_size"],
        subset["time_ms"],
        marker="o",
        label=f"{p} proc, {t} threads",
    )

plt.xlabel("Array Size")
plt.ylabel("Average Time (ms)")
plt.title("Local: Hybrid MPI+OpenMP Performance")
plt.legend()
plt.grid(True)
plt.yscale("log", base=2)
plt.xscale("log", base=2)
plt.tight_layout()
plt.savefig("../report/images/q7.png", dpi=300)
plt.show()
