from typing import Callable

from umamodels import *

class EffectSummary:
    def __init__(self, summary:str, params_formatter:Callable[[list[int], str], str] = None):
        self._summary = summary
        self._params = None
        self._params_formatter = params_formatter
    
    @property
    def params(self)->list[int]:
        return self._params
    
    @params.setter
    def params(self, value:list[int]):
        self._params = value

    @property
    def summary(self)->str:
        if self._params and self._params_formatter:
            return self._params_formatter(self._params, self._summary)
        return self._summary

    def generateEffect(self)->SupportCardEffect|list[SupportCardEffect]:
        return None

class SingleTypeEffectSummary(EffectSummary):

    def __init__(self, summary: str, value_index:int, effect_type:SupportCardEffectType, params_formatter: Callable[[list[int], str], str] = None):
        super().__init__(summary, params_formatter)
        self._value_index = value_index
        self._effect_type = effect_type
    
    def generateEffect(self):
        return SupportCardEffect(SupportCardEffectType(self._effect_type), self.params[self._value_index])

class MultiyEffectSummary(EffectSummary):
    def __init__(self, summary:str, type_index:int, value_index:int, multiply_index:int, params_formatter:Callable[[list[int], str], str] = None):
        super().__init__(summary, params_formatter=params_formatter)
        self._type_index = type_index
        self._value_index = value_index
        self._multiply_index = multiply_index
        
    def generateEffect(self):
        return SupportCardEffect(SupportCardEffectType(self.params[self._type_index]), self.params[self._value_index] * self.params[self._multiply_index])

# 和UmaSim对接
_support_card_effect_name_mapping = {
    SupportCardEffectType.SpecialTagEffectUp.value : "youQing",
    SupportCardEffectType.MotivationUp.value : "ganJing",
    SupportCardEffectType.TrainingSpeedUp.value : "速度ボーナス",
    SupportCardEffectType.TrainingStaminaUp.value : "持久力ボーナス",
    SupportCardEffectType.TrainingPowerUp.value : "パワーボーナス",
    SupportCardEffectType.TrainingGutsUp.value : "根性ボーナス",
    SupportCardEffectType.TrainingWizUp.value : "賢さボーナス",
    SupportCardEffectType.TrainingEffectUp.value : "xunLian",
    SupportCardEffectType.InitialSpeedUp.value : "初期速度アップ",
    SupportCardEffectType.InitalStaminaUp.value : "初期持久力アップ",
    SupportCardEffectType.InitialPowerUp.value : "初期パワーアップ",
    SupportCardEffectType.InitialGutsUp.value : "初期根性アップ",
    SupportCardEffectType.InitialWizUp.value : "初期賢さアップ",
    SupportCardEffectType.InitalEvaluationUp.value : "initialJiBan",
    SupportCardEffectType.SpeedLimitUp.value : "速度上限アップ",
    SupportCardEffectType.StaminaLimitUp.value : "持久力上限アップ",
    SupportCardEffectType.PowerLimitUp.value : "パワー上限アップ",
    SupportCardEffectType.GutzLimitUp.value : "根性上限アップ",
    SupportCardEffectType.WizLimitUp.value : "賢さ上限アップ",
    SupportCardEffectType.EventRecoveryAmountUp.value : "イベント回復量アップ",
    SupportCardEffectType.TrainingFailureRateDown.value : "failRateDrop",
    SupportCardEffectType.EventEffectUp.value : "イベント効果アップ",
    SupportCardEffectType.TrainingHPConsumptionDown.value : "vitalCostDrop",
    SupportCardEffectType.MinigameEffectUP.value : "ミニゲーム効果アップ",
    SupportCardEffectType.SkillPointBonus.value : "スキルポイントボーナス",
    SupportCardEffectType.WizRecoverUp.value : "wizVitalBonus",
    SupportCardEffectType.RaceStatusUp.value : "saiHou",
    SupportCardEffectType.RaceFanUp.value : "レースファンアップ",
    SupportCardEffectType.SkillTipsLvUp.value : "hintLvUp",
    SupportCardEffectType.SkillTipsEventRateUp.value : "hintProbIncrease",
    SupportCardEffectType.GoodTrainingRateUp.value : "deYiLv",
    41: "全属性Bonus"
}

# 高峰/速善信的技能类型
_support_card_ex_name_mapping = {
    1 : "速度",
    2 : "加速度",
    3 : "体力回复"
}

def _getExEffectName(type:int)->str:
    return _support_card_ex_name_mapping.get(type, "未知")

def _getEffectName(type:int)->str:
    return _support_card_effect_name_mapping.get(type, "未知")

_effectTypeExHandlers:dict[int,tuple[int,EffectSummary]] = {
    101: (2, EffectSummary("羁绊>={}时", 
                                               lambda params, summary: summary.format(params[1]))),
    102: (3, SingleTypeEffectSummary("羁绊>={}且不是擅长训练时，训练效果提高({})", 2, SupportCardEffectType.TrainingEffectUp,
                                               lambda params, summary: summary.format(params[1], params[2]))),
    103: (3, SingleTypeEffectSummary("编入支援卡种类>={}时，训练效果提高 ({})", 2, SupportCardEffectType.TrainingEffectUp,
                                               lambda params, summary: summary.format(params[1], params[2]))),
    104: (3, SingleTypeEffectSummary("最大{}人/每{}粉丝，训练效果+1%(总共+({}))", 2, SupportCardEffectType.TrainingEffectUp,
                                                lambda params, summary: summary.format(params[1]*params[2], params[1], params[2]))),
    105: (3, EffectSummary("根据编成支援卡类型的初始属性加成(对应属性+{}，友人/团队卡全属性+{})",
                                                lambda params, summary: summary.format(params[1], params[2]))),
    106: (4, EffectSummary("友情训练后提升友情加成，最多{}次{} ({})",
                                                lambda params, summary: summary.format(params[1], _getEffectName(params[2]), params[3]))),
    107: (6, EffectSummary("体力越低友情加成越高，基础{}，体力<={}时最大{}友情", lambda params, summary: summary.format(params[5], params[3], params[4]))),
    108: (6, EffectSummary("体力最大值越高训练效果提升越高，基础{}，体力每+4，训练效果+3%，合计+{}",
                                                lambda params, summary: summary.format(params[4], params[5]))),
    109: (3, EffectSummary("支援卡羁绊总和越高训练加成越高(最大{})", lambda params, summary: summary.format(_getEffectName(params[1])))),
    110: (3, EffectSummary("同时训练支援卡数量越多，{}提升越高 ({})",
                                                lambda params, summary: summary.format(_getEffectName(params[1]), params[2]))),
    111: (2, EffectSummary("参与训练的设施等级越高，{}效果越高 (5%/每级)", lambda params, summary: summary.format(_getEffectName(params[1])))),
    112: (2, EffectSummary("参与的训练有{}%概率失败率为0%",lambda params, summary: summary.format(params[1]))),
    113: (1, EffectSummary("参加友情训练时，")),
    114: (4, EffectSummary("剩余体力越多，{}效果越高（最大{}%）", lambda params, summary: summary.format(_getEffectName(params[1]), params[3]))),
    115: (3, EffectSummary("编成支援卡的 {} 提升({})", lambda params, summary: summary.format(_getEffectName(params[1]), params[2]))),
    116: (5, MultiyEffectSummary("根据{}技能个数，提高{}效果(每个+{}, 最大{}个)",2,3,4,params_formatter = lambda params, summary: summary.format(_getExEffectName(params[1]), _getEffectName(params[2]), params[3], params[4]))),
    117: (4, EffectSummary("总训练设施等级越高，{}越高(最高+{})", lambda params, summary: summary.format(_getEffectName(params[1]), params[3]) )),
    118: (3, EffectSummary("羁绊>={}时，可以出现在2个位置", lambda params, summary: summary.format(params[2]) )),
}

def _effectTypeHandler(params:list[int], offset)->tuple[SupportCardEffect, int, EffectSummary]:
    if params and len(params) > offset:
        type = params[offset]
        if 0 < type <= 31:
            return SupportCardEffect(SupportCardEffectType(type), params[offset+1]), 2, None
        elif type in _support_card_effect_name_mapping:
            return None, 2, EffectSummary(f"{_support_card_effect_name_mapping[type]}({params[offset+1]})")
    return None,0, None

def _effectTypeExHandler(params:list[int], offset)->tuple[list[SupportCardEffect], int, EffectSummary]:
    if params and len(params) > offset:
        type = params[offset]
        if type in _effectTypeExHandlers:
            length,summary = _effectTypeExHandlers[type]
            if length > 0:
                summary.params = params[offset:offset+length]
            effect = summary.generateEffect()
            ef = {}
            if effect and isinstance(effect, SupportCardEffect):
                ef[int(effect.type)] = effect.value
            effect = ef
            return effect, length, summary
    return None,0,None

def effectTypeToStr(type: SupportCardEffectType):
    return _getEffectName(type.value)

def supportCardEffectToStr(effect: SupportCardEffect):
    return f"{_getEffectName(effect.type.value)}({effect.value})"

def _interplation(start, end, ratio):
    return start + (end - start) * ratio


def parseEffectRow(row: EffectRow)->list[int]:
    #key: 0 5 10 15 20 25 30 35 40 45 50
    row = list(row)
    stack = []
    for i, v in enumerate(row):
        if v != -1:
            stack.append((i, v))
    ret = [0] * 11
    prev_index, prev = stack[-1]
    for i in range(10, -1, -1):
        if not stack:
            ret[i] = 0
            continue
        index, value = stack[-1]
        if i > prev_index:
            ret[i] = prev
        elif i == index:
            ret[i] = value
            prev_index, prev = stack.pop()
        else:
            ret[i] = int(_interplation(value, prev, (i-index) / (prev_index - index)))
    return ret    

def parseUniqueEffectRow(row:UniqueEffectRow)->tuple[str, list[SupportCardEffect], int]:
    level = row.lv
    row = list(row)[1:]
    effects = []
    ret_str = ""
    uniqueType = None
    uniqueParams = None
    for i in range(2):
        params = row[i*6:(i+1)*6]
        size = len(params)
        index = 0
        if ret_str and params[index] != 0:
            ret_str += ","
        while index < size:
            # zero check
            if params[index] == 0:
                break          

            start_index = index
            # 解析固有词条
            effect_list, length, summary = _effectTypeExHandler(params, index)
            if effect_list:
                for k, v in effect_list.items():
                    effects.append(int(k))
                    effects.append(v)
            if params[0] >= 100:    # 记录固有参数
                uniqueType = params[0]
                uniqueParams = params
            if summary:
                ret_str += summary.summary
            index += length
            # 解析固有词条中的基础面板属性
            effect, length, summary = _effectTypeHandler(params, index)
            if effect:
                #effects[effect.type] = effect.value
                effects.append(int(effect.type))
                effects.append(effect.value)
                ret_str += supportCardEffectToStr(effect)
            elif summary:
                # 仅速神鹰有该附言，特判一下
                ret_str += summary.summary
                #effects["AllStatusBonus"] = 1
                effects.append(41)
                effects.append(1)
            index += length
            if start_index == index:
                print(f"↑未知固有效果: {params} {start_index} {index} (skipped), maybe encounter new effect type?")
                break
    return dict(summary=ret_str, effect=effects, unlockLevel=level, type=uniqueType, uniqueParams=uniqueParams)
