from typing import Callable

from umamodels import *

class EffectSummary:
    def __init__(self, summary:str, params_formatter:Callable[[list[int]], str] = None):
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

    def generateEffect(self):
        return None

class SingleTypeEffectSummary(EffectSummary):

    def __init__(self, summary: str, value_index:int, effect_type:SupportCardEffectType, params_formatter: Callable[[list[int]], str] = None):
        super().__init__(summary, params_formatter)
        self._value_index = value_index
        self._effect_type = effect_type
    
    def generateEffect(self):
        return SupportCardEffect(SupportCardEffectType(self._effect_type), self.params[self._value_index])

class MultiyEffectSummary(EffectSummary):
    def __init__(self, summary:str, type_index:int, value_index:int, multiply_index:int, params_formatter:Callable[[list[int]], str] = None):
        super().__init__(summary, params_formatter=params_formatter)
        self._type_index = type_index
        self._value_index = value_index
        self._multiply_index = multiply_index
        
    def generateEffect(self):
        return SupportCardEffect(SupportCardEffectType(self.params[self._type_index]), self.params[self._value_index] * self.params[self._multiply_index])

# 和UmaSim对接
_support_card_name_mapping = {
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
    SupportCardEffectType.EventEffetcUp.value : "イベント効果アップ",
    SupportCardEffectType.TrainingHPConsumptionDown.value : "vitalCostDrop",
    SupportCardEffectType.MinigameEffectUP.value : "ミニゲーム効果アップ",
    SupportCardEffectType.SkillPointBonus.value : "スキルポイントボーナス",
    SupportCardEffectType.WizRecoverUp.value : "wizVitalBonus",
    SupportCardEffectType.RaceStatusUp.value : "saiHou",
    SupportCardEffectType.RaceFanUp.value : "レースファンアップ",
    SupportCardEffectType.SkillTipsLvUp.value : "スキルチップレベルアップ",
    SupportCardEffectType.SkillTipsEventRateUp.value : "hintProbIncrease",
    SupportCardEffectType.GoodTrainingRateUp.value : "deYiLv",
}

_support_card_ex_name_mapping = {
    1 : "速度が上がる",
    3 : "持久力が回復する"
}

def _getExEffectName(type:int)->str:
    return _support_card_ex_name_mapping.get(type, "未知")

def _getEffectName(type:int)->str:
    return _support_card_name_mapping.get(type, "未知")

_effectTypeExHandlers:dict[int,tuple[int,EffectSummary]] = {
    101: (2, EffectSummary("絆ゲージが{}以上の時", 
                                               lambda params, summary: summary.format(params[1]))),
    102: (3, SingleTypeEffectSummary("絆ゲージが{}以上かつ得意トレーニングでない時、トレーニング効果アップ ({})", 2, SupportCardEffectType.TrainingEffectUp,
                                               lambda params, summary: summary.format(params[1], params[2]))),
    103: (3, SingleTypeEffectSummary("編成しているサポートカードのタイプが{}種類以上ならトレーニング効果アップ ({})", 2, SupportCardEffectType.TrainingEffectUp,
                                               lambda params, summary: summary.format(params[1], params[2]))),
    104: (3, SingleTypeEffectSummary("最大{}人ファンまで各{}人ファンでトレーニング効果アップ (1)を獲得 (合計トレーニング効果アップ ({}))", 2, SupportCardEffectType.TrainingEffectUp,
                                                lambda params, summary: summary.format(params[1]*params[2], params[1], params[2]))),
    105: (3, EffectSummary("編成しているサポートカードのタイプに応じて初期基礎能力アップ ({})、友情かグループタイプの場合で各初期礎能力アップ ({})",
                                                lambda params, summary: summary.format(params[1], params[2]))),
    106: (4, EffectSummary("このカードと友情トレーニングをする度に最大{}回まで{} ({})",
                                                lambda params, summary: summary.format(params[1], _getEffectName(params[2]), params[3]))),
    107: (6, EffectSummary("現在の体力が少ないほど、友情ボーナス")),
    108: (6, EffectSummary("体力最大値が高いほど、トレーニング効果アップ")),
    109: (3, EffectSummary("編成したサポートカードの絆ゲージの合計が高いほどトレーニング効果アップ")),
    110: (3, EffectSummary("同じトレーニングに参加したサポートカードが多いほど{} ({})",
                                                lambda params, summary: summary.format(_getEffectName(params[1]), params[2]))),
    111: (2, EffectSummary("参加したトレーニングのトレーニングLvが高いほどトレーニング効果アップ ({})", 
                                               lambda params, summary: summary.format(params[1]))),
    112: (2, EffectSummary("{}%の確率で参加したトレーニングの失敗率が0%になることがある",lambda params, summary: summary.format(params[1]))),
    113: (1, EffectSummary("友情トレーニングが発生しているトレーニングに参加した場合")),
    114: (4, EffectSummary("体力が多いほど、トレーニング効果アップが増える")),
    115: (3, EffectSummary("編成したサポートカードの{} ({})", lambda params, summary: summary.format(_getEffectName(params[1]), params[2]))),
    116: (5, MultiyEffectSummary("最大{}個まで{}スキルの所持数に応じて{}効果アップ({})",2,3,4,params_formatter = lambda params, summary: summary.format(params[4], _getExEffectName(params[1]), _getEffectName(params[2]), params[3]))),
}

def _effectTypeHandler(params:list[int], offset)->tuple[SupportCardEffect, int]:
    if params and len(params) > offset:
        type = params[offset]
        if 0 < type <= 31:
            return SupportCardEffect(SupportCardEffectType(type), params[offset+1]), 2
    return None,0

def _effectTypeExHandler(params:list[int], offset)->tuple[list[SupportCardEffect], int, EffectSummary]:
    if params and len(params) > offset:
        type = params[offset]
        if type in _effectTypeExHandlers:
            length,summary = _effectTypeExHandlers[type]
            if length > 0:
                summary.params = params[offset:offset+length]
            effect = summary.generateEffect()
            if effect and isinstance(effect, SupportCardEffect):
                effect = [effect]
            return effect, length, summary
    return None,0,None

def effectTypeToStr(type: SupportCardEffectType):
    return _getEffectName(type.value)

def supportCardEffectToStr(effect: SupportCardEffect):
    return f"{_getEffectName(effect.type.value)}{effect.value}"

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
    for i in range(2):
        if ret_str:
            ret_str += ","
        params = row[i*6:(i+1)*6]
        size = len(params)
        index = 0
        while index < size:
            # zero check
            if params[index] == 0:
                break
            start_index = index
            effect_list, length, summary = _effectTypeExHandler(params, index)
            if effect_list:
                effects += effect_list
            if summary:
                ret_str += summary.summary
            index += length
            effect, length = _effectTypeHandler(params, index)
            if effect:
                effects.append(effect)
                ret_str += supportCardEffectToStr(effect)
            index += length
            if start_index == index:
                print(f"跳过未知固有效果: {params} {start_index} {index}?")
                break
    return ret_str, effects, level
