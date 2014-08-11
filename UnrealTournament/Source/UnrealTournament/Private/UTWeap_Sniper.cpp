// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTWeap_Sniper.h"
#include "UTProj_Sniper.h"
#include "UTWeaponState.h"
#include "UTWeaponStateFiring.h"
#include "UTWeaponStateZooming.h"

AUTWeap_Sniper::AUTWeap_Sniper(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP.SetDefaultSubobjectClass<UUTWeaponStateZooming>(TEXT("FiringState1")) )
{
	BringUpTime = 0.54f;
	PutDownTime = 0.41f;
	SlowHeadshotScale = 1.75f;
	RunningHeadshotScale = 0.8f;
	ProjClass.Insert(AUTProj_Sniper::StaticClass(), 0);
	if (FiringState.Num() > 1)
	{
#if WITH_EDITORONLY_DATA
		FiringStateType[1] = UUTWeaponStateZooming::StaticClass();
#endif
	}

	IconCoordinates = FTextureUVs(726,532,165,51);
}

AUTProjectile* AUTWeap_Sniper::FireProjectile()
{
	AUTProj_Sniper* SniperProj = Cast<AUTProj_Sniper>(Super::FireProjectile());
	if (SniperProj != NULL)
	{
		if (GetUTOwner()->GetVelocity().Size() <= GetUTOwner()->CharacterMovement->MaxWalkSpeedCrouched)
		{
			SniperProj->HeadScaling *= SlowHeadshotScale;
		}
		else
		{
			SniperProj->HeadScaling *= RunningHeadshotScale;
		}
	}
	return SniperProj;
}
