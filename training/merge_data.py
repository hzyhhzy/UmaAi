import numpy as np
import os

#dir='./vdata'
dir='./tdata'
# Initialize lists to store data
x_list = []
label_list = []

# Traverse through all npz files in the './val' directory
for file in os.listdir(dir):
    if file.endswith('.npz'):
        data = np.load(os.path.join(dir, file))
        x_list.append(data['x'])
        label_list.append(data['label'])

# Combine all arrays
x_combined = np.vstack(x_list)
label_combined = np.vstack(label_list)

print(x_combined.shape[0], "rows")
# Save the combined arrays to a npz file
np.savez_compressed('./tdata.npz', x=x_combined, label=label_combined)