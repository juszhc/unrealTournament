// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "UTHUD_CTF.h"
#include "SUTHudWindow.h"
#include "UTCTFGameState.h"
#include "UTFlagRunHUD.generated.h"

UCLASS()
class UNREALTOURNAMENT_API AUTFlagRunHUD : public AUTHUD_CTF
{
	GENERATED_UCLASS_BODY()

	virtual void DrawHUD() override;
	virtual void NotifyMatchStateChange() override;

	/** icon for player starts on the minimap (foreground) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, NoClear)
	FCanvasIcon PlayerStartIcon;

	int32 RedPlayerCount;
	int32 BluePlayerCount;
	float RedDeathTime;
	float BlueDeathTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToDelayMenuOpenForIntermission;

	virtual EInputMode::Type GetInputMode_Implementation() const;

	FTimerHandle MenuOpenDelayTimerHandle;

	void OpenPowerupSelectMenu();

protected:
	bool bConstructedPowerupWindowForDefense;
	bool bAlreadyForcedWindowOpening;

#if !UE_SERVER
	TSharedPtr<SUTHUDWindow> PowerupSelectWindow;
#endif

	// Manage the powerup select window.
	virtual void HandlePowerups();

};