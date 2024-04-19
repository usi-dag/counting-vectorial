import json
import csv
import os

# Ensure the results directory exists
os.makedirs('./results_vectorial_instructions', exist_ok=True)

# Load the JSON data from the file
with open('./jmh-results-vectorial.json', 'r') as file:
    data = json.load(file)

# Process each benchmark result in the JSON array
for entry in data:
    file_name = entry['benchmark']
    score_unit = entry['primaryMetric']['scoreUnit']
    raw_data = entry['primaryMetric']['rawData']
    concatenated_data = []
    for array in raw_data:
        concatenated_data.extend(array)

    # Define the CSV file path
    csv_file_path = f'./results_vectorial_instructions/{file_name}_instructionsCount.csv'

    # Check if the CSV file exists
    if not os.path.exists(csv_file_path):
        raise FileNotFoundError(f"The file {csv_file_path} does not exist. Please ensure the file is created before running this script.")

    # Read existing data
    with open(csv_file_path, mode='r', newline='') as file:
        reader = csv.reader(file)
        existing_data = list(reader)

    # Find the index of the "Iteration" column
    try:
        iteration_index = existing_data[0].index("Iteration") + 1
    except ValueError:
        raise ValueError("The 'Iteration' column was not found in the existing CSV file.")

    # Check if the lengths match
    if len(existing_data[1:]) != len(concatenated_data):
        raise ValueError("The number of rows in the existing CSV does not match the number of data points in the JSON data.")

    # Add the new column header if not already present
    # if len(existing_data[0]) == len(existing_data[1]) + 1:
    existing_data[0].insert(iteration_index, f'Iteration time {score_unit} (Vectorial Instructions Count)')
    
    # Insert new column data to existing rows
    for i, row in enumerate(existing_data[1:]):
        row.insert(iteration_index, concatenated_data[i])  # Safely insert the new data to existing rows

    # Write the updated data back to the CSV
    with open(csv_file_path, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(existing_data)
        
        
# Ensure the results directory exists
os.makedirs('./results_total_instructions', exist_ok=True)

# Load the JSON data from the file
with open('./jmh-results-total.json', 'r') as file:
    data = json.load(file)

# Process each benchmark result in the JSON array
for entry in data:
    file_name = entry['benchmark']
    score_unit = entry['primaryMetric']['scoreUnit']
    raw_data = entry['primaryMetric']['rawData']
    concatenated_data = []
    for array in raw_data:
        concatenated_data.extend(array)

    # Define the CSV file path
    csv_file_path = f'./results_total_instructions/{file_name}_instructionsCount.csv'

    # Check if the CSV file exists
    if not os.path.exists(csv_file_path):
        raise FileNotFoundError(f"The file {csv_file_path} does not exist. Please ensure the file is created before running this script.")

    # Read existing data
    with open(csv_file_path, mode='r', newline='') as file:
        reader = csv.reader(file)
        existing_data = list(reader)

    # Find the index of the "Iteration" column
    try:
        iteration_index = existing_data[0].index("Iteration") + 1
    except ValueError:
        raise ValueError("The 'Iteration' column was not found in the existing CSV file.")

    # Check if the lengths match
    if len(existing_data[1:]) != len(concatenated_data):
        raise ValueError("The number of rows in the existing CSV does not match the number of data points in the JSON data.")

    # Add the new column header if not already present
    # if len(existing_data[0]) == len(existing_data[1]) + 1:
    existing_data[0].insert(iteration_index, f'Iteration time {score_unit} (Total Instructions Count)')
    
    # Insert new column data to existing rows
    for i, row in enumerate(existing_data[1:]):
        row.insert(iteration_index, concatenated_data[i])  # Safely insert the new data to existing rows

    # Write the updated data back to the CSV
    with open(csv_file_path, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(existing_data)
