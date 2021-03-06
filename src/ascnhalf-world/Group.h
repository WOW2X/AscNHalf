/*
  * AscNHalf MMORPG Server
  * Copyright (C) 2005-2007 Ascent Team <http://www.ascentemu.com/>
  * Copyright (C) 2010 AscNHalf Team <http://ascnhalf.scymex.info/>
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU Affero General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU Affero General Public License for more details.
  *
  * You should have received a copy of the GNU Affero General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  */

#ifndef _GROUP_H_
#define _GROUP_H_

enum PartyErrors
{
	ERR_PARTY_NO_ERROR					= 0,
	ERR_PARTY_CANNOT_FIND				= 1,
	ERR_PARTY_IS_NOT_IN_YOUR_PARTY		= 2,
	ERR_PARTY_UNK						= 3,
	ERR_PARTY_IS_FULL					= 4,
	ERR_PARTY_ALREADY_IN_GROUP			= 5,
	ERR_PARTY_YOU_ARENT_IN_A_PARTY		= 6,
	ERR_PARTY_YOU_ARE_NOT_LEADER		= 7,
	ERR_PARTY_WRONG_FACTION				= 8,
	ERR_PARTY_IS_IGNORING_YOU			= 9,
};

enum GroupTypes
{
	GROUP_TYPE_PARTY	= 0,
	GROUP_TYPE_BG		= 1,
	GROUP_TYPE_RAID		= 2,
	GROUP_TYPE_BGRAID	= GROUP_TYPE_BG | GROUP_TYPE_RAID,
	GROUP_TYPE_LFD		= 8,
	// 10 Leaving battleground, going to normal group
};

enum MaxGroupCount
{
	MAX_GROUP_SIZE_PARTY				= 5,
	MAX_GROUP_SIZE_RAID					= 40,
};

enum QuickGroupUpdateFlags
{
	PARTY_UPDATE_FLAG_POSITION			= 1,
	PARTY_UPDATE_FLAG_ZONEID			= 2,
//	GROUP_UPDATE_FLAG_VEHICLE_SEAT
};

enum GroupFlags
{
	GROUP_FLAG_DONT_DISBAND_WITH_NO_MEMBERS			= 1,
	GROUP_FLAG_REMOVE_OFFLINE_PLAYERS				= 2,
	GROUP_FLAG_BATTLEGROUND_GROUP					= 4,
};

struct PlayerInfo;
typedef struct
{
	PlayerInfo * player_info;
	Player* player;
}GroupMember;

class Group;
class Player;

typedef std::set<PlayerInfo*> GroupMembersSet;

class SERVER_DECL SubGroup	  // Most stuff will be done through here, not through the "Group" class.
{
public:
	friend class Group;

	SubGroup(Group* parent, uint32 id):m_Parent(parent),m_Id(id)
	{
	}

	~SubGroup();

	INLINE GroupMembersSet::iterator GetGroupMembersBegin(void) { return m_GroupMembers.begin(); }
	INLINE GroupMembersSet::iterator GetGroupMembersEnd(void)   { return m_GroupMembers.end();   }

	bool AddPlayer(PlayerInfo * info);
	void RemovePlayer(PlayerInfo * info);
	
	INLINE bool IsFull(void)				{ return m_GroupMembers.size() >= MAX_GROUP_SIZE_PARTY; }
	INLINE size_t GetMemberCount(void)		{ return m_GroupMembers.size(); }
	
	INLINE uint32 GetID(void)			   { return m_Id; }
	INLINE void SetID(uint32 newid)		 { m_Id = newid; }

	INLINE void   SetParent(Group* parent)  { m_Parent = parent; }
	INLINE Group* GetParent(void)		   { return m_Parent; }

	void   Disband();
	bool HasMember(uint32 guid);

protected:

	GroupMembersSet	 m_GroupMembers;
	Group*			  m_Parent;
	uint32			  m_Id;

};

class Arena;
class SERVER_DECL Group
{
public:
	friend class SubGroup;

	static Group* Create();

	Group(bool Assign);
	~Group();

	// Adding/Removal Management
	bool AddMember(PlayerInfo * info, int32 subgroupid=-1);
	void RemovePlayer(PlayerInfo * info);

	// Leaders and Looting
	void SetLeader(Player* pPlayer,bool silent);
	void SetLooter(Player* pPlayer, uint8 method, uint16 threshold);

	// Transferring data to clients
	void Update();

	INLINE void SendPacketToAll(WorldPacket *packet) { SendPacketToAllButOne(packet, NULLPLR); }
	void SendPacketToAllButOne(WorldPacket *packet, Player* pSkipTarget);

	INLINE void SendPacketToAll(StackPacket *packet) { SendPacketToAllButOne(packet, NULLPLR); }
	void SendPacketToAllButOne(StackPacket *packet, Player* pSkipTarget);

	INLINE void OutPacketToAll(uint16 op, uint16 len, const void* data) { OutPacketToAllButOne(op, len, data, NULLPLR); }
	void OutPacketToAllButOne(uint16 op, uint16 len, const void* data, Player* pSkipTarget);
	
	void SendPacketToAllInRange(Player* plr, WorldPacket *packet);

	void SendNullUpdate(Player* pPlayer);

	// Group Combat
	void SendPartyKillLog(Object* player, Object* Unit);

	// Destroying/Converting
	void Disband();
	Player* FindFirstPlayer();
	bool HasDisenchanters();
	
	// Accessing functions
	INLINE SubGroup* GetSubGroup(uint32 Id)
	{
		if(Id >= 8)
			return 0;

		return m_SubGroups[Id];
	}

	INLINE uint32 GetSubGroupCount(void) { return m_SubGroupCount; }

	INLINE uint8 GetMethod(void) { return m_LootMethod; }
	INLINE uint16 GetThreshold(void) { return m_LootThreshold; }
	INLINE PlayerInfo* GetLeader(void) { return m_Leader; }
	INLINE PlayerInfo* GetLooter(void) { return m_Looter; }

	void MovePlayer(PlayerInfo* info, uint8 subgroup);

	bool HasMember(Player* pPlayer);
	bool HasMember(PlayerInfo * info);
	INLINE uint32 MemberCount(void) { return m_MemberCount; }
	INLINE bool IsFull() { return ((m_GroupType == GROUP_TYPE_PARTY && m_MemberCount >= MAX_GROUP_SIZE_PARTY) || (m_GroupType == GROUP_TYPE_RAID && m_MemberCount >= MAX_GROUP_SIZE_RAID)); }

	SubGroup* FindFreeSubGroup();

	void ExpandToRaid();

	void SaveToDB();
	void LoadFromDB(Field *fields);

	INLINE uint8 GetGroupType() { return m_GroupType; }
	INLINE uint32 GetID() { return m_Id; }

	void UpdateOutOfRangePlayer(Player* pPlayer, uint32 Flags, bool Distribute, WorldPacket * Packet);
	void UpdateAllOutOfRangePlayersFor(Player* pPlayer);
	void HandleUpdateFieldChange(uint32 Index, Player* pPlayer);
	void HandlePartialChange(uint32 Type, Player* pPlayer);

	uint64 m_targetIcons[8];
	INLINE Mutex& getLock() { return m_groupLock; }
	INLINE void Lock() { m_groupLock.Acquire(); }
	INLINE void Unlock() { return m_groupLock.Release(); }
	bool m_isqueued;

	void SetAssistantLeader(PlayerInfo * pMember);
	void SetMainTank(PlayerInfo * pMember);
	void SetMainAssist(PlayerInfo * pMember);

	INLINE PlayerInfo * GetAssistantLeader() { return m_assistantLeader; }
	INLINE PlayerInfo * GetMainTank() { return m_mainTank; }
	INLINE PlayerInfo * GetMainAssist() { return m_mainAssist; }

	/************************************************************************/
	/* Voicechat                                                            */
	/************************************************************************/
#ifdef VOICE_CHAT
	void AddVoiceMember(PlayerInfo * pPlayer);
	void RemoveVoiceMember(PlayerInfo * pPlayer);
	void SendVoiceUpdate();
	void CreateVoiceSession();
	void VoiceChannelCreated(uint16 id);
	void VoiceSessionDropped();
	void VoiceSessionReconnected();

public:
	bool m_voiceChannelRequested;
	int16 m_voiceChannelId;
	uint32 m_voiceMemberCount;
protected:
	PlayerInfo* m_voiceMembersList[41];
#endif	// VOICE_CHAT

public:
	INLINE void SetFlag(uint8 groupflag) { m_groupFlags |= groupflag; }
	INLINE void RemoveFlag(uint8 groupflag) { m_groupFlags &= ~groupflag; }
	INLINE bool HasFlag(uint8 groupflag) { return (m_groupFlags & groupflag) > 0 ? true : false; }
	INLINE int8 GetDifficulty() { return m_difficulty; }
	INLINE int8 GetRaidDifficulty() { return m_raiddifficulty; }
	INLINE void SetDifficulty(uint8 diff) { m_difficulty = diff; }
	INLINE void SetRaidDifficulty(uint8 diff) { m_raiddifficulty = diff; }

protected:
	PlayerInfo * m_Leader;
	PlayerInfo * m_Looter;
	PlayerInfo * m_assistantLeader;
	PlayerInfo * m_mainTank;
	PlayerInfo * m_mainAssist;

	uint8 m_LootMethod;
	uint16 m_LootThreshold;
	uint8 m_GroupType;
	uint32 m_Id;

	SubGroup* m_SubGroups[8];
	uint8 m_SubGroupCount;
	uint32 m_MemberCount;
	Mutex m_groupLock;
	bool m_dirty;
	bool m_updateblock;
	uint8 m_difficulty;
	uint8 m_raiddifficulty;
	uint8 m_groupFlags;

	// Evil prayer of mending stuff
public:
	uint8 m_prayerOfMendingCount;
	uint32 m_prayerOfMendingTarget;
};

#endif  // _GROUP_H_
