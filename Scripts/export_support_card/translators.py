from abc import ABC, abstractmethod

from umamodels import *
from net_utils import get_json_from_github_file

class Translator(ABC):
    @abstractmethod
    def translate_skill(self, skill:Skill) -> Skill:
        pass

    @abstractmethod
    def translate_chara_card(self, chara_card:CharacterCard) -> CharacterCard:
        pass

    @abstractmethod
    def translate_support_card(self, support_card:SupportCard) -> SupportCard:
        pass

class UraraWinTranslator(Translator):

    def __init__(self, properties, locale="zh_CN") -> None:
        super().__init__()
        self.translate_mapping = None
        self.locale = locale
        self.p = properties
    
    def _ensure_translate_mapping(self):
        if self.translate_mapping:
            return
        target_file = f"output/urara_translate_{self.locale}.json"
        self.translate_mapping = get_json_from_github_file(
            self.p, target_file, 'wrrwrr111', 'pretty-derby', 'master', f'src/assert/locales/{self.locale}.json')
    
    def _translate(self, text):
        if not text:
            return None
        self._ensure_translate_mapping()
        return self.translate_mapping.get(text, text)

    def _has_translate(self, text):
        self._ensure_translate_mapping()
        return text in self.translate_mapping
    
    def _extract_name(self, name: str):
        if name.startswith('['):
            nick_name = name[1:name.index(']')]
            real_name = name[name.index(']') + 1:]
            return nick_name, real_name
        return None, name
    
    def translate_skill(self, skill:Skill) -> Skill:
        skill.name = self._translate(skill.name)
        skill.description = self._translate(skill.description)
        skill.is_translated = self._has_translate(skill.name)
        return skill
    
    def translate_chara_card(self, chara_card:CharacterCard) -> CharacterCard:
        nick_name, real_name = self._extract_name(chara_card.name)
        nick_name = self._translate(nick_name)
        real_name = self._translate(real_name)
        if nick_name:
            chara_card.name = f"[{nick_name}]{real_name}"
        else:
            chara_card.name = real_name
        chara_card.is_translated = self._has_translate(chara_card.name)
        return chara_card
    
    def translate_support_card(self, support_card:SupportCard) -> SupportCard:
        nick_name, real_name = self._extract_name(support_card.name)
        nick_name = self._translate(nick_name)
        real_name = self._translate(real_name)
        if nick_name:
            support_card.name = f"[{nick_name}]{real_name}"
        else:
            support_card.name = real_name
        support_card.is_translated = self._has_translate(support_card.name)
        return support_card
    
class TrainersLegendTranslator(Translator):

    def __init__(self, properties, locale="zh_CN") -> None:
        super().__init__()
        self.locale = locale
        self.translate_mapping = None
        self.p = properties
    
    def _ensure_translate_mapping(self):
        if self.translate_mapping:
            return
        target_file = f"output/trainer_legend_translate_{self.locale}.json"
        self.translate_mapping = get_json_from_github_file(
            self.p, target_file, 'MinamiChiwa', 'Trainers-Legend-G-TRANS', 'master', f'localized_data/text_data.json')
    
    def _translate(self, text, category, index):
        self._ensure_translate_mapping()
        if category in self.translate_mapping:
            if index in self.translate_mapping[category]:
                return self.translate_mapping[category][index]
            else:
                return text
        else:
            return text
    
    def _has_translate(self, category, index):
        self._ensure_translate_mapping()
        if category in self.translate_mapping:
            if index in self.translate_mapping[category]:
                return True
            else:
                return False
        else:
            return False
    
    def translate_skill(self, skill:Skill) -> Skill:
        skill.name = self._translate(skill.name, "47", skill.id)
        skill.description = self._translate(skill.description, "48", skill.id)
        skill.is_translated = self._has_translate("47", skill.id)
        return skill
    
    def translate_chara_card(self, chara_card:CharacterCard) -> CharacterCard:
        chara_card.name = self._translate(chara_card.name, "4", chara_card.id)
        chara_card.is_translated = self._has_translate("4", chara_card.id)
        return chara_card
    
    def translate_support_card(self, support_card:SupportCard) -> SupportCard:
        support_card.name = self._translate(support_card.name, "75", support_card.id)
        support_card.is_translated = self._has_translate("75", support_card.id)
        return support_card
