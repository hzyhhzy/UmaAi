#import torch
from torch.utils.data import Dataset, DataLoader
import os
import numpy as np
import time
import math
from config import *



class trainset(Dataset):
    def __init__(self, npz_path,param):

        data = np.load(npz_path)
        self.x=data["x"]
        self.label=data["label"]

        valid_data = self.x[:, 44+param] > 0.5
        #valid_data = np.logical_and((self.label[:, 18] > param*1000) , (self.label[:, 18] <= (param+1)*1000))

        # 使用掩码来筛选数组
        self.x = self.x[valid_data]
        self.label = self.label[valid_data]



    def __getitem__(self, index):

        x=self.x[index].astype(np.float32)
        label=self.label[index].astype(np.float32)

        return x,label
    def __len__(self):
        return self.x.shape[0]



