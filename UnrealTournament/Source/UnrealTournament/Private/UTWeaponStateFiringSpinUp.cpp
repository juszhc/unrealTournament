// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTWeaponState.h"
#include "UTWeaponStateFiring.h"
#include "UTWeaponStateFiringSpinUp.h"

UUTWeaponStateFiringSpinUp::UUTWeaponStateFiringSpinUp(const FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{
	WarmupShotIntervals.Add(0.2f);
	WarmupShotIntervals.Add(0.18f);
	WarmupShotIntervals.Add(0.16f);
	WarmupShotIntervals.Add(0.14f);
	WarmupShotIntervals.Add(0.12f);
	CoolDownTime = 0.27f;
}

void UUTWeaponStateFiringSpinUp::BeginState(const UUTWeaponState* PrevState)
{
	CurrentShot = 0;
	ShotTimeRemaining = -0.001f;
	RefireCheckTimer();
	IncrementShotTimer();
	if (GetOuterAUTWeapon()->FireLoopingSound.IsValidIndex(GetFireMode()) && GetOuterAUTWeapon()->FireLoopingSound[GetFireMode()] != NULL)
	{
		GetUTOwner()->SetAmbientSound(GetOuterAUTWeapon()->FireLoopingSound[GetFireMode()]);
	}
	if (WarmupAnim != NULL)
	{
		UAnimInstance* AnimInstance = GetOuterAUTWeapon()->Mesh->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(WarmupAnim, 1.f);
		}
	}
	GetOuterAUTWeapon()->OnStartedFiring();
}

void UUTWeaponStateFiringSpinUp::EndState()
{
	Super::EndState();
	GetWorld()->GetTimerManager().ClearTimer(this, &UUTWeaponStateFiringSpinUp::CooldownFinished);
}

void UUTWeaponStateFiringSpinUp::IncrementShotTimer()
{
	ShotTimeRemaining += (WarmupShotIntervals.IsValidIndex(CurrentShot)) ? WarmupShotIntervals[CurrentShot] : GetOuterAUTWeapon()->GetRefireTime(GetOuterAUTWeapon()->GetCurrentFireMode());
}

void UUTWeaponStateFiringSpinUp::UpdateTiming()
{
	// unnecessary since we're manually incrementing
}

void UUTWeaponStateFiringSpinUp::RefireCheckTimer()
{
	// query bot to consider whether to still fire, switch modes, etc
	AUTBot* B = Cast<AUTBot>(GetUTOwner()->Controller);
	if (B != NULL)
	{
		B->CheckWeaponFiring();
	}

	if (!GetOuterAUTWeapon()->HandleContinuedFiring())
	{
		// spin down instead of going to active immediately
		float TotalWarmupTime = 0.0f;
		float WarmupTimeUsed = 0.0f;
		for (int32 i = 0; i < WarmupShotIntervals.Num(); i++)
		{
			TotalWarmupTime += WarmupShotIntervals[i];
			if (i <= CurrentShot)
			{
				WarmupTimeUsed += WarmupShotIntervals[i];
			}
		}
		if (CurrentShot < WarmupShotIntervals.Num())
		{
			WarmupTimeUsed -= ShotTimeRemaining;
		}
		if (WarmupTimeUsed <= 0.0f)
		{
			GetOuterAUTWeapon()->GotoActiveState();
		}
		else
		{
			if (CooldownAnim != NULL)
			{
				UAnimInstance* AnimInstance = GetOuterAUTWeapon()->Mesh->GetAnimInstance();
				if (AnimInstance != NULL)
				{
					AnimInstance->Montage_Play(CooldownAnim, 1.f / (WarmupTimeUsed / TotalWarmupTime));
				}
			}
			GetWorld()->GetTimerManager().SetTimer(this, &UUTWeaponStateFiringSpinUp::CooldownFinished, CoolDownTime * (WarmupTimeUsed / TotalWarmupTime), false);
		}
	}
	else
	{
		FireShot();
		CurrentShot++;
		IncrementShotTimer();
		if (CurrentShot == WarmupShotIntervals.Num() && FiringLoopAnim != NULL)
		{
			UAnimInstance* AnimInstance = GetOuterAUTWeapon()->Mesh->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FiringLoopAnim, 1.f);
			}
		}
	}
}

void UUTWeaponStateFiringSpinUp::CooldownFinished()
{
	GetOuterAUTWeapon()->GotoActiveState();
}

void UUTWeaponStateFiringSpinUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetWorld()->GetTimerManager().IsTimerActive(this, &UUTWeaponStateFiringSpinUp::CooldownFinished))
	{
		ShotTimeRemaining -= DeltaTime * GetUTOwner()->GetFireRateMultiplier();
		if (ShotTimeRemaining <= 0.0f)
		{
			RefireCheckTimer();
		}
	}
}