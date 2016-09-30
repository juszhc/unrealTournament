// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTWeap_GrenadeLauncher.h"
#include "UTProj_Grenade_Sticky.h"


AUTProj_Grenade_Sticky::AUTProj_Grenade_Sticky(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LifeTime = 20.0f;
	MinimumLifeTime = 0.2f;
}

void AUTProj_Grenade_Sticky::BeginPlay()
{
	Super::BeginPlay();

	if (Role == ROLE_Authority && !bFakeClientProjectile)
	{
		AUTCharacter* UTCharacter = Cast<AUTCharacter>(Instigator);
		if (UTCharacter)
		{ 
			GrenadeLauncherOwner = Cast<AUTWeap_GrenadeLauncher>(UTCharacter->GetWeapon());
			if (GrenadeLauncherOwner)
			{
				GrenadeLauncherOwner->RegisterStickyGrenade(this);
			}
		}

		GetWorldTimerManager().SetTimer(FLifeTimeHandle, this, &ThisClass::ExplodeDueToTimeout, LifeTime, false);
		GetWorldTimerManager().SetTimer(FArmedHandle, this, &ThisClass::ArmGrenade, MinimumLifeTime, false);
	}
}

void AUTProj_Grenade_Sticky::ArmGrenade()
{
	bArmed = true;
}

void AUTProj_Grenade_Sticky::Destroyed()
{
	Super::Destroyed();

	if (Role == ROLE_Authority)
	{
		GetWorldTimerManager().ClearTimer(FLifeTimeHandle);

		if (GrenadeLauncherOwner)
		{
			GrenadeLauncherOwner->UnregisterStickyGrenade(this);
		}
	}
}

void AUTProj_Grenade_Sticky::ExplodeDueToTimeout()
{
	ArmGrenade();
	Explode(GetActorLocation(), FVector(0, 0, 0), nullptr);
}

void AUTProj_Grenade_Sticky::Explode_Implementation(const FVector& HitLocation, const FVector& HitNormal, UPrimitiveComponent* HitComp)
{
	if (bArmed)
	{
		Super::Explode_Implementation(HitLocation, HitLocation, HitComp);
	}
}