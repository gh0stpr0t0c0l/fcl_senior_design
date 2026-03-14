import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("flightDump.csv")

# Convert to numeric first (invalid values become NaN)
df["Time(us)"] = pd.to_numeric(df["Time(us)"], errors="coerce")

# Drop rows where conversion failed
df = df.dropna(subset=["Time(us)"])

# Convert microseconds → datetime
df["Time(us)"] = pd.to_datetime(df["Time(us)"], unit="us")

df.set_index("Time(us)", inplace=True)

df.plot(subplots=True, layout=(6,1), sharex=True)

plt.tight_layout()
plt.show()