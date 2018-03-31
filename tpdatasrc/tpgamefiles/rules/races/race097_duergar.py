from toee import *
import race_defs

###################################################

raceEnum = race_dwarf + (3 << 5)

raceSpec = race_defs.RaceSpec()
raceSpec.hit_dice        = dice_new("0d0")
raceSpec.level_modifier  = 1
raceSpec.help_topic      = "TAG_DUERGAR"
raceSpec.flags           = 0
raceSpec.modifier_name   = "Duergar"
raceSpec.height_male     = [45, 53]
raceSpec.height_female   = [43, 51]
raceSpec.weight_male     = [148, 178]
raceSpec.weight_female   = [104, 134]
raceSpec.stat_modifiers  = [0, 0, 2, 0, 0, -4]   # str, dex, con, int, wis, cha
raceSpec.proto_id        = 13002
raceSpec.material_offset = 6         # offset into rules/material_ext.mes file

###################################################
def RegisterRace():
	print "Registering race: Duergar"
	raceSpec.register(raceEnum)

def GetFavoredClass(obj = OBJ_HANDLE_NULL):
	return stat_level_fighter

def GetLevelModifier(obj = OBJ_HANDLE_NULL):
	return 1