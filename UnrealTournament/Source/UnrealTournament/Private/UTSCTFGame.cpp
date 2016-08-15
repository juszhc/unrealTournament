// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTTeamGameMode.h"
#include "UTHUD_SCTF.h"
#include "UTCTFGameMode.h"
#include "UTSCTFGame.h"
#include "UTSCTFFlagBase.h"
#include "UTSCTFFlag.h"
#include "UTSCTFGameState.h"
#include "UTSCTFGameMessage.h"
#include "UTCTFGameMessage.h"
#include "UTCTFRewardMessage.h"
#include "UTFirstBloodMessage.h"
#include "UTCountDownMessage.h"
#include "UTPickup.h"
#include "UTGameMessage.h"
#include "UTMutator.h"
#include "UTCTFSquadAI.h"
#include "UTWorldSettings.h"
#include "Widgets/SUTTabWidget.h"
#include "Dialogs/SUTPlayerInfoDialog.h"
#include "StatNames.h"
#include "Engine/DemoNetDriver.h"
#include "UTCTFScoreboard.h"
#include "UTShowdownRewardMessage.h"
#include "UTShowdownGameMessage.h"
#include "UTDroppedAmmoBox.h"
#include "UTDroppedLife.h"

AUTSCTFGame::AUTSCTFGame(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	FlagCapScore = 1;
	GoalScore = 3;
	TimeLimit = 0;
	InitialBoostCount = 0;
	DisplayName = NSLOCTEXT("UTGameMode", "SCTF", "Single Flag CTF");
	
	IntermissionDuration = 30.f;
	GameStateClass = AUTSCTFGameState::StaticClass();
	HUDClass = AUTHUD_SCTF::StaticClass();
	SquadType = AUTCTFSquadAI::StaticClass();
	RoundLives=0;
	bPerPlayerLives = false;
	bAsymmetricVictoryConditions = false;
	FlagSwapTime=10;
	FlagPickupDelay=0;
	FlagSpawnDelay=30;
	MapPrefix = TEXT("CTF");
	bHideInUI = true;
	bAttackerLivesLimited = false;
	bDefenderLivesLimited = false;
	bRollingAttackerSpawns = false;
	bWeaponStayActive = false;
}

void AUTSCTFGame::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	bForceRespawn = false;
	
	FlagSwapTime = FMath::Max(0, UGameplayStatics::GetIntOption(Options, TEXT("FlagSwapTime"), FlagSwapTime));
	FlagSpawnDelay = float(FMath::Max<int32>(0.0f, UGameplayStatics::GetIntOption(Options, TEXT("FlagSpawnDelay"), FlagSpawnDelay)));

}

void AUTSCTFGame::InitGameState()
{
	Super::InitGameState();
	SCTFGameState = Cast<AUTSCTFGameState>(UTGameState);
	if (SCTFGameState)
	{
		SCTFGameState->bWeightedCharacter = true;
		SCTFGameState->FlagBases.Empty();
		SCTFGameState->FlagBases.AddZeroed(2);
		for (TActorIterator<AUTSCTFFlagBase> It(GetWorld()); It; ++It)
		{
			if (It->bScoreBase )
			{
				if ( SCTFGameState->FlagBases.IsValidIndex(It->GetTeamNum()) )
				{
					SCTFGameState->FlagBases[It->GetTeamNum()] = *It;
				}
			}
			else if (SCTFGameState->FlagDispenser == nullptr)
			{
				SCTFGameState->FlagDispenser = *It;
			}
		}

		SCTFGameState->FlagSwapTime = FlagSwapTime;
	}
}


void AUTSCTFGame::InitRound()
{
	Super::InitRound();

	if (SCTFGameState->FlagDispenser) SCTFGameState->FlagDispenser->RoundReset();
	for (int32 i = 0; i < SCTFGameState->FlagBases.Num(); i++)
	{
		AUTSCTFFlagBase* Base  = Cast<AUTSCTFFlagBase>(SCTFGameState->FlagBases[i]);
		if (Base)
		{
			Base->RoundReset();
		}
	}

	SCTFGameState->AttackingTeam = 255;
	SCTFGameState->SetFlagSpawnTimer(FlagSpawnDelay);	// TODO: Make me an option
	FTimerHandle TempHandle;
	GetWorldTimerManager().SetTimer(TempHandle, this, &AUTSCTFGame::SpawnInitalFlag, FlagSpawnDelay * GetActorTimeDilation());

	SCTFGameState->bIsDefenseAbleToGainPowerup = false;
	SCTFGameState->bIsOffenseAbleToGainPowerup = false;
}

void AUTSCTFGame::SpawnInitalFlag()
{
	if (SCTFGameState->FlagDispenser)
	{
		SCTFGameState->FlagDispenser->Activate();	
	}
}

void AUTSCTFGame::FlagTeamChanged(uint8 NewTeamIndex)
{
	SCTFGameState->AttackingTeam = NewTeamIndex;
	for (int32 i = 0; i < SCTFGameState->FlagBases.Num(); i++)
	{
		AUTSCTFFlagBase* Base  = Cast<AUTSCTFFlagBase>(SCTFGameState->FlagBases[i]);
		if (Base)
		{
			if (Base->GetTeamNum() != NewTeamIndex)
			{
				Base->Deactivate();
			}
			else
			{
				Base->Activate();
			}
		}
	}

	if (SCTFGameState->Flag->GetTeamNum() != 255)
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
			if (PC)
			{
				PC->ClientReceiveLocalizedMessage(UUTSCTFGameMessage::StaticClass(), PC->GetTeamNum() == NewTeamIndex ? 2 : 3, nullptr, nullptr, nullptr);
			}
		}
	}

}

// Looks to see if a given team has a chance to keep playing
bool AUTSCTFGame::IsTeamStillAlive(uint8 TeamNum)
{
	// Look to see if anyone else is alive on this team...
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerState* PlayerState = Cast<AUTPlayerState>((*Iterator)->PlayerState);
		if (PlayerState && PlayerState->GetTeamNum() == TeamNum)
		{
			AUTCharacter* UTChar = Cast<AUTCharacter>((*Iterator)->GetPawn());
			if (!PlayerState->bOutOfLives || (UTChar && !UTChar->IsDead()))
			{
				return true;
			}
		}
	}
	return true;
}

bool AUTSCTFGame::CanFlagTeamSwap(uint8 NewTeamNum)
{
	return IsTeamStillAlive(NewTeamNum);
}
