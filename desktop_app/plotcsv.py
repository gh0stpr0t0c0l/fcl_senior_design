import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("flightDump.csv")

df["Time(us)"] = pd.to_datetime(df["Time(us)"])
df.set_index("Time(us)", inplace=True)

# Create one subplot per column
df.plot(subplots=True, layout=(6, 1), figsize=(10, 12), sharex=True)

plt.tight_layout()
plt.show()