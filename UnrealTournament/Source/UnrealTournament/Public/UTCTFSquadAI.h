// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTSquadAI.h"

#include "UTCTFSquadAI.generated.h"

UCLASS(NotPlaceable)
class AUTCTFSquadAI : public AUTSquadAI
{
	GENERATED_UCLASS_BODY()

	UPROPERTY()
	AUTGameObjective* FriendlyBase;
	UPROPERTY()
	AUTGameObjective* EnemyBase;
	/** current friendly flag carrier hide location */
	UPROPERTY()
	FRouteCacheItem HideTarget;
	/** time flag carrier reached hide location, used for tracking how long it was able to hide */
	UPROPERTY()
	float StartHideTime;
	/** last time flag carrier was discovered while hiding, used to make the FC fight/run away for a while before trying to hide again */
	UPROPERTY()
	float HidingSpotDiscoveredTime;
	/** recent hiding spots to avoid reusing a spot too quickly */
	UPROPERTY()
	TArray< TWeakObjectPtr<UUTPathNode> > UsedHidingSpots;

	virtual void Initialize(AUTTeamInfo* InTeam, FName InOrders) override;
	virtual bool MustKeepEnemy(APawn* TheEnemy) override;
	virtual float ModifyEnemyRating(float CurrentRating, const FBotEnemyInfo& EnemyInfo, AUTBot* B) override;
	virtual bool CheckSquadObjectives(AUTBot* B) override;

	/** return whether the passed in location is close to an enemy base (e.g. if enemy carrier is this close, it's about to score) */
	virtual bool IsNearEnemyBase(const FVector& TestLoc);
	/** return action for flag carrier to take */
	virtual bool SetFlagCarrierAction(AUTBot* B);
	/** tell bot best action to recover friendly flag (assumed not at home base) */
	virtual bool RecoverFriendlyFlag(AUTBot* B);

	virtual void NotifyObjectiveEvent(AActor* InObjective, AController* InstigatedBy, FName EventName) override;
	virtual bool HasHighPriorityObjective(AUTBot* B);
};
