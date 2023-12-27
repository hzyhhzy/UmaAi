
from dataset_filtered import trainset
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



if not os.path.exists("../saved_models"):
    os.mkdir("../saved_models")

def BCEfunction(x,y):
    return torch.nn.functional.softplus(x)-x*y + torch.log(y+1e-10)*y+torch.log(1-y+1e-10)*(1-y)

def cross_entropy_loss(output, target):
    t = torch.log_softmax(output,dim=1)
    losses = torch.sum(-t*target, dim=1)+torch.sum(torch.log(target+1e-10)*target, dim=1)

    return losses.mean(dim=0)



def calculateLoss(output,label):
    output_policy=output[:,:18]
    output_value=output[:,18:]
    label_policy=label[:,:18]
    label_value=label[:,18:]
    #print(output_value*200+30000,label_value)
    #print(torch.softmax(output_policy[:,:10],dim=1), label_policy[:,:10])

    huberloss=nn.HuberLoss(reduction='mean',delta=1.0)
    vloss1 = huberloss(output_value[:,0],(label_value[:,0]-30000)/200)
    vloss2 = huberloss(output_value[:,1],(label_value[:,1]-0)/100)
    vloss3 = huberloss(output_value[:,2],(label_value[:,2]-30000)/200)
    vloss=vloss1+vloss2+vloss3

    ploss1=cross_entropy_loss(output_policy[:,:10],label_policy[:,:10])
    ploss2=(label_policy[:,:5]*BCEfunction(output_policy[:,10:15],label_policy[:,10:15])).sum(1).mean(0)
    ploss3=(BCEfunction(output_policy[:,15:],label_policy[:,15:])).sum(1).mean(0)
    ploss = ploss1+ploss2+ploss3
    return vloss,ploss


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    #data settings
    parser.add_argument('--vdatadir', type=str, default='../selfplay8/selfplay/t1.npz', help='validation dataset path: dir include dataset files or single dataset file')

    parser.add_argument('--savename', type=str ,default='auto', help='model save pth, ""null"" means does not save, ""auto"" means modeltype+modelparam')
   #training parameters
    parser.add_argument('--gpu', type=int,
                        default=0, help='which gpu, -1 means cpu')
    parser.add_argument('--batchsize', type=int,
                        default=32, help='batch size')
    #parser.add_argument('--lr', type=float, default=2e-3, help='learning rate')
    args = parser.parse_args()
    #print("用的旧版数据，别忘了改回来")
    if(args.gpu==-1):
        device=torch.device('cpu')
    else:
        #print(torch.cuda.device_count())
        #os.environ['CUDA_VISIBLE_DEVICES'] = str(args.gpu)
        #print(torch.cuda.device_count())
        device = torch.device(f"cuda:{args.gpu}")




    print("Counting Data Files.........................................................................................")

    vdata_files=[]
    if(args.vdatadir is not None and os.path.exists(args.vdatadir)):
        if(os.path.splitext(args.vdatadir)[-1]=='.npz'): #single file
            vdata_files=[args.vdatadir]
        else:
            for (path,dirnames,filenames) in os.walk(args.vdatadir):
                filenames = [os.path.join(path,filename) for filename in filenames if filename.endswith('.npz')]
                vdata_files.extend(filenames)
    print("Finished counting data")

    basepath = f'../saved_models/{args.savename}/'

    print("Building model..............................................................................................")
    modelpath=os.path.join(basepath,"model.pth")
    if os.path.exists(modelpath) and (args.savename != 'null'):
        modeldata = torch.load(modelpath,map_location="cpu")
        model_type=modeldata['model_type']
        model_param=modeldata['model_param']
        model = ModelDic[model_type](*model_param).to(device)

        model.load_state_dict(modeldata['state_dict'])
        totalstep = modeldata['totalstep']
        print(f"Loaded model: type={model_type}, size={model_param}, totalstep={totalstep}")
    else:
        assert(False)

    for p in range(66):

        time0=time.time()
        loss_record_init=[0,0,0,1e-30,0]
        vdata_file = random.choice(vdata_files)
        vDataset = trainset(vdata_file,p)
        if(len(vDataset)==0):
            continue
        vDataloader = DataLoader(vDataset, shuffle=False, batch_size=args.batchsize)
        loss_record_val = loss_record_init.copy()
        vsamp=0
        model.eval()
        with torch.no_grad():
            for s, (x,label) in enumerate(vDataloader):
                if(x.shape[0]!=args.batchsize): #只要完整的batch
                    continue
                vsamp+=args.batchsize
                x = x.to(device)
                label = label.to(device)

                # optimize
                nnoutput = model(x)

                vloss,ploss = calculateLoss(nnoutput,label)

                _, p1_predicted = torch.max(nnoutput[:,:10], 1)
                _, p1_labels = torch.max(label[:,:10], 1)
                p1_correct = (p1_predicted == p1_labels).sum().item()

                loss = 1.0*vloss+1.0*ploss

                loss_record_val[0]+=(vloss.detach().item()+ploss.detach().item())
                loss_record_val[1]+=vloss.detach().item()
                loss_record_val[2]+=ploss.detach().item()
                loss_record_val[3]+=1
                loss_record_val[4]+=p1_correct/args.batchsize

        time1 = time.time()
        time_used = time1 - time0
        time0 = time1
        totalloss_val = loss_record_val[0] / loss_record_val[3]
        vloss_val = loss_record_val[1] / loss_record_val[3]
        ploss_val = loss_record_val[2] / loss_record_val[3]
        p1acc_val = loss_record_val[4] / loss_record_val[3]
        print("Validation: name: {}, param: {}, time: {:.2f} s, samp: {}, totalloss: {:.4f}, vloss: {:.4f}, ploss: {:.4f}, p1acc: {:.2f}%"
              .format(args.savename, p, time_used, vDataset.__len__(), totalloss_val, vloss_val, ploss_val, p1acc_val*100))
