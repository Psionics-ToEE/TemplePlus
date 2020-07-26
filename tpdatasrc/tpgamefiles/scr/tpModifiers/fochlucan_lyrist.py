from templeplus.pymod import PythonModifier
from toee import *
import tpdp
import char_class_utils

###################################################

def GetConditionName():
	return "Fochlucan Lyrist"

def GetSpellCasterConditionName():
	return "Fochlucan Lyrist Spellcasting"
	
print "Registering " + GetConditionName()

classEnum = stat_level_fochlucan_lyrist
classSpecModule = __import__('class082_fochlucan_lyrist')

###################################################


#### standard callbacks - BAB and Save values
def OnGetToHitBonusBase(attachee, args, evt_obj):
	classLvl = attachee.stat_level_get(classEnum)
	babvalue = game.get_bab_for_class(classEnum, classLvl)
	evt_obj.bonus_list.add(babvalue, 0, 137) # untyped, description: "Class"
	return 0

def OnGetSaveThrowFort(attachee, args, evt_obj):
	value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Fortitude)
	evt_obj.bonus_list.add(value, 0, 137)
	return 0

def OnGetSaveThrowReflex(attachee, args, evt_obj):
	value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Reflex)
	evt_obj.bonus_list.add(value, 0, 137)
	return 0

def OnGetSaveThrowWill(attachee, args, evt_obj):
	value = char_class_utils.SavingThrowLevel(classEnum, attachee, D20_Save_Will)
	evt_obj.bonus_list.add(value, 0, 137)
	return 0


classSpecObj = PythonModifier(GetConditionName(), 0)
classSpecObj.AddHook(ET_OnToHitBonusBase, EK_NONE, OnGetToHitBonusBase, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_FORTITUDE, OnGetSaveThrowFort, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_REFLEX, OnGetSaveThrowReflex, ())
classSpecObj.AddHook(ET_OnSaveThrowLevel, EK_SAVE_WILL, OnGetSaveThrowWill, ())

##### Spell casting

# Fochlucan Lyrist raises the caster level for its two base classes specified in Modifier args 0 & 1

# configure the spell casting condition to hold the highest two Arcane/Divine classes as chosen-to-be-extended classes
def OnAddSpellCasting(attachee, args, evt_obj):
	#arg0 holds the arcane class
	if args.get_arg(0) == 0:
		args.set_arg(0, char_class_utils.GetHighestArcaneClass(attachee))
	
	#arg1 holds the divine class
	if args.get_arg(1) == 0:
		args.set_arg(1, char_class_utils.GetHighestDivineClass(attachee))
	return 0

def OnGetBaseCasterLevel(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	class_code = evt_obj.arg0
	if class_code != class_extended_1 and class_code != class_extended_2:
		if evt_obj.arg1 == 0: # are you specifically looking for the Fochlucan Lyrist caster level?
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnSpellListExtensionGet(attachee, args, evt_obj):
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	class_code = evt_obj.arg0
	if (class_code != class_extended_1 and class_code != class_extended_2):
		if (evt_obj.arg1 == 0): # are you specifically looking for the Fochlucan Lyrist caster level?
			return 0
	classLvl = attachee.stat_level_get(classEnum)
	evt_obj.bonus_list.add(classLvl, 0, 137)
	return 0

def OnInitLevelupSpellSelection(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	classSpecModule.InitSpellSelection(attachee, class_extended_1, class_extended_2)
	return 0

def OnLevelupSpellsCheckComplete(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	if not classSpecModule.LevelupCheckSpells(attachee, class_extended_1, class_extended_2):
		evt_obj.bonus_list.add(-1, 0, 137) # denotes incomplete spell selection
	return 1

def OnLevelupSpellsFinalize(attachee, args, evt_obj):
	if evt_obj.arg0 != classEnum:
		return 0
	class_extended_1 = args.get_arg(0)
	class_extended_2 = args.get_arg(1)
	classSpecModule.LevelupSpellsFinalize(attachee, class_extended_1, class_extended_2)
	return

spellCasterSpecObj = PythonModifier(GetSpellCasterConditionName(), 8)
spellCasterSpecObj.AddHook(ET_OnConditionAdd, EK_NONE, OnAddSpellCasting, ())
spellCasterSpecObj.AddHook(ET_OnGetBaseCasterLevel, EK_NONE, OnGetBaseCasterLevel, ())
spellCasterSpecObj.AddHook(ET_OnSpellListExtensionGet, EK_NONE, OnSpellListExtensionGet, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Activate, OnInitLevelupSpellSelection, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Check_Complete, OnLevelupSpellsCheckComplete, ())
spellCasterSpecObj.AddHook(ET_OnLevelupSystemEvent, EK_LVL_Spells_Finalize, OnLevelupSpellsFinalize, ())

#Unbound Ability

# XP Penalty
def FochlucanLyristXPPenalty(attachee, args, evt_obj):
	evt_obj.return_val = 1
	return 0
	
def FochlucanLyristIgnoreDruidOath(attachee, args, evt_obj):
	if evt_obj.item == OBJ_HANDLE_NULL:
		return 0
		
	wearFlags = evt_obj.item.get_item_wear_flags()
	
	if (1 << item_wear_armor) & wearFlags:
		armor_flags =  evt_obj.item.obj_get_int(obj_f_armor_flags)
		if (armor_flags & ARMOR_TYPE_NONE) or (armor_flags == ARMOR_TYPE_LIGHT):
			evt_obj.ignore_druid_oath = 1
		return 0

fochlucanLyristUnbound = PythonModifier("Unbound", 2) #Spare, Spare
fochlucanLyristUnbound.MapToFeat("Unbound")
fochlucanLyristUnbound.AddHook(ET_OnD20PythonQuery, "No MultiClass XP Penalty", FochlucanLyristXPPenalty, () )
fochlucanLyristUnbound.AddHook(ET_OnIgnoreDruidOathCheck, EK_NONE, FochlucanLyristIgnoreDruidOath, ())

def FochlucanLyristBardicMusicBonusLevels(attachee, args, evt_obj):
	evt_obj.return_val = evt_obj.return_val + attachee.stat_level_get(classEnum)
	return 0

fochlucanLyristBardicMusic = PythonModifier("Lyrist Bardic Music", 2) #Spare, Spare
fochlucanLyristBardicMusic.MapToFeat("Lyrist Bardic Music")
fochlucanLyristBardicMusic.AddHook(ET_OnD20PythonQuery, "Bardic Music Bonus Levels", FochlucanLyristBardicMusicBonusLevels, () )

