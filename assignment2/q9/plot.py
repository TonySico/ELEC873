import numpy as np
import matplotlib.pyplot as plt

# Load CSV
data = np.loadtxt("results.csv", delimiter=",")
msg_size = data[:, 1]  # message size (bytes)
avg_time = data[:, 2] * 1000  # convert to milliseconds
bandwidth = data[:, 4]  # MB/s

# --- Least-squares linear fit: T = α + βn ---
# Fit still in seconds, so we scale accordingly
A = np.vstack([msg_size, np.ones(len(msg_size))]).T
beta_sec, alpha_sec = np.linalg.lstsq(A, data[:, 2], rcond=None)[0]

# Convert fitted line to milliseconds
alpha = alpha_sec * 1000
beta = beta_sec * 1000
projected_time = alpha + beta * msg_size

print(f"Startup time (α): {alpha:.3f} ms")
print(f"Per-byte time (β): {beta:.6f} ms/byte")

# --- Plot time vs message size ---
plt.figure(figsize=(8, 5))
plt.plot(msg_size, avg_time, "o", label="Measured")
plt.plot(msg_size, projected_time, "r--", label="Fitted (least-squares)")
plt.xscale("log", base=2)
plt.xlabel("Message size (bytes, log scale)")
plt.ylabel("Round-trip time (ms)")
plt.title("Message Time vs Size (MPI_Send/MPI_Recv)")
plt.legend()
plt.grid(True, which="both", ls="--")
plt.tight_layout()
plt.savefig("../report/images/q9_rtt.png", dpi=300)
plt.show()

# --- Plot bandwidth vs message size ---
plt.figure(figsize=(8, 5))
plt.plot(msg_size, bandwidth, "o-", label="Measured Bandwidth")
plt.xscale("log", base=2)
plt.xlabel("Message size (bytes, log scale)")
plt.ylabel("Bandwidth (MB/s)")
plt.title("Bandwidth vs Message Size (MPI_Send/MPI_Recv)")
plt.grid(True, which="both", ls="--")
plt.tight_layout()
plt.savefig("../report/images/q9_band.png", dpi=300)
plt.show()
