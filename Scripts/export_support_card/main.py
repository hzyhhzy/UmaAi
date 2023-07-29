#encoding: utf-8

from umadb import * 
from umamodels import * 
from support_card_helper import *
import net_utils, translators, sys, re
import jsons, codecs

ShortType = ['', '速', '耐', '力', '根', '智', '友', '团']
DirectKeys = ['deYiLv', 'failRateDrop', 'ganJing', 'hintProbIncrease',
              'saiHou', 'vitalCostDrop', 'wizVitalBonus', 'xunLian',
              'youQing', 'initialJiBan']
InitialKeys = {
    "初期速度アップ": 0,
    "初期持久力アップ": 1,
    "初期パワーアップ": 2,
    "初期根性アップ": 3,
    "初期賢さアップ": 4
}
BonusKeys = {
    "速度ボーナス": 0,
    "持久力ボーナス": 1,
    "パワーボーナス": 2,
    "根性ボーナス": 3,
    "賢さボーナス": 4,
    "スキルポイントボーナス": 5
}

def short_name(card):
    return re.sub(r"\[.*\]", "[%s]" % ShortType[card.type], card.original_name)

db = Umadb('master.mdb')
trans = translators.TrainersLegendTranslator(None)

card_list = db.get_all_support_card_data()
result = {}

for card in card_list:
    # 调用github TLG翻译
    trans_card = trans.translate_support_card(card)
    print(trans_card.name)
    
    # 处理效果
    effect = {}
    unique_effect = None
    effect_range = 3 + card.rarity

    if card.unique_effect:
        unique_effect = parseUniqueEffectRow(card.unique_effect)
    if card.effect_row_dict:    
        for tp, row in card.effect_row_dict.items():
            type_name = effectTypeToStr(tp)
            type_data = parseEffectRow(row)
            effect[type_name] = type_data[effect_range : effect_range+5]

    # 整理
    trans_card.unique_effect = unique_effect
    trans_card.effect_row_dict = effect
    trans_card.original_name = trans_card.name
    trans_card.name = short_name(trans_card)
    
    # 转换成UmaSim格式
    ucard = dict(
        cardId = trans_card.id,
        cardName = trans_card.name,
        fullName = trans_card.original_name,
        rarity = trans_card.rarity.value,
        cardSkill = dict(
            SkillList = trans_card.train_skill_list,
            skillNum = len(trans_card.train_skill_list)
        ),
        cardType = trans_card.type.value,
        cardValue = [],
        uniqueEffect = trans_card.unique_effect
    )
    for i in range(0, 5):
        d = dict(
            filled = True,
            bonus = [0, 0, 0, 0, 0, 0],
            initialBonus = [0, 0, 0, 0, 0, 0]
        )
        for key, value in trans_card.effect_row_dict.items():
            if key in DirectKeys:
                d[key] = value[i]
            elif '初期' in key:
                d['initialBonus'][InitialKeys[key]] = value[i]
            elif 'ボーナス' in key:
                d['bonus'][BonusKeys[key]] = value[i]
        ucard["cardValue"].append(d)
    result[trans_card.id] = ucard
    
with codecs.open('card/card.json', 'w', encoding='utf-8') as f:
    f.write(jsons.dumps(result, jdkwargs=dict(ensure_ascii=False, indent=2)))
    f.write("\n")
    