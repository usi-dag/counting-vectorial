import pandas as pd
import os

# Directory containing the results
directory = "./results"

# Dictionary to store data for each group
data_dict = {}

# Iterate over all files in the directory
for filename in os.listdir(directory):
    if filename.endswith(".csv"):
        # Extract the base part of the filename up to the second dot
        base_name = '.'.join(filename.split('.')[1:2])
        
        # Read the CSV file
        df = pd.read_csv(os.path.join(directory, filename))
        
        # Compute the required statistics
        # total_instructions_sum = df["Total Number of (any) instructions"].sum()
        total_instructions_avg = df["Total Number of (any) instructions"].mean()
        # ratio_vectorial_sum = df["Ratio vectorial instructions"].sum()
        ratio_vectorial_avg = df["Ratio vectorial instructions"].mean()
        
        # Prepare data for output
        if base_name not in data_dict:
            data_dict[base_name] = {}
        
        vectorial_type = '.'.join(filename.split('.')[3:4])
        vectorial_type = '_'.join(vectorial_type.split('_')[:1])
        
        # data_dict[base_name][f"Total Number of (any) instructions {vectorial_type}"] = total_instructions_sum
        data_dict[base_name][f"Total Number of (any) instructions avg {vectorial_type}"] = total_instructions_avg
        # data_dict[base_name][f"Ratio vectorial instructions {vectorial_type}"] = ratio_vectorial_sum
        data_dict[base_name][f"Ratio vectorial instructions avg {vectorial_type}"] = ratio_vectorial_avg

# Create CSV files for each group
for base_name, stats in data_dict.items():
    # Create a DataFrame from the dictionary
    output_df = pd.DataFrame([stats])
    
    # Save to CSV
    output_df.to_csv(f"./resultsComparison/{base_name}_comparision.csv", index=False)

print("CSV files have been created successfully.")
