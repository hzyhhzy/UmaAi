from config import *
from dataset import trainset
from model import ModelDic,Model_EncoderMlpSimple
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


def exportWeight(x, name, exportfile):
    x = x.data.reshape([-1]).cpu().numpy()
    print(name, file=exportfile)
    print(x.shape[0],file=exportfile)
    for i in range(x.shape[0]):
        print(x[i],end=' ',file=exportfile)
    print('',file=exportfile)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--vdata', type=str, default='./example/example_data.npz', help='validation dataset file')

    parser.add_argument('--savename', type=str ,default='ems_1_128_1_256_256', help='model save pth')

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
    modeldata = torch.load(modelpath,map_location="cpu")
    modeltype = modeldata['model_type']
    assert(modeltype=="ems")
    model_param=modeldata['model_param']
    model = Model_EncoderMlpSimple(*model_param).to(device)
    model.load_state_dict(modeldata['state_dict'])
    totalstep = modeldata['totalstep']
    print(f"Loaded model: type={modeltype}, size={model_param}, totalstep={totalstep}")


    if(args.vdata!=""):
        vDataset = trainset(args.vdata)
        vDataloader = DataLoader(vDataset, shuffle=False, batch_size=args.batchsize)
        model.eval()


        for s, (x, label) in enumerate(vDataloader):
            if (x.shape[0] != args.batchsize):  # 只要完整的batch
                continue

            x = x.to(device)
            #traced_model = torch.jit.trace(model, x)

            print(model(x[:8])[:,:5])

            break

    exportPath = os.path.join(basepath, "model.txt")
    exportfile=open(exportPath,'w')


    print(modeltype,file=exportfile)
    print(model.model_param[0],model.model_param[1],model.model_param[2],model.model_param[3],model.model_param[4],file=exportfile)

    hasPersonFeature=Game_Head_Num!=0
    headN=Game_Head_Num if hasPersonFeature else Game_Card_Num

    #inputhead
    exportWeight(model.inputheadGlobal1.weight,"inputheadGlobal1",exportfile)
    exportWeight(model.inputheadGlobal2.weight,"inputheadGlobal2",exportfile)
    exportWeight(model.inputheadCard.weight,"inputheadCard",exportfile)
    if hasPersonFeature:
        exportWeight(model.inputheadPerson.weight,"inputheadPerson",exportfile)

    for i in range(len(model.encoderTrunk)):
        exportWeight(model.encoderTrunk[i].lin_Q.weight / headN, f"encoder_{i}.lin_Q", exportfile)
        exportWeight(model.encoderTrunk[i].lin_V.weight, f"encoder_{i}.lin_V", exportfile)
        exportWeight(model.encoderTrunk[i].lin_global.weight, f"encoder_{i}.lin_global", exportfile)


    exportWeight(model.linBeforeMLP1.weight,"linBeforeMLP1",exportfile)
    exportWeight(model.linBeforeMLP2.weight / headN,"linBeforeMLP2",exportfile)

    for i in range(len(model.mlpTrunk)):
        exportWeight(model.mlpTrunk[i].lin1.weight, f"mlp_{i}.lin1", exportfile)
        exportWeight(model.mlpTrunk[i].lin2.weight, f"mlp_{i}.lin2", exportfile)

    exportWeight(model.outputhead.weight, "outputhead_w", exportfile)
    exportWeight(model.outputhead.bias,"outputhead_b",exportfile)