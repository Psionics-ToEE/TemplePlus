#pragma once
#include "stdafx.h"
#include "common.h"
#include "tig/tig_mes.h"


#define actSeqArraySize 0x20

struct LocationSys;
struct PathQuery;
struct CombatSystem;
struct D20System;
struct ActnSeq;
struct DescriptionSystem;
struct TurnBasedStatus;
struct Pathfinding;
struct CmbtIntrpts;
struct TurnBasedSys;
struct Objects;

struct ActionSequenceSystem : AddressTable
{
	
	D20System * d20;
	CombatSystem * combat;
	Pathfinding * pathfinding;
	LocationSys * location;
	TurnBasedSys * turnbased;
	Objects * object;

	// global variables and structs
	ActnSeq ** actSeqCur;
	ActnSeq * actSeqArray; // size 32
	uint32_t * actnProcState;
	MesHandle  * actionMesHandle; 
	uint32_t * seqSthg_10B3D5C0; // init to 0
	uint32_t * actnProc_10B3D5A0;
	TurnBasedStatus * tbStatus118CD3C0;

	int32_t * seqSthg_118CD3B8; // init to -1
	int32_t * seqSthg_118A0980; // init to 1
	int32_t * seqSthg_118CD570; // init to 0
	uint32_t * numSimultPerformers;
	uint32_t * simulsIdx;  //10B3D5BC
	objHndl * simultPerformerQueue; 

	void curSeqReset(objHndl objHnd);
	void ActionAddToSeq();
		uint32_t addD20AToSeq(D20Actn * d20a, ActnSeq * actSeq);
	uint32_t isPerforming(objHndl objHnd);
	uint32_t addSeqSimple(D20Actn * d20a, ActnSeq * actSeq);
	void IntrrptSthgsub_100939D0(D20Actn * d20a, CmbtIntrpts * str84);
	uint32_t moveSequenceParse(D20Actn * d20aIn, ActnSeq* actSeq, TurnBasedStatus *actnSthg, float distSthg, float reach, int a5);
		void releasePath(PathQueryResult*);
		void addReadiedInterrupts(ActnSeq* actSeq, CmbtIntrpts * intrpts);
		void updateDistTraversed(ActnSeq* actSeq);
	uint32_t actSeqOkToPerform();
	TurnBasedStatus* curSeqGetTurnBasedStatus();
	const char * ActionErrorString(uint32_t actnErrorCode);

	uint32_t AllocSeq(objHndl objHnd);
	uint32_t AssignSeq(objHndl objHnd);
	uint32_t TurnBasedStatusInit(objHndl objHnd);
	void ActSeqCurSetSpellPacket(SpellPacketBody* spellPacketBody, int flag);
	int (__cdecl *sub_1008B9A0)(D20Actn *d20a, float float1, PathQuery *pathQ);
	void sub_1008BB40(ActnSeq*actSeq, D20Actn * d20a); // actSeq@<ebx>
	uint32_t sub_10093950(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t sub_10096450(ActnSeq * actSeq, uint32_t idx, void* iO);
	//10097C20
	
	uint32_t (__cdecl *seqCheckFuncssub_10094CA0)(TurnBasedStatus *actnSthg);
	uint32_t seqCheckFuncs(TurnBasedStatus *actnSthg);
	void AOOSthgSub_10097D50(objHndl, objHndl);
	int32_t AOOSthg2_100981C0(objHndl);
	int32_t InterruptSthg_10099320(D20Actn *d20a);
	int32_t InterruptSthg_10099360(D20Actn *d20a);
	uint32_t combatTriggerSthg(ActnSeq* actSeq);

	unsigned seqCheckAction(D20Actn* d20a, TurnBasedStatus* iO);
	uint32_t curSeqNext();
	void actionPerform();
	void sequencePerform();
	bool projectileCheckBeforeNextAction();
	uint32_t actSeqSpellHarmful(ActnSeq* actSeq);
	uint32_t isSimultPerformer(objHndl);
	uint32_t simulsOk(ActnSeq* actSeq);
	uint32_t simulsAbort(objHndl);
	uint32_t isSomeoneAlreadyActingSimult(objHndl objHnd);


	ActionSequenceSystem();
private:
	bool (__cdecl *_actionPerformProjectile)();
	void (__cdecl *_sub_1008BB40)(D20Actn * d20a); // ActnSeq*actSeq@<ebx>, 
	uint32_t (__cdecl *getRemainingMaxMoveLength)(D20Actn *d20a, TurnBasedStatus *actnSthg, float *floatOut); // doesn't take things like having made 5 foot step into account, just a raw calculation
	uint32_t (__cdecl *_sub_10093950)(D20Actn* d20a);
	void (__cdecl *_sub_100939D0)(CmbtIntrpts* d20a); // D20Actn*@<eax>
	uint32_t (__cdecl* _sub_10096450)(ActnSeq * actSeq, uint32_t); // void * iO @<ebx>
	void(__cdecl* _AOOSthgSub_10097D50)(objHndl, objHndl);
	void(__cdecl *_actionPerformRecursion)();
	int32_t(__cdecl *_AOOSthg2_100981C0)(objHndl);
	uint32_t (__cdecl *_curSeqNext)();
	int32_t(__cdecl * _InterruptSthg_10099360)();
	int32_t(__cdecl * _InterruptSthg_10099320)();
	uint32_t (__cdecl *_actSeqSpellHarmful)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl *_combatTriggerSthg)(); // ActnSeq* @<ebx> 
	uint32_t(__cdecl * _moveSeqD20Sthg)(ActnSeq* actSeq, TurnBasedStatus *actnSthg, float a3, float reach, int a5); //, D20Actn * d20aIn @<eax>
	
};

extern ActionSequenceSystem actSeqSys;



struct TurnBasedStatus
{
	uint32_t hourglassState; // 4 - full action remaining; 2 - single action remaining; 1 - move action remaining
	D20CAF callActionFrameFlags;
	uint32_t idxSthg;
	float surplusMoveDistance; // is nonzero when you have started a move action already and haven't used it all up
	uint32_t field_10;
	uint32_t field_14;
	uint32_t field_18;
	uint32_t field_1C;
	uint32_t errCode;
};

const uint32_t TestSizeOfActnSthg = sizeof(TurnBasedStatus); // should be 36 (0x24)

#pragma pack(push, 1)
struct ActnSeq
{
	D20Actn d20ActArray[32];
	int32_t d20ActArrayNum;
	int32_t d20aCurIdx;
	ActnSeq * prevSeq;
	uint32_t field_B0C;
	uint32_t seqOccupied;
	TurnBasedStatus tbStatus;
	objHndl performer;
	LocAndOffsets performerLoc;
	objHndl targetObj;
	SpellPacketBody spellPktBody;
	D20Actn * d20Action;
	uint32_t aiSpellFlagSthg_maybe;
};
#pragma pack(pop)

struct IntrptSthg
{
	uint32_t field0;
	uint32_t field4;
	objHndl interrupter;
};

struct CmbtIntrpts
{
	IntrptSthg* intrptSthgs[32];
	int32_t numItems;
};

const uint32_t TestSizeOfD20Action= sizeof(D20Actn); // should be 0x58 (88)

const uint32_t TestSizeOfActionSequence = sizeof(ActnSeq); // should be 0x1648 (5704)

uint32_t _addD20AToSeq(D20Actn* d20a, ActnSeq* actSeq);
uint32_t _addSeqSimple(D20Actn* d20a, ActnSeq * actSeq);
unsigned _seqCheckAction(D20Actn* d20a, TurnBasedStatus* iO);
uint32_t _isPerforming(objHndl objHnd);
uint32_t _actSeqOkToPerform();
void _actionPerform();
uint32_t _isSimultPerformer(objHndl objHnd);
uint32_t _seqCheckFuncsCdecl(TurnBasedStatus *actnSthg);
uint32_t _moveSequenceParseUsercallWrapper(ActnSeq *actSeq, TurnBasedStatus *actnSthg, float distSthg, float reach, int flagSthg); //, D20_Action *d20aIn@<eax>
uint32_t _unspecifiedMoveAddToSeq(D20Actn *d20a, ActnSeq *actSeq, TurnBasedStatus *actnSthg);
void _sequencePerform();
void _curSeqReset(objHndl objHnd);
uint32_t _allocSeq(objHndl objHnd);
uint32_t _assignSeq(objHndl objHnd);
TurnBasedStatus * _curSeqGetTurnBasedStatus();
uint32_t _turnBasedStatusInit(objHndl objHnd);
const char * _ActionErrorString(uint32_t actnErrorCode);
void __cdecl _ActSeqCurSetSpellPacket(SpellPacketBody *, int flag );