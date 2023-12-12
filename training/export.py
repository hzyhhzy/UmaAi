
from dataset import trainset
from model import ModelDic
#from config import boardH,boardW

import argparse
from torch.utils.data import Dataset, DataLoader
from torch.utils.tensorboard import SummaryWriter
import torch.optim as optim
import torch
import torch.nn as nn
import os
import time
import random
import copy


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--vdata', type=str, default='./example/example_data.npz', help='validation dataset file')

    parser.add_argument('--savename', type=str ,default='ems_1_128_3_256_256', help='model save pth')

    #training parameters
    parser.add_argument('--gpu', type=int,
                        default=-1, help='which gpu, -1 means cpu')
    parser.add_argument('--batchsize', type=int,
                        default=8, help='batch size')

    args = parser.parse_args()

    if(args.gpu==-1):
        device=torch.device('cpu')
    else:
        #print(torch.cuda.device_count())
        #os.environ['CUDA_VISIBLE_DEVICES'] = str(args.gpu)
        #print(torch.cuda.device_count())
        device = torch.device(f"cuda")


    basepath = f'../saved_models/{args.savename}/'

    print("Building model..............................................................................................")
    modelpath=os.path.join(basepath,"model.pth")
    if os.path.exists(modelpath):
        modeldata = torch.load(modelpath,map_location="cpu")
        model_type=modeldata['model_type']
        model_param=modeldata['model_param']
        model = ModelDic[model_type](*model_param).to(device)

        model.load_state_dict(modeldata['state_dict'])
        totalstep = modeldata['totalstep']
        print(f"Loaded model: type={model_type}, size={model_param}, totalstep={totalstep}")
    else:
        assert(False)




    vDataset = trainset(args.vdata)
    vDataloader = DataLoader(vDataset, shuffle=False, batch_size=args.batchsize)
    model.eval()


    for s, (x, label) in enumerate(vDataloader):
        if (x.shape[0] != args.batchsize):  # 只要完整的batch
            continue

        x = x.to(device)
        traced_model = torch.jit.trace(model, x)

        print(model(x[:8])[:,:5])

        break

    traced_model.save(basepath+"model_traced.pt")