# Anthony Sicoie (20214793)

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

static = pd.read_csv("static.csv", header=None, names=["processes", "datacount", "time_init", "time_no_init"])


plt.figure(figsize=(8, 6))

# Plot dynamic and static
plt.plot(static["datacount"], static["time_init"], marker="o", label="Static")
plt.plot(static["datacount"], static["time_no_init"], marker="o", label="Static")


# Log2 x-scale and labels
plt.xscale("log", base=2)
plt.xlabel("Data Size (B)")
plt.ylabel("Time (s)")
plt.title("Time  MPI Performance")
plt.legend()
plt.grid(True, which="both", ls="--", lw=0.5)

plt.tight_layout()

plt.savefig("../report/images/q8.png", dpi=300)

plt.show()
