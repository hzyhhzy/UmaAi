import sqlite3
from typing import Generator
from dataclasses import dataclass, asdict
import collections

from umamodels import *

_TABLE_SKILL_DATA = 'skill_data'
_TABLE_TEXT_DATA = 'text_data'
_TABLE_CARD_RARITY_DATA = 'card_rarity_data'
_TABLE_CARD_DATA = 'card_data'
_TABLE_SKILL_SET = 'skill_set'
_TABLE_AVAILABLE_SKILL_SET = "available_skill_set"
_TABLE_SUPPORT_CARD_DATA = "support_card_data"
_TABLE_SUPPORT_CARD_EFFECT_TABLE = "support_card_effect_table"
_TABLE_SUPPORT_CARD_UNIQUE_EFFECT_TABLE = "support_card_unique_effect"
_TABLE_SINGLE_MODE_HINT_GAIN = "single_mode_hint_gain"
_TABLE_SKILL_UPGRADE_DESCRIPTION = "skill_upgrade_description"

_TEXT_SKILL_NAME = 47
_TEXT_SKILL_DESCRIPTION = 48
_TEXT_CARD_NAME = 4
_TEXT_SUPPORT_CARD_NAME = 75

@dataclass
class _TextData:
    id: int
    category: int
    index: int
    text: str


@dataclass
class _CardData:
    id: int
    chara_id: int
    default_rarity: int
    limited_chara: int
    skill_set_id: int
    talent_speed: int
    talent_stamina: int
    talent_power: int
    talent_guts: int
    talent_wiz: int
    talent_group_id: int
    bg_id: int
    get_piece_id: int
    running_style: int


@dataclass
class _CardRarityData:
    id: int
    card_id: int
    rarity: int
    race_dress_id: int
    skill_set_id: int
    speed: int
    stamina: int
    power: int
    guts: int
    wiz: int
    max_speed: int
    max_stamina: int
    max_pow: int
    max_guts: int
    max_wiz: int
    proper_distance_short: int
    proper_distance_mile: int
    proper_distance_middle: int
    proper_distance_long: int
    proper_running_style_nige: int
    proper_running_style_senko: int
    proper_running_style_sashi: int
    proper_running_style_oikomi: int
    proper_ground_turf: int
    proper_ground_dirt: int
    get_dress_id_1: int
    get_dress_id_2: int


@dataclass
class _SkillSet:
    id: int
    skill_id1: int
    skill_level1: int
    skill_id2: int
    skill_level2: int
    skill_id3: int
    skill_level3: int
    skill_id4: int
    skill_level4: int
    skill_id5: int
    skill_level5: int
    skill_id6: int
    skill_level6: int
    skill_id7: int
    skill_level7: int
    skill_id8: int
    skill_level8: int
    skill_id9: int
    skill_level9: int
    skill_id10: int
    skill_level10: int
    skill_id11: int
    skill_level11: int
    skill_id12: int
    skill_level12: int
    skill_id13: int
    skill_level13: int
    skill_id14: int
    skill_level14: int
    skill_id15: int
    skill_level15: int
    skill_id16: int
    skill_level16: int
    skill_id17: int
    skill_level17: int
    skill_id18: int
    skill_level18: int
    skill_id19: int
    skill_level19: int
    skill_id20: int
    skill_level20: int

@dataclass
class _AvailableSkillSet:
    id: int
    skill_set_id: int
    skill_id:int
    need_rank:int

@dataclass
class _SupportCardData:
    id: int
    chara_id: int
    rarity: int
    exchange_item_id: int
    effect_table_id: int
    unique_effect_id: int
    command_type: int
    command_id: int
    support_card_type: int
    skill_set_id: int
    detail_pos_x: int
    detail_pos_y: int
    detail_scale: int
    detail_rot_z: int
    start_date: int
    outing_max: int
    effect_id: int

@dataclass
class _SupportCardEffectTable:
    id: int
    type: int
    init: int
    limit_lv5: int
    limit_lv10: int
    limit_lv15: int
    limit_lv20: int
    limit_lv25: int
    limit_lv30: int
    limit_lv35: int
    limit_lv40: int
    limit_lv45: int
    limit_lv50: int

@dataclass
class _SupportCardUniqueEffect:
    id: int
    lv: int
    type_0: int
    value_0: int
    value_0_1: int
    value_0_2: int
    value_0_3: int
    value_0_4: int
    type_1: int
    value_1: int
    value_1_1: int
    value_1_2: int
    value_1_3: int
    value_1_4: int

@dataclass
class _SingleModeHintGain:
    id: int
    hint_id: int
    support_card_id: int
    hint_group: int
    hint_gain_type: int
    hint_value_1: int
    hint_value_2: int
    group_id: int
    condition_set_id: int
    priority: int

@dataclass
class _SkillUpgradeDescription:
    id: int
    card_id: int
    rank: int
    skill_id: int
    start_date: int

class Umadb:
    def __init__(self, dbpath: str) -> None:
        self.dbpath = dbpath
        self._text_data = {}

    def __enter__(self) -> sqlite3.Connection:
        self.conn = sqlite3.connect(self.dbpath)
        return self.conn

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.conn.close()

    def create_table(self, table_name: str, columns: dict) -> None:
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(
                f'CREATE TABLE {table_name} ({", ".join([f"{key} {value}" for key, value in columns.items()])})')
            conn.commit()

    def insert(self, table_name: str, values: dict) -> None:
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(
                f'INSERT INTO {table_name} ({", ".join(values.keys())}) VALUES ({", ".join(["?" for _ in values.values()])})', tuple(values.values()))
            conn.commit()

    def select(self, table_name: str, columns: tuple, where: dict = None) -> list:
        with self as conn:
            cursor = conn.cursor()
            if where is None:
                cursor.execute(
                    f'SELECT {", ".join(columns)} FROM {table_name}')
            else:
                cursor.execute(
                    f'SELECT {", ".join(columns)} FROM {table_name} WHERE {where["column"]} = ?', (where["value"],))
            return cursor.fetchall()

    def update(self, table_name: str, values: dict, where: dict) -> None:
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(f'UPDATE {table_name} SET {", ".join([f"{key} = ?" for key in values.keys()])} WHERE {where["column"]} = ?', tuple(
                values.values()) + (where["value"],))
            conn.commit()

    def delete(self, table_name: str, where: dict) -> None:
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(
                f'DELETE FROM {table_name} WHERE {where["column"]} = ?', (where["value"],))
            conn.commit()

    def drop_table(self, table_name: str) -> None:
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(f'DROP TABLE {table_name}')
            conn.commit()

    def _get_all_text_data(self) -> list[dict]:
        if self._text_data:
            return self._text_data
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(
                f'SELECT * FROM {_TABLE_TEXT_DATA}')
            _dict = collections.defaultdict(dict)
            for info in cursor.fetchall():
                text_data = _TextData(*info)
                _dict[text_data.category][text_data.index] = text_data.text
            self._text_data = _dict
        return self._text_data

    def get_all_skill_data(self) -> Generator[Skill, None, None]:
        with self as conn:
            cursor = conn.cursor()
            _text = _TABLE_TEXT_DATA
            _skill = _TABLE_SKILL_DATA
            cursor.execute(
                f'SELECT {_text}.[text], {_text}.category, {_skill}.* FROM {_text} INNER JOIN {_skill} \
                    ON {_text}.[index] = {_skill}.id WHERE {_text}.category IN ({_TEXT_SKILL_NAME},{_TEXT_SKILL_DESCRIPTION}) ORDER BY {_skill}.id, {_text}.category')
            while (row := cursor.fetchone()) is not None:
                description = cursor.fetchone()[0]
                skillDataList = []
                for i in range(2):
                    left = 16 + i*26
                    if not row[left + 1]:
                        break
                    effectList = []
                    for i in range(3):
                        start = left+5+i*7
                        if row[start] != 0:
                            effectList.append(
                                SkillEffect(*row[start:start + 7]))
                    skillDataList.append(
                        SkillData(*row[left:left+5], effectList))
                rarity = row[3]
                if rarity == 1:
                    rarity = SkillRarity.Normal
                elif rarity == 2:
                    rarity = SkillRarity.Rare
                elif 3 <= rarity <= 5:
                    rarity = SkillRarity.Unique
                elif rarity == 6:
                    rarity = SkillRarity.Upgrade
                else:
                    rarity = SkillRarity.Normal
                skill = Skill(str(row[2]), row[0], description, str(row[-7]), str(row[4]), rarity, skillDataList)
                if row[10] > 0:
                    skill.unique_skill_ids = [row[10]]
                    if row[11] > 0:
                        skill.unique_skill_ids.append(row[11])
                yield skill

    def get_all_character_card_data(self) -> list[CharacterCard]:
        text_dict = self._get_all_text_data()
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(f'SELECT * FROM {_TABLE_CARD_DATA} WHERE default_rarity != 0')
            card_data = [_CardData(*row) for row in cursor.fetchall()]
            cursor.execute(f'SELECT * FROM {_TABLE_CARD_RARITY_DATA}')
            card_rarity_data = [_CardRarityData(*row) for row in cursor.fetchall()]
            cursor.execute(f'SELECT * FROM {_TABLE_SKILL_SET}')
            skill_set_dict = {row[0]: _SkillSet(*row) for row in cursor.fetchall()}
            cursor.execute(f'SELECT * FROM {_TABLE_AVAILABLE_SKILL_SET}')
            available_skill_set_data = [_AvailableSkillSet(*row) for row in cursor.fetchall()]
            available_skill_set_dict = collections.defaultdict(lambda:collections.defaultdict(list))
            for available_skill_set in available_skill_set_data:
                available_skill_set_dict[available_skill_set.skill_set_id][available_skill_set.need_rank].append(str(available_skill_set.skill_id))
            cursor.execute(f'SELECT * FROM {_TABLE_SKILL_UPGRADE_DESCRIPTION}')
            skill_upgrade_description_data = [_SkillUpgradeDescription(*row) for row in cursor.fetchall()]
            skill_upgrade_description_dict = collections.defaultdict(lambda:collections.defaultdict(list))
            for desc in skill_upgrade_description_data:
                skill_upgrade_description_dict[desc.card_id][desc.rank].append(str(desc.skill_id))
        card_dict = {}
        name_dict = text_dict[_TEXT_CARD_NAME]
        for card in card_data:
            chara_card = CharacterCard(str(card.id), name_dict[card.id], str(card.bg_id), Talent(
                card.talent_speed, card.talent_stamina, card.talent_power, card.talent_guts, card.talent_wiz))
            chara_card.available_skill_set = available_skill_set_dict[card.skill_set_id]
            chara_card.upgrade_skill_set = skill_upgrade_description_dict[card.id]
            card_dict[card.id] = chara_card
        for card in card_rarity_data:
            chara_card: CharacterCard = card_dict[card.card_id]
            chara_card.status_set[card.rarity] = Status(
                card.speed, card.stamina, card.power, card.guts, card.wiz)
            chara_card.proper_set[card.rarity] = Proper(card.proper_distance_short, card.proper_distance_mile, card.proper_distance_middle, card.proper_distance_long,
                                                        card.proper_running_style_nige, card.proper_running_style_senko, card.proper_running_style_sashi, card.proper_running_style_oikomi,
                                                        card.proper_ground_turf, card.proper_ground_dirt)
            skill_set:_SkillSet = skill_set_dict[card.skill_set_id]
            chara_card.rairty_skill_set[card.rarity] = [ str(id) for i in range(10) if (id := getattr(skill_set,f"skill_id{i+1}")) > 0]
        return list(card_dict.values())
    
    def _get_support_card_type(self, card:_SupportCardData)->SupportCardType:
        if card.support_card_type == 1:
            if card.command_id == 101:
                return SupportCardType.Speed
            elif card.command_id == 102:
                return SupportCardType.Power
            elif card.command_id == 103:
                return SupportCardType.Guts
            elif card.command_id == 105:
                return SupportCardType.Stamina
            elif card.command_id == 106:
                return SupportCardType.Wiz
        elif card.support_card_type == 2:
            return SupportCardType.Friend
        elif card.support_card_type == 3:
            return SupportCardType.Team
        else:
            # unknown
            return SupportCardType.Speed
    
    def _get_effect_row_dict(self, effects:list[_SupportCardEffectTable])->dict[SupportCardEffectType,EffectRow]:
        ret = {}
        if effects:
            for effect in effects:
                effect_type = SupportCardEffectType(effect.type)
                effect_row = EffectRow(*list(asdict(effect).values())[2:])
                ret[effect_type] = effect_row
        return ret

    def _get_unique_effect_row(self, unique_effect:_SupportCardUniqueEffect)->UniqueEffectRow:
        if not unique_effect:
            return None
        return UniqueEffectRow(*list(asdict(unique_effect).values())[1:])

    def get_all_support_card_data(self) ->List[SupportCard]:
        text_dict = self._get_all_text_data()
        with self as conn:
            cursor = conn.cursor()
            cursor.execute(f'SELECT * FROM {_TABLE_SUPPORT_CARD_DATA}')
            support_card_data = [_SupportCardData(*row) for row in cursor.fetchall()]
            cursor.execute(f'SELECT * FROM {_TABLE_SUPPORT_CARD_EFFECT_TABLE}')
            support_card_effect_table_data = [_SupportCardEffectTable(*row) for row in cursor.fetchall()]
            cursor.execute(f'SELECT * FROM {_TABLE_SUPPORT_CARD_UNIQUE_EFFECT_TABLE} ')
            support_card_unique_effect_data = [_SupportCardUniqueEffect(*row) for row in cursor.fetchall()]
            cursor.execute(f'SELECT * FROM {_TABLE_SINGLE_MODE_HINT_GAIN} WHERE hint_gain_type = 0')
            hint_gain_data = [_SingleModeHintGain(*row) for row in cursor.fetchall()]
            hint_skill_dict = collections.defaultdict(list)
            for hint_gain in hint_gain_data:
                hint_skill_dict[hint_gain.support_card_id].append(str(hint_gain.hint_value_1))
        effect_dict = collections.defaultdict(list)
        for effect in support_card_effect_table_data:
            effect_dict[effect.id].append(effect)
        unique_effect_dict = {}
        for effect in support_card_unique_effect_data:
            unique_effect_dict[effect.id] = effect
        ret = []
        name_dict = text_dict[_TEXT_SUPPORT_CARD_NAME]
        for card in support_card_data:
            name = name_dict[card.id]
            rarity = SupportCardRarity(card.rarity)
            type = self._get_support_card_type(card)
            effect_row_dict = self._get_effect_row_dict(effect_dict.get(card.id))
            unique_effect = self._get_unique_effect_row(unique_effect_dict.get(card.id))     
            support_card = SupportCard(str(card.id), name, rarity, type,
                                       train_skill_list=hint_skill_dict[card.id],
                                       unique_effect=unique_effect,effect_row_dict=effect_row_dict)
            ret.append(support_card)
        return ret

            
