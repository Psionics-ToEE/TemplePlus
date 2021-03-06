
#include "stdafx.h"
#include "dice.h"
#include <temple/dll.h>

static struct DiceAddresses : temple::AddressTable {

	int (__cdecl *DiceRoller)(int nNum_Die, int nDie_Type, int nBonus);
	
	int (__cdecl *RandomIntRange)(int from, int to);

	DiceAddresses() {
		rebase(DiceRoller, 0x10038B60);
		rebase(RandomIntRange, 0x10038DF0);
	}
} addresses;

int Dice::Roll(int count, int sides, int modifier) {
	return addresses.DiceRoller(count, sides, modifier);
}

int Dice::Roll() {
	return Roll(mCount, mSides, mModifier);
}

bool Dice::Parse(const char* diceStr, int& count, int& sides, int& modifier) {
	modifier = 0;
	count = atoi(diceStr);
	auto tmp = strchr(diceStr, 'd');
	if (tmp) {
		sides = atoi(tmp + 1);
		tmp = strchr(diceStr, '-');
		if (!tmp) {
			tmp = strchr(diceStr, '+');
		}
		if (tmp) {
			modifier = atoi(tmp);
		}
		return true;
	}
	return false;
}

int RandomIntRange(int from, int to) {
	return addresses.RandomIntRange(from, to);
}
