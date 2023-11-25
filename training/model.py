
import torch
import torch.nn as nn
import numpy as np
from config import *




class ResnetLayer(nn.Module):
    def __init__(self, inout_c, mid_c):
        super().__init__()
        self.lin1 = nn.Linear(inout_c, mid_c)
        self.lin2 = nn.Linear(mid_c, inout_c)

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

class Model_ResNetDP(nn.Module):

    def __init__(self,b,f,dptype,dprate):
        super().__init__()
        self.model_type = "resdp"
        self.model_param=(b,f,dptype,dprate)

        self.inputhead=nn.Linear(Game_Input_C, f)
        self.dropout=nn.Dropout(p=dprate*0.01)
        self.dropoutType=dptype
        self.trunk=nn.ModuleList()
        for i in range(b):
            self.trunk.append(ResnetLayer(f,f))
        self.outputhead=nn.Linear(f, Game_Output_C)

    def forward(self, x):
        if(self.dropoutType==0):
            x=self.dropout(x)
        h=self.inputhead(x)

        h=torch.relu(h)
        if(self.dropoutType==1):
            h=self.dropout(h)

        for block in self.trunk:
            h=block(h)

        return self.outputhead(h)
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
    "res": Model_ResNet, #resnet对照组
    "resdp": Model_ResNetDP, #resnet对照组
    "tl": Model_twolayer,
    "lin": Model_linear,
    #"v2_noOppVCF": Model_v2_noOppVCF,
    #"v3": Model_v3,
}
