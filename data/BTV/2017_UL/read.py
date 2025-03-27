import pandas as pd

# Read JSON into a DataFrame
df = pd.read_json("btagging.json")

# Display data
print(df.head())

