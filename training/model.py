
import torch
import torch.nn as nn
import torch.nn.functional as F
import numpy as np
from config import *


class LinearBN(nn.Module):
    def __init__(self, in_features,out_features,bias):
        super().__init__()
        self.lin = nn.Linear(in_features=in_features,out_features=out_features,bias=bias)
        self.bn=nn.BatchNorm1d(out_features,affine=bias)

    def forward(self, x):
        y = self.lin(x)
        y = self.bn(y)
        return y

class EncoderLayer(nn.Module):
    def __init__(self, inout_c, mid_c, global_c):
        super().__init__()
        self.inout_c = inout_c
        self.mid_c = mid_c
        self.lin_Q = nn.Linear(inout_c, mid_c, bias=False)
        self.lin_K = nn.Linear(inout_c, mid_c, bias=False)
        self.lin_V = nn.Linear(inout_c, inout_c, bias=False)
        self.lin_global = nn.Linear(global_c, inout_c, bias=False)

    def forward(self, x, gf):
        y=x.view(x.shape[0]*x.shape[1],x.shape[2])
        q=self.lin_Q(y).reshape(x.shape[0],x.shape[1],self.mid_c)
        k=self.lin_K(y).reshape(x.shape[0],x.shape[1],self.mid_c)
        v=self.lin_V(y).reshape(x.shape[0],x.shape[1],self.inout_c)
        att=torch.relu(torch.einsum("nac,nbc->nab",q,k))
        y=torch.einsum("nab,nac->nbc",att,v)/x.shape[1]
        y=torch.relu(y+self.lin_global(gf).reshape(x.shape[0],1,self.inout_c))

        y = y + x
        return y

class EncoderLayerSimple(nn.Module):
    def __init__(self, inout_c, global_c):
        super().__init__()
        self.inout_c = inout_c
        self.lin_Q = nn.Linear(inout_c, inout_c, bias=False)
        self.lin_V = nn.Linear(inout_c, inout_c, bias=False)
        self.lin_global = nn.Linear(global_c, inout_c, bias=False)

    def forward(self, x, gf):
        y=x.view(x.shape[0]*x.shape[1],x.shape[2])
        q=self.lin_Q(y).view(x.shape[0],x.shape[1],self.inout_c)
        v=self.lin_V(y).view(x.shape[0],x.shape[1],self.inout_c)
        att=torch.relu(torch.bmm(x,q.transpose(1,2)))*0.05
        y=torch.bmm(att,v)
        y=torch.relu(y+self.lin_global(gf).view(x.shape[0],1,self.inout_c))

        y = y + x
        return y

class EncoderLayerSimpleBN(nn.Module):
    def __init__(self, inout_c, global_c):
        super().__init__()
        self.inout_c = inout_c
        self.lin_Q = LinearBN(inout_c, inout_c, bias=False)
        self.lin_V = LinearBN(inout_c, inout_c, bias=False)
        self.lin_global = LinearBN(global_c, inout_c, bias=False)

    def forward(self, x, gf):
        y=x.view(x.shape[0]*x.shape[1],x.shape[2])
        q=self.lin_Q(y).view(x.shape[0],x.shape[1],self.inout_c)
        v=self.lin_V(y).view(x.shape[0],x.shape[1],self.inout_c)
        att=torch.relu(torch.bmm(x,q.transpose(1,2)))*0.05
        y=torch.bmm(att,v)
        y=torch.relu(y+self.lin_global(gf).view(x.shape[0],1,self.inout_c))

        y = y + x
        return y

class ResnetLayer(nn.Module):
    def __init__(self, inout_c, mid_c):
        super().__init__()
        self.lin1 = nn.Linear(inout_c, mid_c, bias=False)
        self.lin2 = nn.Linear(mid_c, inout_c, bias=False)

    def forward(self, x):
        y = self.lin1(x)
        y = torch.relu(y)
        y = self.lin2(y)
        y = torch.relu(y)
        y = y + x
        return y

class ResnetLayerBN(nn.Module):
    def __init__(self, inout_c, mid_c):
        super().__init__()
        self.lin1 = LinearBN(inout_c, mid_c, bias=False)
        self.lin2 = LinearBN(mid_c, inout_c, bias=False)

    def forward(self, x):
        y = self.lin1(x)
        y = torch.relu(y)
        y = self.lin2(y)
        y = torch.relu(y)
        y = y + x
        return y

class Model_ResNet(nn.Module):

    def __init__(self,b,f):
        super().__init__()
        self.model_type = "res"
        self.model_param=(b,f)

        self.inputhead=nn.Linear(Game_Input_C, f)
        self.trunk=nn.ModuleList()
        for i in range(b):
            self.trunk.append(ResnetLayer(f,f))
        self.outputhead=nn.Linear(f, Game_Output_C)

    def forward(self, x):
        h=self.inputhead(x)

        h=torch.relu(h)
        for block in self.trunk:
            h=block(h)

        return self.outputhead(h)

class Model_Transformer(nn.Module): #GPT大法好

    def __init__(self,b,f,nhead):
        super().__init__()
        self.model_type = "tf"
        self.model_param=(b,f,nhead)
        self.f=f
        self.headC=357+72+74
        self.inputhead=nn.Linear(self.headC, f)
        self.transformer_encoder = nn.TransformerEncoder(nn.TransformerEncoderLayer(d_model=f, nhead=nhead, dim_feedforward=2*f),num_layers=b)
        self.outputhead=nn.Linear(f, Game_Output_C)

    def prepareInput(self,x):
        batch_size=x.shape[0]
        A = 357 #全局信息的通道数
        B = 72 #支援卡参数的通道数
        C = 74 #larc人头的通道数
        assert(A+7*B+20*C==x.shape[1])

        # 分割张量
        x_A = x[:, :A]
        x_B = x[:, A:A + 7 * B].view(batch_size, 7, B)
        x_C = x[:, A + 7 * B:].view(batch_size, 20, C)

        # 配对B和C
        paired_BCs = []
        # 前6对
        for i in range(6):
            paired_BCs.append(torch.cat((x_B[:, i], x_C[:, i]), dim=1))
        # 第7对
        paired_BCs.append(torch.cat((x_B[:, 6], x_C[:, 18]), dim=1))
        # 剩余的C与零填充的B配对
        zero_B = 0.0*x[:,:B]
        for i in range(6, 20):
            if i != 18:  # 跳过已经配对的第19个C
                paired_BCs.append(torch.cat((zero_B, x_C[:, i]), dim=1))

        # 合并A
        result = [torch.cat((x_A, paired_BC), dim=1) for paired_BC in paired_BCs]
        result = torch.stack(result, dim=0)
        assert(result.shape[2]==self.headC)
        return result

    def forward(self, x):
        h=self.prepareInput(x)
        h=h.view(-1,self.headC)
        h=self.inputhead(h)
        h=torch.relu(h)
        h=h.view(20,x.shape[0],self.f)

        h=self.transformer_encoder(h)
        h=h.mean(dim=0)


        return self.outputhead(h)

class Model_Transformer2(nn.Module): #输出加一个relu

    def __init__(self,b,f,nhead):
        super().__init__()
        self.model_type = "tf2"
        self.model_param=(b,f,nhead)
        self.f=f
        self.headC=357+72+74
        self.inputhead=nn.Linear(self.headC, f)
        self.transformer_encoder = nn.TransformerEncoder(nn.TransformerEncoderLayer(d_model=f, nhead=nhead, dim_feedforward=2*f),num_layers=b)
        self.outputhead=nn.Linear(f, Game_Output_C)

    def prepareInput(self,x):
        batch_size=x.shape[0]
        A = 357 #全局信息的通道数
        B = 72 #支援卡参数的通道数
        C = 74 #larc人头的通道数
        assert(A+7*B+20*C==x.shape[1])

        # 分割张量
        x_A = x[:, :A]
        x_B = x[:, A:A + 7 * B].view(batch_size, 7, B)
        x_C = x[:, A + 7 * B:].view(batch_size, 20, C)

        # 配对B和C
        paired_BCs = []
        # 前6对
        for i in range(6):
            paired_BCs.append(torch.cat((x_B[:, i], x_C[:, i]), dim=1))
        # 第7对
        paired_BCs.append(torch.cat((x_B[:, 6], x_C[:, 18]), dim=1))
        # 剩余的C与零填充的B配对
        zero_B = 0.0*x[:,:B]
        for i in range(6, 20):
            if i != 18:  # 跳过已经配对的第19个C
                paired_BCs.append(torch.cat((zero_B, x_C[:, i]), dim=1))

        # 合并A
        result = [torch.cat((x_A, paired_BC), dim=1) for paired_BC in paired_BCs]
        result = torch.stack(result, dim=0)
        assert(result.shape[2]==self.headC)
        return result

    def forward(self, x):
        h=self.prepareInput(x)
        h=h.view(-1,self.headC)
        h=self.inputhead(h)
        h=torch.relu(h)
        h=h.view(20,x.shape[0],self.f)

        h=self.transformer_encoder(h)
        h=torch.relu(h)
        h=h.mean(dim=0)


        return self.outputhead(h)

class Model_TransformerMlp(nn.Module): #输出加一个relu

    def __init__(self,b,f,nhead,mlpb):
        super().__init__()
        self.model_type = "tfmlp"
        self.model_param=(b,f,nhead,mlpb)
        self.f=f
        self.headC=357+72+74
        self.inputhead=nn.Linear(self.headC, f)
        self.transformer_encoder = nn.TransformerEncoder(nn.TransformerEncoderLayer(d_model=f, nhead=nhead, dim_feedforward=2*f),num_layers=b)

        self.trunk=nn.ModuleList()
        for i in range(mlpb):
            self.trunk.append(ResnetLayer(f,f))

        self.outputhead=nn.Linear(f, Game_Output_C)

    def prepareInput(self,x):
        batch_size=x.shape[0]
        A = Game_Input_C_Global #全局信息的通道数
        B = Game_Input_C_Card #支援卡参数的通道数
        C = Game_Input_C_Person #larc人头的通道数
        assert(A+7*B+20*C==x.shape[1])

        # 分割张量
        x_A = x[:, :A]
        x_B = x[:, A:A + 7 * B].view(batch_size, 7, B)
        x_C = x[:, A + 7 * B:].view(batch_size, 20, C)

        # 配对B和C
        paired_BCs = []
        # 前6对
        for i in range(7):
            paired_BCs.append(torch.cat((x_B[:, i], x_C[:, i]), dim=1))
        # 剩余的C与零填充的B配对
        zero_B = 0.0*x[:,:B]
        for i in range(7, 20):
            paired_BCs.append(torch.cat((zero_B, x_C[:, i]), dim=1))

        # 合并A
        result = [torch.cat((x_A, paired_BC), dim=1) for paired_BC in paired_BCs]
        result = torch.stack(result, dim=0)
        assert(result.shape[2]==self.headC)
        return result

    def forward(self, x):
        h=self.prepareInput(x)
        h=h.view(-1,self.headC)
        h=self.inputhead(h)
        h=torch.relu(h)
        h=h.view(20,x.shape[0],self.f)

        h=self.transformer_encoder(h)
        h=torch.relu(h)
        h=h.mean(dim=0)

        for block in self.trunk:
            h=block(h)

        return self.outputhead(h)

class Model_EncoderMlpSimple(nn.Module): #简易版transformer+mlp

    def __init__(self,encoderB,encoderF,mlpB,mlpF,globalF):
        super().__init__()
        self.model_type = "ems"
        self.model_param=(encoderB,encoderF,mlpB,mlpF,globalF)
        self.encoderF=encoderF
        self.inputheadGlobal1=nn.Linear(Game_Input_C_Global, globalF, bias=False)
        self.inputheadGlobal2=nn.Linear(globalF, encoderF, bias=False)
        self.inputheadCard=nn.Linear(Game_Input_C_Card, encoderF, bias=False)
        self.inputheadPerson=nn.Linear(Game_Input_C_Person, encoderF, bias=False)

        self.encoderTrunk=nn.ModuleList()
        for i in range(encoderB):
            self.encoderTrunk.append(EncoderLayerSimple(inout_c=encoderF,global_c=globalF))

        self.linBeforeMLP1=nn.Linear(globalF,mlpF, bias=False)
        self.linBeforeMLP2=nn.Linear(encoderF,mlpF, bias=False)

        self.mlpTrunk=nn.ModuleList()
        for i in range(mlpB):
            self.mlpTrunk.append(ResnetLayer(mlpF,mlpF))

        self.outputhead=nn.Linear(mlpF, Game_Output_C)

    def forward(self, x):
        x1=x[:,:Game_Input_C_Global]
        x2=x[:,Game_Input_C_Global:Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card].reshape(-1,Game_Input_C_Card)
        x3=x[:,Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card:].reshape(-1,Game_Input_C_Person)

        gf=torch.relu(self.inputheadGlobal1(x1))
        h=self.inputheadGlobal2(gf).view(-1,1,self.encoderF)+\
          F.pad(self.inputheadCard(x2).view(-1,Game_Card_Num,self.encoderF),(0,0,0,Game_Head_Num-Game_Card_Num,0,0))+\
          self.inputheadPerson(x3).view(-1,Game_Head_Num,self.encoderF)

        h=torch.relu(h)

        for block in self.encoderTrunk:
            h=block(h,gf)

        h=h.mean(dim=1)

        h=self.linBeforeMLP2(h)+self.linBeforeMLP1(gf)
        h=torch.relu(h)

        for block in self.mlpTrunk:
            h=block(h)

        return self.outputhead(h)


class Model_EncoderMlpSimpleBatchnorm(nn.Module): #简易版transformer+mlp

    def __init__(self,encoderB,encoderF,mlpB,mlpF,globalF):
        super().__init__()
        self.model_type = "emsb"
        self.model_param=(encoderB,encoderF,mlpB,mlpF,globalF)
        self.encoderF=encoderF
        self.inputheadGlobal1=LinearBN(Game_Input_C_Global, globalF, bias=False)
        self.inputheadGlobal2=LinearBN(globalF, encoderF, bias=False)
        self.inputheadCard=LinearBN(Game_Input_C_Card, encoderF, bias=False)
        self.inputheadPerson=LinearBN(Game_Input_C_Person, encoderF, bias=False)

        self.encoderTrunk=nn.ModuleList()
        for i in range(encoderB):
            self.encoderTrunk.append(EncoderLayerSimpleBN(inout_c=encoderF,global_c=globalF))

        self.linBeforeMLP1=LinearBN(globalF,mlpF, bias=False)
        self.linBeforeMLP2=LinearBN(encoderF,mlpF, bias=False)

        self.mlpTrunk=nn.ModuleList()
        for i in range(mlpB):
            self.mlpTrunk.append(ResnetLayerBN(mlpF,mlpF))

        self.outputhead=nn.Linear(mlpF, Game_Output_C)

    def forward(self, x):
        x1=x[:,:Game_Input_C_Global]
        x2=x[:,Game_Input_C_Global:Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card].reshape(-1,Game_Input_C_Card)
        x3=x[:,Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card:].reshape(-1,Game_Input_C_Person)

        gf=torch.relu(self.inputheadGlobal1(x1))
        h=self.inputheadGlobal2(gf).view(-1,1,self.encoderF)+\
          F.pad(self.inputheadCard(x2).view(-1,Game_Card_Num,self.encoderF),(0,0,0,Game_Head_Num-Game_Card_Num,0,0))+\
          self.inputheadPerson(x3).view(-1,Game_Head_Num,self.encoderF)

        h=torch.relu(h)

        for block in self.encoderTrunk:
            h=block(h,gf)

        h=h.mean(dim=1)

        h=self.linBeforeMLP2(h)+self.linBeforeMLP1(gf)
        h=torch.relu(h)

        for block in self.mlpTrunk:
            h=block(h)

        return self.outputhead(h)

class Model_EncoderMlpSimpleBatchnorm2(nn.Module): #加了个跳过batchnorm到输出的层

    def __init__(self,encoderB,encoderF,mlpB,mlpF,globalF):
        super().__init__()
        self.model_type = "emsb2"
        self.model_param=(encoderB,encoderF,mlpB,mlpF,globalF)
        self.encoderF=encoderF
        self.skipLinear1=nn.Linear(Game_Input_C_Global, Game_Output_C)
        self.skipLinear2=nn.Linear(globalF, Game_Output_C)
        self.inputheadGlobal1=LinearBN(Game_Input_C_Global, globalF, bias=False)
        self.inputheadGlobal2=LinearBN(globalF, encoderF, bias=False)
        self.inputheadCard=LinearBN(Game_Input_C_Card, encoderF, bias=False)
        self.inputheadPerson=LinearBN(Game_Input_C_Person, encoderF, bias=False)

        self.encoderTrunk=nn.ModuleList()
        for i in range(encoderB):
            self.encoderTrunk.append(EncoderLayerSimpleBN(inout_c=encoderF,global_c=globalF))

        self.linBeforeMLP1=LinearBN(globalF,mlpF, bias=False)
        self.linBeforeMLP2=LinearBN(encoderF,mlpF, bias=False)

        self.mlpTrunk=nn.ModuleList()
        for i in range(mlpB):
            self.mlpTrunk.append(ResnetLayerBN(mlpF,mlpF))

        self.outputhead=nn.Linear(mlpF, Game_Output_C)

    def forward(self, x):
        x1=x[:,:Game_Input_C_Global]
        x2=x[:,Game_Input_C_Global:Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card].reshape(-1,Game_Input_C_Card)
        x3=x[:,Game_Input_C_Global+Game_Card_Num*Game_Input_C_Card:].reshape(-1,Game_Input_C_Person)

        gf=torch.relu(self.inputheadGlobal1(x1))
        h=self.inputheadGlobal2(gf).view(-1,1,self.encoderF)+\
          F.pad(self.inputheadCard(x2).view(-1,Game_Card_Num,self.encoderF),(0,0,0,Game_Head_Num-Game_Card_Num,0,0))+\
          self.inputheadPerson(x3).view(-1,Game_Head_Num,self.encoderF)

        h=torch.relu(h)

        for block in self.encoderTrunk:
            h=block(h,gf)

        h=h.mean(dim=1)

        h=self.linBeforeMLP2(h)+self.linBeforeMLP1(gf)
        h=torch.relu(h)

        for block in self.mlpTrunk:
            h=block(h)

        h=self.outputhead(h) + 0.01*self.skipLinear1(x1)+ 0.05*self.skipLinear2(gf)
        return h


class Model_twolayer(nn.Module):
    #
    def __init__(self, mid_c):  # 1d卷积通道，policy通道，胜负和通道
        super().__init__()
        self.model_type = "tl"
        self.model_param = (mid_c,)
        self.mid_c = mid_c

        self.inputhead = nn.Linear(Game_Input_C, mid_c)
        self.outputhead = nn.Linear(mid_c, Game_Output_C)

    def forward(self, x):
        y=self.inputhead(x)
        y=torch.relu(y)
        y=self.outputhead(y)
        return y

class Model_linear(nn.Module):
    #
    def __init__(self, _):  # 1d卷积通道，policy通道，胜负和通道
        super().__init__()
        self.model_type = "lin"
        self.model_param = (_,)

        self.linear1 = nn.Linear(Game_Input_C, Game_Output_C)

    def forward(self, x):
        y=self.linear1(x)
        return y



ModelDic = {
    "res": Model_ResNet, #带”res“的mlp
    #"tf": Model_Transformer, #transformer
    #"tf2": Model_Transformer2, #多加了一个relu
    "tf": Model_TransformerMlp, #transformer后接mlp
    "tl": Model_twolayer, #2 layer mlp
    "lin": Model_linear, #linear
    "ems": Model_EncoderMlpSimple, #手写极简版transformer+mlp
    "emsb": Model_EncoderMlpSimpleBatchnorm, #手写极简版transformer+mlp
    "emsb2": Model_EncoderMlpSimpleBatchnorm2, #手写极简版transformer+mlp
}
