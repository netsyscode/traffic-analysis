import os
import pandas as pd
import difflib
from collections import defaultdict

def calculate_similarity(str1, str2):
    return difflib.SequenceMatcher(None, str1, str2).ratio()

def find_similar_files(dir_path):
    # Store filenames in a dictionary where the key is the base name (i.e., without number suffixes)
    file_dict = defaultdict(list)

    filenames = [f for f in os.listdir(dir_path) if f.endswith('.csv')]

    while filenames:
        filename = filenames.pop(0)
        basename = ''.join([i for i in filename if not i.isdigit()]).rstrip('.csv')
        similar_files = [filename]

        for other_file in filenames[:]:
            other_basename = ''.join([i for i in other_file if not i.isdigit()]).rstrip('.csv')

            if calculate_similarity(basename, other_basename) >= 0.8:
                similar_files.append(other_file)
                filenames.remove(other_file)

        file_dict[basename] = similar_files

    return file_dict

def merge_files(dir_path, output_path, file_dict):
    for basename, filenames in file_dict.items():
        # Merge files
        merged_df = pd.concat([pd.read_csv(os.path.join(dir_path, filename)) for filename in filenames])

        # Save the merged dataframe to a new csv file
        merged_df.to_csv(os.path.join(output_path, f"{basename}_merged.csv"), index=False)
        print(f"Merged file {basename}_merged.csv created.")

# Directory path
dir_path ='tls_feat_extraction/Output_ios'
output_path ='dataset/Merge_ios'

# Find similar files in the directory
file_dict = find_similar_files(dir_path)

# Merge the similar files
merge_files(dir_path, output_path, file_dict)
