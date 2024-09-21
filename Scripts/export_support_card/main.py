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

# 特判直接合并面板（得意率）的卡
# 智太阳神，活动力乌拉拉，东商
DirectMergeCards = [ 30155, 30171, 30128 ]

HintValues = [
    [6, 0, 2, 0, 0, 0],
    [0, 6, 0, 2, 0, 0],
    [0, 2, 6, 0, 0, 0],
    [1, 0, 1, 6, 0, 0],
    [2, 0, 0, 0, 6, 0]
]

InitialKeys = {
    "初期速度アップ": 0,
    "初期持久力アップ": 1,
    "初期パワーアップ": 2,
    "初期根性アップ": 3,
    "初期賢さアップ": 4,
    "初期PTアップ": 5
}
BonusKeys = {
    "速度ボーナス": 0,
    "持久力ボーナス": 1,
    "パワーボーナス": 2,
    "根性ボーナス": 3,
    "賢さボーナス": 4,
    "スキルポイントボーナス": 5
}

RaceConditionIds = {
    700: "Pre-OP",
    100: "G1",
    300: "G3"
}

# 生涯比赛会改变的情况（能不比就不比）
# 回合数已经-1了
SpecialRaces = {
    1005: {
        "races": [32, 45],
        "note": "富士 - 选择英里路线"
    },
    1009: {
        "races": [33],
        "note": "大和 - 比赛回合不变"
    },
    1016: {
        "races": [55, 67, 69, 71],
        "note": "白仁 - 比赛回合不变"
    },
    1022: {
        "freeRaces": [
            {
                "startTurn": 67,
                "endTurn": 71,
                "count": 1
            }
        ],
        "note": "美妙 - 自选最终比赛"
    },
    1031: {
        "freeRaces": [
            {
                "startTurn": 69,
                "endTurn": 71,
                "count": 1
            }
        ],
        "note": "风神 - 自选最终比赛"
    },
    1032: {
        "races": [33],
        "note": "速子 - 设置为日本德比（不触发低心情改赛）"
    },
    1056: {
        "races": [29],
        "freeRaces": [
            {
                "startTurn": 60,
                "endTurn": 63,
                "count": 1
            }
        ],
        "note": "福来 - 抽签比赛，设置为第二年3月下，第三年夏季自选比赛"
    },
    1062: {
        "note": "诗歌剧 - 设置为不比日本杯（不流鼻血要扣分）"
    },
    1071: {
        "races": [43],
        "note": "阿尔丹 - 比赛回合不变"
    },
    1069: {
        "races": [58],
        "note": "千代 - 设置为安田纪念"
    },
    1093: {
        "races": [41],
        "note": "凯斯 - 比赛改为休息"
    },
    1109: {
        "races": [32],
        "note": "莱茵 - 设置为NHK"
    },
    1116: {
        "races": [71],
        "note": "贵妇人 - 增加有马纪念"
    },
    1121: {
        "races": [41],
        "note": "圣剑 - 不参加短距锦标"
    }
}

def short_name(card):
    return re.sub(r"\[.*\]", "[%s]" % ShortType[card.type], card.original_name)

def file_name(card):
    return re.sub(r"[\.\*\?\/\\\&\$\:]", "", card.original_name) + ".json"

db = Umadb('master.mdb')
trans = translators.TrainersLegendTranslator(None)

card_list = db.get_all_support_card_data()
uma_list = db.get_all_character_card_data()
result = {}

def mergeCardEffect(cardValue, effect):
    ret = dict(cardValue)

    for i in range(0, int(len(effect)/2)):
        key, value = effect[i*2], effect[i*2+1]
        if key == 41: # 神鹰特判
            for j in range(0, 5):
                ret["bonus"][j] = cardValue["bonus"][j] + 1
        elif key == 1:  # 友情
            ret["youQing"] = (100.0 + value) * (100 + cardValue.get("youQing", 0)) / 100.0 - 100
        elif key == 2: # 干劲
            ret["ganJing"] = cardValue.get("ganJing", 0) + value
        elif key == 19:    # 得意率
            ret["deYiLv"] = (100.0 + value) * (100 + cardValue.get("deYiLv", 0)) / 100.0 - 100
        elif key == 8: # 训练
            ret["xunLian"] = cardValue.get("xunLian", 0) + value
        elif key == 15:    # 赛后
            ret["saiHou"] = cardValue.get("saiHou", 0) + value
        elif key == 27:    # 失败率-减乘
            ret["failRateDrop"] = 100 - (100 - cardValue.get("failRateDrop", 0)) * (100 - value) / 100
        elif key == 28:    # 体力消耗-减乘
            ret["vitalCostDrop"] = 100 - (100 - cardValue.get("vitalCostDrop", 0)) * (100 - value) / 100
        elif key == 31:    # 回体增加-诗歌剧
            ret["wizVitalBonus"] = cardValue.get("wizVitalBonus", 0) + value
        elif key >= 3 and key <= 7:   # 副属性
            which = key-3
            ret["bonus"][which] = cardValue["bonus"][which] + value
        elif key == 30:    # 技能点
            ret["bonus"][5] = cardValue["bonus"][5] + value
        elif key >= 9 and key <= 13:  # 初始属性
            which = key-9
            ret["initialBonus"][which] = cardValue["initialBonus"][which] + value
        elif key == 14:    # 初始羁绊
            ret["initialJiBan"] = cardValue.get("initialJiBan", 0) + value
        elif key == 17:   # hint等级
            ret["hintBonus"][5] += value * 5
        elif key == 18: # hint率：不处理
            pass
        else:
            print(f"未知数值词条 {key} = {value}")
#             // 忽略的属性
#         case (int)UniqueEffectType::RaceFanUp:   // 粉丝
#         case (int)UniqueEffectType::SkillTipsEventRateUp:    // Hint率         
    return ret


def prepareUniqueEffect(card, ueffect):
    utype = 0
    useParam = False
    if (not ueffect["type"]) or (card["cardId"] in DirectMergeCards):
         # 默认固有或者特判直接合并的卡（智太阳神）
        utype = 0
        for which in range(0, 5):
            card["cardValue"][which] = mergeCardEffect(card["cardValue"][which], ueffect["effect"])
    elif ueffect["type"] == 101:
        # 把CY的固有编号转换为AI的
        jiBan = ueffect["uniqueParams"][1]
        if jiBan == 80:
            utype = 1
        elif jiBan == 100:
            utype = 2
        else:
            print(f"新的羁绊型固有: {card['cardName']} - {jiBan}")
        useParam = True
    else:
        # 其他固有，只复制参数和映射ID
        if ueffect["type"] == 102:
            utype = 3
            useParam = True
        elif ueffect["type"] == 103:
            utype = 21   # 改为21号固有，参见文档
            ueffect["uniqueParams"][3] = ueffect["uniqueParams"][2]
            ueffect["uniqueParams"][2] = 8  # 调整参数顺序，方便AI处理
            useParam = True
        else:
            if ueffect["type"] <= 120:
                utype = ueffect["type"] - 100    # 直接映射
            else:
                utype = ueffect["type"] - 99     # 121 -> 22
            useParam = True        

    card["uniqueEffectType"] = utype
    card["uniqueEffectSummary"] = ueffect["summary"]
    if useParam:
        card["uniqueEffectParam"] = ueffect["uniqueParams"]

# 支援卡解析
def parseSupportCard():
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
            cardId = int(trans_card.id),
            charaId = trans_card.chara_id,
            cardName = trans_card.name,
            fullName = trans_card.original_name,
            rarity = trans_card.rarity.value,
        # cardSkill = list(map(lambda x: int(x), trans_card.train_skill_list)),
            cardType = trans_card.type.value - 1, # 从0开始
            cardValue = []
            #uniqueEffect = trans_card.unique_effect
        )
        for i in range(0, 5):
            d = dict(
                filled = True,
                bonus = [0, 0, 0, 0, 0, 0],
                initialBonus = [0, 0, 0, 0, 0, 0],
                hintBonus = [0, 0, 0, 0, 0, 5], # deprecated
                hintLevel = 0
            )
            for key, value in trans_card.effect_row_dict.items():
                if key in DirectKeys:
                    d[key] = value[i]
                elif key == "hintLvUp":
                    d["hintBonus"][5] += 5 * value[i]
                    d["hintLevel"] = value[i] + 1
                elif '初期' in key:
                    d['initialBonus'][InitialKeys[key]] = value[i]
                elif 'ボーナス' in key:
                    d['bonus'][BonusKeys[key]] = value[i]
            ucard["cardValue"].append(d)
        # 对UniqueEffect字段进行进一步预处理
        if trans_card.unique_effect:
            prepareUniqueEffect(ucard, trans_card.unique_effect)
        if len(trans_card.train_skill_list) == 0 and ucard["cardType"] < 5:
            # 无技能的卡，把hint技能点换为属性
            for i in range(0, 5):
                ucard["cardValue"][i]["hintBonus"] = HintValues[ucard["cardType"]]
                ucard["cardValue"][i]["hintLevel"] = 0
            print(f"hintBonus = {HintValues[ucard['cardType']]}")
        result[int(trans_card.id)] = ucard
    return result

# 马娘解析
def parseUma():
    result = {}
    for uma in uma_list:
        # 调用github TLG翻译
        trans_uma = trans.translate_chara_card(uma)
        print(trans_uma.id, trans_uma.name)

        fiveStatusBonus = [
            uma.talent_info.speed,
            uma.talent_info.stamina,
            uma.talent_info.power,
            uma.talent_info.guts,
            uma.talent_info.wiz
        ]
        fiveStatusInitial = [
            uma.status_set[5].speed,
            uma.status_set[5].stamina,
            uma.status_set[5].power,
            uma.status_set[5].guts,
            uma.status_set[5].wiz
        ]

        # 生涯比赛
        chara_id = int(int(uma.id) / 100)
        races = []
        freeRaces = []
        last_race = -1
        for race in uma.route_races:
            if race.condition_type == 1:
                # 固定比赛
                if race.turn not in races:
                    if race.determine_race != 0:
                        if chara_id not in SpecialRaces:
                            print(f"** 回合 {race.turn} 有特殊生涯比赛，需要手动添加")
                    else:
                        races.append(race.turn - 1)
            elif race.condition_type == 2:
                # 自选比赛
                grade = RaceConditionIds[race.condition_id]
                rank = race.condition_value_1
                count = race.condition_value_2
                print(f"  回合 {last_race+1} -> {race.turn} 在 {count} 场 {grade} 中取得 {rank} 名以内")
                freeRaces.append({
                    "startTurn": last_race,
                    "endTurn": race.turn-1,
                    "count": count
                })
            elif race.condition_type == 3:
                # 粉丝数
                print(f"  回合 {last_race+1} -> {race.turn} 取得 {race.condition_value_1} 粉丝")
                freeRaces.append({
                    "startTurn": last_race,
                    "endTurn": race.turn-1,
                    "count": 1
                })
            last_race = race.turn
        
        # 特殊生涯比赛
        if chara_id in SpecialRaces:
            entry = SpecialRaces[chara_id]
            if "races" in entry:
                races.extend(entry["races"])
            if "freeRaces" in entry:
                freeRaces.extend(entry["freeRaces"])
            if "note" in entry:
                print("** 特殊生涯比赛: " + entry["note"])

        races.sort()
        freeRaces.sort(key=lambda x: x["startTurn"])

        # 返回
        result[uma.id] = dict(
            bonusData=[],
            fiveStatusBonus=fiveStatusBonus,
            fiveStatusInitial=fiveStatusInitial,
            freeRaces=freeRaces,
            gameId=int(uma.id),
            name=trans_uma.name,
            preferRaces=[],
            preferReds=[],
            races=races,
            star=5
        )
    return result

if __name__ == "__main__":
    cardDB = parseSupportCard()
    umaDB = parseUma()

    with codecs.open('cardDB.json', 'w', encoding='utf-8') as f:
        f.write(jsons.dumps(cardDB, strip_nulls=True, jdkwargs=dict(ensure_ascii=False, indent=2, skipkeys=True)))
        f.write("\n")
    with codecs.open('umaDB.json', 'w', encoding='utf-8') as f:
        f.write(jsons.dumps(umaDB, strip_nulls=True, jdkwargs=dict(ensure_ascii=False, indent=2, skipkeys=True)))
        f.write("\n")