#import torch
from torch.utils.data import Dataset, DataLoader
import os
import numpy as np
import time
import math
from config import *



class trainset(Dataset):
    def __init__(self, npz_path):

        data = np.load(npz_path)
        self.x=data["x"]
        self.label=data["label"]

        #valid_data = self.x[:, 2] > 0.3 * (math.log(1024 + 1.0) - 7.0)

        # 使用掩码来筛选数组
        #self.x = self.x[valid_data]
        #self.label = self.label[valid_data]



    def __getitem__(self, index):

        x=self.x[index].astype(np.float32)
        label=self.label[index].astype(np.float32)

        return x,label
    def __len__(self):
        return self.x.shape[0]



