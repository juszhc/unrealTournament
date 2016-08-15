// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTCarriedObject.h"
#include "UTGhostFlag.generated.h"

UCLASS(meta = (ChildCanTick))
class UNREALTOURNAMENT_API AUTGhostFlag : public AActor
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Pickup)
	UParticleSystemComponent* TimerEffect;

	UPROPERTY(ReplicatedUsing=OnSetCarriedObject, BlueprintReadOnly, Category = Flag)
	AUTCarriedObject* MyCarriedObject;

	UPROPERTY(ReplicatedUsing=OnSetMidPoint)
	FVector MidPoints[3];

	UPROPERTY()
	class AUTFlagReturnTrail* Trail;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AUTFlagReturnTrail> TrailClass;

	UPROPERTY()
	int32 TeamIndex;

	UPROPERTY()
	float TrailSpawnTime;

	UFUNCTION()
	virtual void OnSetCarriedObject();

	UFUNCTION()
	virtual void OnSetMidPoint();

	virtual void SetCarriedObject(AUTCarriedObject* NewCarriedObject, const FFlagTrailPos NewPosition);
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	virtual void OnRep_ReplicatedMovement() override;
};