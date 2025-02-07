
from dataset import trainset
from model import ModelDic
from config import *

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


backup_checkpoints=[50000*i for i in range(500)]


def save_checkpoint(model, optimizer, path):
    savedic={'totalstep': totalstep,
            'state_dict': model.state_dict(),
            'model_type': model.model_type,
            'model_param':model.model_param}
    if(optimizer is not None):
        savedic['optimizer_state_dict']=optimizer.state_dict()

    torch.save(
        savedic,
        path)
    print('Model saved in {}\n'.format(path))

if not os.path.exists("./saved_models"):
    os.mkdir("./saved_models")

def BCEfunction(x,y):
    return torch.nn.functional.softplus(x)-x*y + torch.log(y+1e-10)*y+torch.log(1-y+1e-10)*(1-y)

def cross_entropy_loss(output, target):
    t = torch.log_softmax(output,dim=1)
    losses = torch.sum(-t*target, dim=1)+torch.sum(torch.log(target+1e-10)*target, dim=1)

    return losses.mean(dim=0)



def calculateLoss(output,label):
    output_policy=output[:,:Game_Output_C_Policy]
    output_value=output[:,Game_Output_C_Policy:]
    label_policy=label[:,:Game_Output_C_Policy]
    label_value=label[:,Game_Output_C_Policy:]
    #print(output_value*200+30000,label_value)
    #print(torch.softmax(output_policy[:,:10],dim=1), label_policy[:,:10])

    huberloss=nn.HuberLoss(reduction='mean',delta=1.0)
    vloss1 = 0.5*huberloss(output_value[:,0],(label_value[:,0]-Value_Mean)/Value_Scale)
    vloss2 = 1.0*huberloss(output_value[:,1],(label_value[:,1]-0)/Valuevar_Scale)
    vloss3 = 0.5*huberloss(output_value[:,2],(label_value[:,2]-Value_Mean)/Value_Scale)
    vloss=vloss1+vloss2+vloss3

    ploss1=cross_entropy_loss(output_policy,label_policy)
    ploss = ploss1

    #if(random.randint(0,1000)==0):
    #    print(f"vloss items = {vloss1.item():.4f} {vloss2.item():.4f} {vloss3.item():.4f} {ploss1.item():.4f} ")
    return vloss,ploss


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    #data settings
    parser.add_argument('--tdatadir', type=str, default='./tdata.npz', help='train dataset path: dir include dataset files or single dataset file')
    parser.add_argument('--vdatadir', type=str, default='./vdata.npz', help='validation dataset path: dir include dataset files or single dataset file')
    parser.add_argument('--maxvalsamp', type=int, default=32768, help='validation sample num')
    parser.add_argument('--maxstep', type=int, default=5000000000, help='max step to train')
    parser.add_argument('--savestep', type=int, default=2000, help='step to save and validation')
    parser.add_argument('--infostep', type=int, default=500, help='step to logger')

    parser.add_argument('--sampling', type=float, default=1, help='sampling rate(to avoid overfitting)')
    parser.add_argument('--valuesampling', type=float, default=1, help='value sampling rate(to avoid overfitting)')

    #model parameters
    #parser.add_argument('--modeltype', type=str, default='ems',help='model type defined in model.py')
    #parser.add_argument('--modelparam', nargs='+',type=int,
    #                    default=(1,128,3,256,256), help='model size')
    #parser.add_argument('--modeltype', type=str, default='tl',help='model type defined in model.py')
    #parser.add_argument('--modelparam', nargs='+',type=int,
    #                    default=(256,), help='model size')
    parser.add_argument('--modeltype', type=str, default='tf',help='model type defined in model.py')
    parser.add_argument('--modelparam', nargs='+',type=int,
                        default=(3,256,4,2), help='model size')

    parser.add_argument('--savename', type=str ,default='auto', help='model save pth, ""null"" means does not save, ""auto"" means modeltype+modelparam')
    parser.add_argument('--new', action='store_true', default=False, help='whether to retrain')

    #training parameters
    parser.add_argument('--gpu', type=int,
                        default=0, help='which gpu, -1 means cpu')
    parser.add_argument('--datathread', type=int,
                        default=1, help='how many threads for dataloader (windows has some bugs)')
    parser.add_argument('--batchsize', type=int,
                        default=2048, help='batch size')
    #parser.add_argument('--lr', type=float, default=2e-3, help='learning rate')
    parser.add_argument('--lrscale', type=float, default=1.0, help='learning rate scale')
    parser.add_argument('--wdscale', type=float, default=1.0, help='weight decay')
    parser.add_argument('--gradclipscale', type=float, default=1.0, help='grad clip')
    parser.add_argument('--rollbackthreshold', type=float, default=0.05, help='if loss increased this value, roll back 2*infostep steps')
    args = parser.parse_args()
    #print("用的旧版数据，别忘了改回来")
    if(args.gpu==-1):
        device=torch.device('cpu')
    else:
        #print(torch.cuda.device_count())
        #os.environ['CUDA_VISIBLE_DEVICES'] = str(args.gpu)
        #print(torch.cuda.device_count())
        device = torch.device(f"cuda:{args.gpu}")

    if(args.savename=="auto"):
        args.savename=args.modeltype
        for i in args.modelparam:
            args.savename=args.savename+"_"+str(i)




    print("Counting Data Files.........................................................................................")

    tdata_files=[]
    if(os.path.splitext(args.tdatadir)[-1]=='.npz'): #single file
        tdata_files=[args.tdatadir]
    else:
        for (path,dirnames,filenames) in os.walk(args.tdatadir):
            filenames = [os.path.join(path,filename) for filename in filenames if filename.endswith('.npz')]
            tdata_files.extend(filenames)

    vdata_files=[]
    if(args.vdatadir is not None and os.path.exists(args.vdatadir)):
        if(os.path.splitext(args.vdatadir)[-1]=='.npz'): #single file
            vdata_files=[args.vdatadir]
        else:
            for (path,dirnames,filenames) in os.walk(args.vdatadir):
                filenames = [os.path.join(path,filename) for filename in filenames if filename.endswith('.npz')]
                vdata_files.extend(filenames)
    print("Finished counting data")

    basepath = f'./saved_models/{args.savename}/'
    if not os.path.exists(basepath):
        os.mkdir(basepath)
    backuppath=os.path.join(basepath,"backup")
    if not os.path.exists(backuppath):
        os.mkdir(backuppath)
    tensorboardpath=os.path.join(basepath,"tensorboardData")

    #tensorboard writer
    if not os.path.exists(tensorboardpath):
        os.mkdir(tensorboardpath)
    train_writer=SummaryWriter(os.path.join(tensorboardpath,"train"))
    val_writer=SummaryWriter(os.path.join(tensorboardpath,"val"))

    print("Building model..............................................................................................")
    optimizer_state_dict_initial=None
    modelpath=os.path.join(basepath,"model.pth")
    if os.path.exists(modelpath) and (not args.new) and (args.savename != 'null'):
        modeldata = torch.load(modelpath,map_location="cpu")
        model_type=modeldata['model_type']
        model_param=modeldata['model_param']
        model = ModelDic[model_type](*model_param).to(device)

        model.load_state_dict(modeldata['state_dict'])
        if("optimizer_state_dict" in modeldata):
            optimizer_state_dict_initial=modeldata["optimizer_state_dict"]
            print("Loaded optimizer state dict")
        totalstep = modeldata['totalstep']
        print(f"Loaded model: type={model_type}, size={model_param}, totalstep={totalstep}")
    else:
        totalstep = 0
        model_type=args.modeltype
        model_param=args.modelparam
        model = ModelDic[model_type](*model_param).to(device)

    startstep=totalstep
    if model_type == 'res' or model_type == 'tl':

        lr = 1e-3
        lrhead = 1e-3
        wd = 1e-3
        wdhead = 1e-3
        # lowl2param是一些密集型神经网络参数(mlp,cnn等)，对lr和weightdecay更敏感，使用float32计算，几乎不需要weightdecay
        # otherparam需要高的weightdecay
        headparam = list(map(id, model.inputhead.parameters()))
        otherparam = list(filter(lambda p: id(p) not in headparam, model.parameters()))
        headparam = list(filter(lambda p: id(p) in headparam, model.parameters()))
        optimizer = optim.AdamW([{'params': otherparam},
                                {'params': headparam, 'lr': lrhead*args.lrscale, 'weight_decay': wdhead*args.wdscale}],
                               lr=lr*args.lrscale, weight_decay=wd*args.wdscale)
    elif model_type == 'tf' or model_type == 'tf2' or model_type == 'tfmlp':

        lr = 1e-3
        lrhead = 1e-3
        lrtf = 1e-3
        wd = 1e-3
        wdhead = 1e-3
        headparam = list(map(id, model.inputhead.parameters()))
        transformerparam=list(map(id, model.transformer_encoder.parameters()))
        otherparam = list(filter(lambda p: id(p) not in headparam and id(p) not in transformerparam, model.parameters()))
        headparam = list(filter(lambda p: id(p) in headparam, model.parameters()))
        transformerparam = list(filter(lambda p: id(p) in transformerparam, model.parameters()))
        optimizer = optim.AdamW([{'params': otherparam},
                                {'params': headparam, 'lr': lrhead*args.lrscale, 'weight_decay': wdhead*args.wdscale},
                                {'params': transformerparam, 'lr': lrtf*args.lrscale, 'weight_decay': wd*args.wdscale}],
                               lr=lr*args.lrscale, weight_decay=wd*args.wdscale)

    elif model_type == 'em' or model_type == 'ems' or model_type == 'ems2' or model_type == 'ems3' or model_type == 'emsf' or model_type == 'emsm' or model_type == 'emsb':
        lr = 1e-3
        wd = 1e-3
        optimizer = optim.AdamW(model.parameters(),lr=lr*args.lrscale,weight_decay=wd*args.wdscale)
    else:
        lr = 1e-3
        wd = 1e-3
        optimizer = optim.AdamW(model.parameters(),lr=lr*args.lrscale,weight_decay=wd*args.wdscale)


    if optimizer_state_dict_initial is not None:
        # 复制优化器状态，但不复制学习率等参数
        optimizer_state_dict_initial['param_groups']=optimizer.state_dict()['param_groups']
        optimizer.load_state_dict(optimizer_state_dict_initial)

    model.train()

    #for rollbacking if loss explodes
    modelbackup1=copy.deepcopy(model.state_dict())
    modelbackup2=copy.deepcopy(model.state_dict())
    modelbackup1_step=startstep
    modelbackup2_step=startstep
    modelbackup1_loss=1e10
    modelbackup2_loss=1e10

    time0=time.time()
    loss_record_init=[0,0,0,1e-30,0,0,0,0]
    loss_record=loss_record_init.copy()
    print("Start Training..............................................................................................")
    while True:
        #if there is only one file, avoid repeating loading it
        if((len(tdata_files)!=1) or ('tDataloader' not in locals())):
            tdata_file=random.choice(tdata_files)
            #print(f"Selected training file: {tdata_file}")
            tDataset = trainset(tdata_file)
            #print(f"{tDataset.__len__()} rows")
            tDataloader = DataLoader(tDataset, shuffle=True, batch_size=args.batchsize, pin_memory=True, num_workers=args.datathread)

        for _ , (x,label) in enumerate(tDataloader):
            if(x.shape[0]!=args.batchsize): #只要完整的batch
                continue
            if(random.random()>args.sampling): #随机舍去1-args.sampling的数据
                continue
            # data
            x = x.to(device)
            label = label.to(device)

            # optimize
            optimizer.zero_grad()
            nnoutput = model(x)

            vloss,ploss = calculateLoss(nnoutput,label)


            nnoutput_policy=nnoutput[:, :Game_Output_C_Policy]
            label_policy=label[:, :Game_Output_C_Policy]
            _, p1_predicted = torch.max(nnoutput_policy, 1)
            p1_labelValues, p1_labels = torch.max(label_policy, 1)
            p1_predictedValues = label_policy[torch.arange(label_policy.shape[0]),p1_predicted]
            p1_valueDif = 100 * (torch.log(p1_labelValues + 0.01) - torch.log(p1_predictedValues + 0.01)).sum().item()
            p1_correct = (p1_predicted == p1_labels).sum().item()
            #print(torch.log(p1_labelValues + 0.01) - torch.log(p1_predictedValues + 0.01))

            loss = 0.0*vloss+1.0*ploss
            if(random.random()<=args.valuesampling):
                loss=Vloss_Scale*vloss+1.0*ploss
            loss_record[0] += loss.detach().item()
            loss_record[1] += vloss.detach().item()
            loss_record[2] += ploss.detach().item()
            loss_record[3] += 1
            loss_record[4] += p1_correct / args.batchsize
            loss_record[5] += p1_valueDif / args.batchsize

            loss.backward()
            default_grad_clip=500
            gradclipscale=args.gradclipscale if args.gradclipscale > 0 else 1e8
            gnorm_cap=args.gradclipscale*default_grad_clip
            gnorm_batch = torch.nn.utils.clip_grad_norm_(model.parameters(), max_norm=gnorm_cap)
            exgnorm_batch = max(0.0, gnorm_batch - gnorm_cap)
            loss_record[6] += gnorm_batch
            loss_record[7] += exgnorm_batch


            optimizer.step()

            # logs
            totalstep += 1
            if(totalstep % args.infostep == 0):
                time1=time.time()
                time_used=time1-time0
                time0=time1
                totalloss_train=loss_record[0]/loss_record[3]
                vloss_train=loss_record[1]/loss_record[3]
                ploss_train=loss_record[2]/loss_record[3]
                p1acc_train=loss_record[4]/loss_record[3]
                p1dif_train=loss_record[5]/loss_record[3]
                gnorm_train=loss_record[6]/loss_record[3]
                exgnorm_train=loss_record[7]/loss_record[3]
                print("name: {}, time: {:.2f} s, step: {}, totalloss: {:.4f}, vloss: {:.4f}, ploss: {:.4f}, p1acc: {:.2f}%, p1dif: {:.2f}, gnorm: {:.2f}, exgnorm: {:.2f}"
                      .format(args.savename,time_used,totalstep,totalloss_train,vloss_train,ploss_train,100*p1acc_train,p1dif_train,gnorm_train,exgnorm_train))
                train_writer.add_scalar("steps_each_second",loss_record[3]/time_used,global_step=totalstep)
                train_writer.add_scalar("totalloss",totalloss_train,global_step=totalstep)
                train_writer.add_scalar("vloss",vloss_train,global_step=totalstep)
                train_writer.add_scalar("ploss",ploss_train,global_step=totalstep)
                train_writer.add_scalar("p1acc",p1acc_train,global_step=totalstep)
                train_writer.add_scalar("p1dif",p1dif_train,global_step=totalstep)
                train_writer.add_scalar("gnorm",gnorm_train,global_step=totalstep)
                train_writer.add_scalar("exgnorm",exgnorm_train,global_step=totalstep)

                loss_record = loss_record_init.copy()

                #check whether loss explodes
                if(totalloss_train>modelbackup1_loss+args.rollbackthreshold):
                    print(f"loss explodes, rollback {2*args.infostep} steps to {modelbackup2_step}")
                    model.load_state_dict(modelbackup2)
                    modelbackup1=copy.deepcopy(modelbackup2)
                    totalstep=modelbackup2_step
                    modelbackup1_step=modelbackup2_step
                    modelbackup1_loss=modelbackup2_loss
                else:
                    #update backups
                    modelbackup2=modelbackup1
                    modelbackup2_step=modelbackup1_step
                    modelbackup2_loss=modelbackup1_loss
                    modelbackup1=copy.deepcopy(model.state_dict())
                    modelbackup1_step=totalstep
                    modelbackup1_loss=totalloss_train


            if((totalstep % args.savestep == 0) or (totalstep-startstep==args.maxstep) or (totalstep in backup_checkpoints)):

                print(f"Finished training {totalstep} steps")
                save_checkpoint(model=model,optimizer=optimizer,path=modelpath)
                print('Model saved in {}\n'.format(modelpath))

                if(totalstep in backup_checkpoints):
                    modelpath_backup=os.path.join(backuppath,str(totalstep)+".pth")
                    save_checkpoint(model=model,optimizer=optimizer,path=modelpath_backup)
                    print('Model saved in {}\n'.format(modelpath_backup))


                if vdata_files:
                    time0=time.time()
                    print("Start validation")
                    vdata_file = random.choice(vdata_files)
                    print(f"Selected validation file: {vdata_file}")
                    vDataset = trainset(vdata_file)
                    print(f"{vDataset.__len__()} rows")
                    vDataloader = DataLoader(vDataset, shuffle=False, batch_size=args.batchsize, pin_memory=True, num_workers=args.datathread)
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
                            optimizer.zero_grad()
                            nnoutput = model(x)

                            vloss,ploss = calculateLoss(nnoutput,label)

                            nnoutput_policy = nnoutput[:, :Game_Output_C_Policy]
                            label_policy = label[:, :Game_Output_C_Policy]
                            _, p1_predicted = torch.max(nnoutput_policy, 1)
                            p1_labelValues, p1_labels = torch.max(label_policy, 1)
                            p1_predictedValues = label_policy[torch.arange(label_policy.shape[0]),p1_predicted]
                            p1_valueDif = 100 * (torch.log(p1_labelValues + 0.01) - torch.log(
                                p1_predictedValues + 0.01)).sum().item()
                            p1_correct = (p1_predicted == p1_labels).sum().item()

                            loss = Vloss_Scale*vloss+1.0*ploss

                            loss_record_val[0]+=loss.detach().item()
                            loss_record_val[1]+=vloss.detach().item()
                            loss_record_val[2]+=ploss.detach().item()
                            loss_record_val[3]+=1
                            loss_record_val[4]+=p1_correct/args.batchsize
                            loss_record_val[5]+=p1_valueDif/args.batchsize
                            if(vsamp>=args.maxvalsamp):
                                break

                    time1 = time.time()
                    time_used = time1 - time0
                    time0 = time1
                    totalloss_val = loss_record_val[0] / loss_record_val[3]
                    vloss_val = loss_record_val[1] / loss_record_val[3]
                    ploss_val = loss_record_val[2] / loss_record_val[3]
                    p1acc_val = loss_record_val[4] / loss_record_val[3]
                    p1dif_val = loss_record_val[5] / loss_record_val[3]

                    # 计算模型权重范数
                    weight_norm = 0.0
                    for param in model.parameters():
                        if param.requires_grad:
                            weight_norm += torch.norm(param).item()**2
                    weight_norm = weight_norm**0.5

                    print("Validation: name: {}, time: {:.2f} s, step: {}, totalloss: {:.4f}, vloss: {:.4f}, ploss: {:.4f}, p1acc: {:.2f}%, p1dif: {:.2f}, weight_norm: {:.2f}"
                          .format(args.savename, time_used, totalstep, totalloss_val, vloss_val, ploss_val, p1acc_val*100,p1dif_val,weight_norm))
                    val_writer.add_scalar("steps_each_second", loss_record[3] / time_used, global_step=totalstep)
                    val_writer.add_scalar("totalloss", totalloss_val, global_step=totalstep)
                    val_writer.add_scalar("vloss", vloss_val, global_step=totalstep)
                    val_writer.add_scalar("ploss", ploss_val, global_step=totalstep)
                    val_writer.add_scalar("p1acc", p1acc_val, global_step=totalstep)
                    val_writer.add_scalar("p1dif",p1dif_val,global_step=totalstep)
                    val_writer.add_scalar("weight_norm", weight_norm, global_step=totalstep)

                    model.train()

            if(totalstep - startstep >= args.maxstep):
                break

        if(totalstep - startstep >= args.maxstep):
            break