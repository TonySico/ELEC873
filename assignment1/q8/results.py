# Anthony Sicoie (20214793)

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load CSVs
# static = pd.read_csv("knlstatic.csv", header=None, names=["processes", "time"])
# dynamic = pd.read_csv("knldynamic.csv", header=None, names=["processes", "time"])
# seq = pd.read_csv("knlseq.csv", header=None, names=["processes", "time"])
static = pd.read_csv("static.csv", header=None, names=["processes", "time"])
dynamic = pd.read_csv("dynamic.csv", header=None, names=["processes", "time"])
seq = pd.read_csv("seq.csv", header=None, names=["processes", "time"])

# Get sequential time (assume only one row in seq.csv)
seq_time_ms = seq["time"].iloc[0]

plt.figure(figsize=(8, 6))

# Plot dynamic and static
plt.plot(static["processes"], static["time"], marker="o", label="Static")
plt.plot(dynamic["processes"], dynamic["time"], marker="s", label="Dynamic")

# Horizontal line for sequential
plt.axhline(
    y=seq_time_ms,
    color="r",
    linestyle="--",
    label=f"Baseline (Sequential) ({seq_time_ms:.3f} s)",
)

# Log2 x-scale and labels
plt.xscale("log", base=2)
plt.xlabel("Number of Processes")
plt.ylabel("Time (s)")
plt.title("Local: Static vs Dynamic vs Sequential MPI Performance")
plt.legend()
plt.grid(True, which="both", ls="--", lw=0.5)

plt.tight_layout()

plt.savefig("../report/images/q8.png", dpi=300)

plt.show()
