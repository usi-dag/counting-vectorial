import pandas as pd
import os

# Directory containing the CSV files
directory = "./results"

# Loop through every file in the directory
for filename in os.listdir(directory):
    if filename.endswith(".csv"):
        # Construct full file path
        file_path = os.path.join(directory, filename)
        
        # Read the CSV file into a DataFrame
        df = pd.read_csv(file_path)
        
        # Check if the DataFrame has at least 3 columns
        if df.shape[1] < 3:
            print(f"File {filename} does not have enough columns.")
            continue
        
        # Add a new column 'Ratio vectorial instructions' after the second column
        df.insert(2, 'Ratio vectorial instructions', 0)
        
        # Iterate over each row in the DataFrame
        for index, row in df.iterrows():
            # Sum of all numbers from the 4th column onwards
            vectorial_instructions = row[3:].sum()
            
            # Column name that contains 'Total Number of (any) instructions'
            total_instructions_col = [col for col in df.columns if "Total Number of (any) instructions" in col]
            
            if not total_instructions_col:
                print(f"Column with 'Total Number of (any) instructions' not found in {filename}")
                continue
            
            # Calculate the ratio
            total_instructions = row[total_instructions_col[0]]
            if total_instructions != 0:
                ratio_vectorial_instructions = vectorial_instructions / total_instructions
            else:
                ratio_vectorial_instructions = 0
            
            # Assign the calculated ratio to the new column
            df.at[index, 'Ratio vectorial instructions'] = ratio_vectorial_instructions
        
        # Save the modified DataFrame back to CSV
        df.to_csv(file_path, index=False)
        print(f"Processed {filename}")

print("All files processed.")
