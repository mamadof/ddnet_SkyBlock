/* (c) Shereef Marzouk. See "licence DDRace.txt" and the readme.txt in the root of the distribution for more information. */
/* Based on Race mod stuff and tweaked by GreYFoX@GTi and others to fit our DDRace needs. */
#include "DDRace.h"

#include <engine/server.h>
#include <engine/shared/config.h>
#include <game/mapitems.h>
#include <game/server/entities/character.h>
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <game/server/score.h>
#include <game/version.h>
#include <game/server/entities/projectile.h>

//my stuff
#include <skyblock/values.h>

#define GAME_TYPE_NAME "SkyBl"
#define TEST_TYPE_NAME "SkyBl_t"

CGameControllerDDRace::CGameControllerDDRace(class CGameContext *pGameServer) :
	IGameController(pGameServer)
{
	m_pGameType = g_Config.m_SvTestingCommands ? TEST_TYPE_NAME : GAME_TYPE_NAME;
}

CGameControllerDDRace::~CGameControllerDDRace() = default;

CScore *CGameControllerDDRace::Score()
{
	return GameServer()->Score();
}

void CGameControllerDDRace::HandleCharacterTiles(CCharacter *pChr, int MapIndex)
{
	CPlayer *pPlayer = pChr->GetPlayer();
	const int ClientID = pPlayer->GetCID();
	//My switch Tiles
	int my_SwitchType = pChr->Collision()->GetSwitchType(MapIndex);
	int my_SwitchNumber = pChr->Collision()->GetSwitchNumber(MapIndex);
	int my_SwitchDelay = pChr->Collision()->GetSwitchDelay(MapIndex);

	int m_TileIndex = GameServer()->Collision()->GetTileIndex(MapIndex);
	int m_TileFIndex = GameServer()->Collision()->GetFTileIndex(MapIndex);

	//Sensitivity
	int S1 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + pChr->GetProximityRadius() / 3.f, pChr->GetPos().y - pChr->GetProximityRadius() / 3.f));
	int S2 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x + pChr->GetProximityRadius() / 3.f, pChr->GetPos().y + pChr->GetProximityRadius() / 3.f));
	int S3 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - pChr->GetProximityRadius() / 3.f, pChr->GetPos().y - pChr->GetProximityRadius() / 3.f));
	int S4 = GameServer()->Collision()->GetPureMapIndex(vec2(pChr->GetPos().x - pChr->GetProximityRadius() / 3.f, pChr->GetPos().y + pChr->GetProximityRadius() / 3.f));
	int Tile1 = GameServer()->Collision()->GetTileIndex(S1);
	int Tile2 = GameServer()->Collision()->GetTileIndex(S2);
	int Tile3 = GameServer()->Collision()->GetTileIndex(S3);
	int Tile4 = GameServer()->Collision()->GetTileIndex(S4);
	int FTile1 = GameServer()->Collision()->GetFTileIndex(S1);
	int FTile2 = GameServer()->Collision()->GetFTileIndex(S2);
	int FTile3 = GameServer()->Collision()->GetFTileIndex(S3);
	int FTile4 = GameServer()->Collision()->GetFTileIndex(S4);

	const int PlayerDDRaceState = pChr->m_DDRaceState;
	bool IsOnStartTile = (m_TileIndex == TILE_START) || (m_TileFIndex == TILE_START) || FTile1 == TILE_START || FTile2 == TILE_START || FTile3 == TILE_START || FTile4 == TILE_START || Tile1 == TILE_START || Tile2 == TILE_START || Tile3 == TILE_START || Tile4 == TILE_START;
	// start
	if(IsOnStartTile && PlayerDDRaceState != DDRACE_CHEAT)
	{
		const int Team = GameServer()->GetDDRaceTeam(ClientID);
		if(Teams().GetSaving(Team))
		{
			GameServer()->SendStartWarning(ClientID, "You can't start while loading/saving of team is in progress");
			pChr->Die(ClientID, WEAPON_WORLD);
			return;
		}
		if(g_Config.m_SvTeam == SV_TEAM_MANDATORY && (Team == TEAM_FLOCK || Teams().Count(Team) <= 1))
		{
			GameServer()->SendStartWarning(ClientID, "You have to be in a team with other tees to start");
			pChr->Die(ClientID, WEAPON_WORLD);
			return;
		}
		if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO && Team > TEAM_FLOCK && Team < TEAM_SUPER && Teams().Count(Team) < g_Config.m_SvMinTeamSize)
		{
			char aBuf[128];
			str_format(aBuf, sizeof(aBuf), "Your team has fewer than %d players, so your team rank won't count", g_Config.m_SvMinTeamSize);
			GameServer()->SendStartWarning(ClientID, aBuf);
		}
		if(g_Config.m_SvResetPickups)
		{
			pChr->ResetPickups();
		}

		Teams().OnCharacterStart(ClientID);
		pChr->m_LastTimeCp = -1;
		pChr->m_LastTimeCpBroadcasted = -1;
		for(float &CurrentTimeCp : pChr->m_aCurrentTimeCp)
		{
			CurrentTimeCp = 0.0f;
		}
	}

	// finish
	if(((m_TileIndex == TILE_FINISH) || (m_TileFIndex == TILE_FINISH) || FTile1 == TILE_FINISH || FTile2 == TILE_FINISH || FTile3 == TILE_FINISH || FTile4 == TILE_FINISH || Tile1 == TILE_FINISH || Tile2 == TILE_FINISH || Tile3 == TILE_FINISH || Tile4 == TILE_FINISH) && PlayerDDRaceState == DDRACE_STARTED)
		Teams().OnCharacterFinish(ClientID);

	// unlock team
	else if(((m_TileIndex == TILE_UNLOCK_TEAM) || (m_TileFIndex == TILE_UNLOCK_TEAM)) && Teams().TeamLocked(GameServer()->GetDDRaceTeam(ClientID)))
	{
		Teams().SetTeamLock(GameServer()->GetDDRaceTeam(ClientID), false);
		GameServer()->SendChatTeam(GameServer()->GetDDRaceTeam(ClientID), "Your team was unlocked by an unlock team tile");
	}

	// solo part
	if(((m_TileIndex == TILE_SOLO_ENABLE) || (m_TileFIndex == TILE_SOLO_ENABLE)) && !Teams().m_Core.GetSolo(ClientID))
	{
		GameServer()->SendChatTarget(ClientID, "You are now in a solo part");
		pChr->SetSolo(true);
	}
	else if(((m_TileIndex == TILE_SOLO_DISABLE) || (m_TileFIndex == TILE_SOLO_DISABLE)) && Teams().m_Core.GetSolo(ClientID))
	{
		GameServer()->SendChatTarget(ClientID, "You are now out of the solo part");
		pChr->SetSolo(false);
	}


	// char abuff[100];
	// str_format(abuff, sizeof(abuff), "SavePos x:  y:: %d", pChr->m_SetSavePos);
	// GameServer()->SendBroadcast(abuff, ClientID);


	//Score Farming
	if ((my_SwitchType == 22) && (my_SwitchNumber == 20) && my_SwitchDelay)
	{
		if(pChr->m_TickInFarm >= 100000)
		{
			pChr->m_TickInFarm = 0;
		}
		pChr->m_TickInFarm++;
		if(pChr->m_TickInFarm % my_SwitchDelay == 0)
			{
				pPlayer->my_score += 1;
				GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_PICKUP_HEALTH);
				
			}
	}	

	//my Switch system number begins from 20
	if ((my_SwitchType == 22) && (my_SwitchNumber >= 21))
	{
		//Upgrade System
		switch (my_SwitchNumber)
		{
			case 21: //Jetpack Upgrades
			if(!pChr->m_PriceShown  && pChr->m_Jetpack_Ups != mc_Jetpack_Up_Max)
			{
				pChr->PrintThePrice(mc_Jetpack_Up_Price);
			}
			else if(pChr->m_Jetpack_Ups == mc_Jetpack_Up_Max && !pChr->m_MaximumShown)
			{
				GameServer()->SendBroadcast("Maximum jetpack upgrades !", ClientID);
				pChr->m_MaximumShown = true;
			}
			if(pPlayer->my_score >= mc_Jetpack_Up_Price && pChr->m_Jetpack_Ups < mc_Jetpack_Up_Max && (pChr->Core()->m_ActiveWeapon == WEAPON_HAMMER) && pChr->m_Fire && !pChr->m_Buyed)
			{
				pChr->m_Jetpack_Ups++;
				if(pChr->m_Jetpack_Ups == 1)
				{
					pChr->Core()->m_Jetpack = true;
				}
				else
				{
					pChr->m_CharJetpackStrenght += 21;
					pPlayer->my_score -= mc_Jetpack_Up_Price;
				}

				pChr->m_Buyed = true;
				GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_PICKUP_HEALTH);
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
				GameServer()->CreateDeath(pChr->Core()->m_Pos, ClientID);
				// GameServer()->CreateDamageInd(pChr->Core()->m_Pos, 3.44, pChr->m_Jetpack_Ups);
				// pPlayer->BroadCastUpgrades();
			}
			break;

			case 22: //Jump Upgrades
			if(!pChr->m_PriceShown  && pChr->m_Jump_Ups != mc_Jump_Up_Max){
				pChr->PrintThePrice(mc_Jump_Up_Price);
			}
			else if(pChr->m_Jump_Ups == mc_Jump_Up_Max && !pChr->m_MaximumShown)
			{
				GameServer()->SendBroadcast("Maximum jump upgrades !", ClientID);
				pChr->m_MaximumShown = true;
			}
			if((pPlayer->my_score >= mc_Jump_Up_Price) && (pChr->m_Jump_Ups < mc_Jump_Up_Max && (pChr->Core()->m_ActiveWeapon == WEAPON_HAMMER) && pChr->m_Fire && !pChr->m_Buyed))
			{	
				pChr->m_Jump_Ups++;
				pChr->Core()->m_Jumps++;
				pPlayer->my_score -= mc_Jump_Up_Price;
				pChr->m_Buyed = true;

				GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_PICKUP_HEALTH);
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
				GameServer()->CreateDeath(pChr->Core()->m_Pos, ClientID);
				// GameServer()->CreateDamageInd(pChr->Core()->m_Pos, 3.44, pChr->m_Jump_Ups);
				// pPlayer->BroadCastUpgrades();


			}
			break;

			case 23: //Hook upgrade
			if(!pChr->m_PriceShown && pChr->m_Hook_Ups != mc_Hook_Up_Max){
				pChr->PrintThePrice(mc_Hook_Up_Price);
				pChr->m_PriceShown = true;

			}else if(pChr->m_Hook_Ups == mc_Hook_Up_Max && !pChr->m_MaximumShown){
				GameServer()->SendBroadcast("Maximum hook upgrades !", ClientID);
				pChr->m_MaximumShown = true;
			}
			if(pPlayer->my_score >= mc_Hook_Up_Price && pChr->m_Hook_Ups < mc_Hook_Up_Max && (pChr->Core()->m_ActiveWeapon == WEAPON_HAMMER) && pChr->m_Fire && !pChr->m_Buyed){

				pChr->m_Hook_Ups++;
				pPlayer->my_score -= mc_Hook_Up_Price;
				pChr->m_Buyed = true;

				GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_PICKUP_HEALTH);
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
				GameServer()->CreateDeath(pChr->Core()->m_Pos, ClientID);
				// GameServer()->CreateDamageInd(pChr->Core()->m_Pos, 3.44, pChr->m_Hook_Ups);
				// pPlayer->BroadCastUpgrades();
			}
			break;

			case 24://ExtraLives
			if(!pChr->m_PriceShown && pChr->m_ExtraLives != MAX_EXTRA_LIVES){
				pChr->PrintThePrice(PRICE_EXTRA_LIVES);
				pChr->m_PriceShown = true;

			}else if(pChr->m_ExtraLives >= MAX_EXTRA_LIVES && !pChr->m_MaximumShown){
				GameServer()->SendBroadcast("Maximum Extra Lives !", ClientID);
				pChr->m_MaximumShown = true;
			}
			if(pPlayer->my_score >= PRICE_EXTRA_LIVES && pChr->m_ExtraLives < MAX_EXTRA_LIVES && (pChr->Core()->m_ActiveWeapon == WEAPON_HAMMER) && pChr->m_Fire && !pChr->m_Buyed){

				pChr->m_ExtraLives++;
				pPlayer->my_score -= PRICE_EXTRA_LIVES;
				pChr->m_Buyed = true;

				GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_PICKUP_HEALTH);
				pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
				GameServer()->CreateDeath(pChr->Core()->m_Pos, ClientID);
				// GameServer()->CreateDamageInd(pChr->Core()->m_Pos, 3.44, pChr->m_Hook_Ups);
				// pPlayer->BroadCastUpgrades();
			}
			break;

			case 25://bank money deposit
			if(pPlayer->m_IsLoged)
			{
				char abuff[200];
				if((pChr->Core()->m_ActiveWeapon == WEAPON_HAMMER) && pChr->m_Fire && !pChr->m_Buyed && pPlayer->my_score != 0)
				{
					if(pPlayer->my_score >= NSkyb::MONEY_DIPASIT_AMOUNT)
					{
						pPlayer->ChangeMoney(NSkyb::MONEY_DIPASIT_AMOUNT);
						pPlayer->my_score -= NSkyb::MONEY_DIPASIT_AMOUNT;
					}
					else if(pPlayer->my_score < NSkyb::MONEY_DIPASIT_AMOUNT)
					{
						pPlayer->ChangeMoney(pPlayer->my_score);
						pPlayer->my_score = 0;
					}
					GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_HIT);
					pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
					GameServer()->CreateDeath(pChr->Core()->m_Pos, ClientID);
					
					pChr->m_Buyed = true;
					pChr->m_IsMoneyChanged = true;
				}
				if(!pChr->m_PriceShown || pChr->m_IsMoneyChanged)// shows the money anyways
				{
					pChr->m_IsMoneyChanged = false;
					str_format(abuff, sizeof(abuff), "Saved Money: %llu", pPlayer->ReadMoney());
					GameServer()->SendBroadcast(abuff, pPlayer->GetCID());
					pChr->m_PriceShown = true;
				}
			}
			else if(!pChr->m_MaximumShown)
			{
				GameServer()->SendBroadcast("You need a bank account first. try /register or /login", ClientID);
				pChr->m_MaximumShown = true;
			}
			break;

			case 26://bank money withdraw
			if(pPlayer->m_IsLoged)
			{
				unsigned long long int money = pPlayer->ReadMoney();
				if((pChr->Core()->m_ActiveWeapon == WEAPON_HAMMER) && pChr->m_Fire && !pChr->m_Buyed && pPlayer->my_score != NSkyb::PLAYER_MAXIMUM_SCORE && money != 0)
				{
					if(money >= NSkyb::MONEY_WITHDRAW_AMOUNT)
					{
						if((pPlayer->my_score + NSkyb::MONEY_WITHDRAW_AMOUNT) <= NSkyb::PLAYER_MAXIMUM_SCORE)
						{
							pPlayer->ChangeMoney(NSkyb::MONEY_WITHDRAW_AMOUNT * -1);
							pPlayer->my_score += NSkyb::MONEY_WITHDRAW_AMOUNT;
						}
						else if((pPlayer->my_score + NSkyb::MONEY_WITHDRAW_AMOUNT) >= NSkyb::PLAYER_MAXIMUM_SCORE)
						{
							pPlayer->ChangeMoney((NSkyb::PLAYER_MAXIMUM_SCORE - pPlayer->my_score) * -1);
							pPlayer->my_score = NSkyb::PLAYER_MAXIMUM_SCORE;
						}
					}
					else if(money < NSkyb::MONEY_WITHDRAW_AMOUNT)
					{
						pPlayer->my_score += money;
						pPlayer->ChangeMoney(-money);
					}

					GameServer()->CreateSound(pChr->Core()->m_Pos, SOUND_HIT);
					pChr->SetEmote(EMOTE_HAPPY, Server()->Tick() + 500 * Server()->TickSpeed() / 1000);
					GameServer()->CreateDeath(pChr->Core()->m_Pos, ClientID);

					pChr->m_Buyed = true;
					pChr->m_IsMoneyChanged = true;
				}
				if(!pChr->m_PriceShown || pChr->m_IsMoneyChanged)
				{
					char abuff[200];
					pChr->m_IsMoneyChanged = false;
					str_format(abuff, sizeof(abuff), "Saved Money: %llu", pPlayer->ReadMoney());
					GameServer()->SendBroadcast(abuff, pPlayer->GetCID());
					pChr->m_PriceShown = true;
				}
			}
			else if(!pChr->m_MaximumShown)
			{
				GameServer()->SendBroadcast("You need a bank account first. try /register or /login", ClientID);
				pChr->m_MaximumShown = true;
			}
			break;

			default:
			break;
		}
		pChr->m_BeenInShop = true;
		pChr->m_BeenPos = pChr->Core()->m_Pos;
	}



}

void CGameControllerDDRace::OnPlayerConnect(CPlayer *pPlayer)
{
	IGameController::OnPlayerConnect(pPlayer);
	int ClientID = pPlayer->GetCID();

	// init the player
	Score()->PlayerData(ClientID)->Reset();

	// Can't set score here as LoadScore() is threaded, run it in
	// LoadScoreThreaded() instead
	Score()->LoadPlayerData(ClientID);

	if(!Server()->ClientPrevIngame(ClientID))
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "'%s' entered and joined the %s", Server()->ClientName(ClientID), GetTeamName(pPlayer->GetTeam()));
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, aBuf, -1, CGameContext::CHAT_SIX);

		// GameServer()->SendChatTarget(ClientID, "DDraceNetwork Mod. Version: " GAME_VERSION);
		GameServer()->SendChatTarget(ClientID, "Telegram: @ddskyblock");
	}
}

void CGameControllerDDRace::OnPlayerDisconnect(CPlayer *pPlayer, const char *pReason)
{
	int ClientID = pPlayer->GetCID();
	bool WasModerator = pPlayer->m_Moderating && Server()->ClientIngame(ClientID);

	IGameController::OnPlayerDisconnect(pPlayer, pReason);

	if(!GameServer()->PlayerModerating() && WasModerator)
		GameServer()->SendChat(-1, CGameContext::CHAT_ALL, "Server kick/spec votes are no longer actively moderated.");

	if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO)
		Teams().SetForceCharacterTeam(ClientID, TEAM_FLOCK);
}

void CGameControllerDDRace::OnReset()
{
	IGameController::OnReset();
	Teams().Reset();
}

void CGameControllerDDRace::Tick()
{
	static bool once = false;
	char abuff[100];
	static CCharacter *pChr;
	static CCharacter *pChrVictim;
	static int ClientID;

	static bool SetMap = false;
	if(!SetMap)
	{
		Server()->ChangeMap("SkyBlock Alpha v0.1");
		SetMap = true;
	}


	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		pChr = GameServer()->GetPlayerChar(i);

		if(pChr)
		{
			ClientID = pChr->GetPlayer()->GetCID();
			// GameServer()->SendChatTarget(ClientID, GameServer()->m_apPlayers[ClientID]->m_Account.m_Username);	
			
			//hooking score stuff
			pChrVictim = GameServer()->GetPlayerChar(pChr->Core()->HookedPlayer());
			if((pChr->Core()->HookedPlayer() != -1) && (pChrVictim = GameServer()->GetPlayerChar(pChr->Core()->HookedPlayer())))
			{
				if(pChr->IsAlive() )
				{
					pChrVictim->m_PlayerHooker = i;
					pChrVictim->m_PlayerHookerLastTick = Server()->Tick();
				}
			}
			pChr->PlayerHookerNormelizer();


			
			//HookUps
			if(pChr->m_Hook_Ups && (pChr->Core()->HookedPlayer() != -1) && pChr->m_UsedHookUps)
			{
				pChr->Core()->m_HookTick--;
				pChr->m_UsedHookUps--;
			}else if(pChr->m_UsedHookUps <= (pChr->m_UsedHookUps *pChr->m_Hook_Ups) && pChr->Core()->HookedPlayer() == -1)
			{
				pChr->m_UsedHookUps = pChr->HookTimeUpPerUpgrade * pChr->m_Hook_Ups;
			}
		}
	}


	IGameController::Tick();
	Teams().ProcessSaveTeam();
	Teams().Tick();
}

void CGameControllerDDRace::DoTeamChange(class CPlayer *pPlayer, int Team, bool DoChatMsg)
{
	Team = ClampTeam(Team);
	if(Team == pPlayer->GetTeam())
		return;

	CCharacter *pCharacter = pPlayer->GetCharacter();

	if(Team == TEAM_SPECTATORS)
	{
		if(g_Config.m_SvTeam != SV_TEAM_FORCED_SOLO && pCharacter)
		{
			// Joining spectators should not kill a locked team, but should still
			// check if the team finished by you leaving it.
			int DDRTeam = pCharacter->Team();
			Teams().SetForceCharacterTeam(pPlayer->GetCID(), TEAM_FLOCK);
			Teams().CheckTeamFinished(DDRTeam);
		}
	}

	IGameController::DoTeamChange(pPlayer, Team, DoChatMsg);
}
