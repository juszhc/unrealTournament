// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLocalMessage.h"
#include "UTRedeemerLaunchAnnounce.h"
#include "GameFramework/LocalMessage.h"
#include "UTCharacterVoice.h"

UUTRedeemerLaunchAnnounce::UUTRedeemerLaunchAnnounce(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsSpecial = true;
	bIsUnique = true;
	bIsConsoleMessage = false;
	bIsStatusAnnouncement = false;
	Lifetime = 2.f;
	AnnouncementDelay = 0.f;
	MessageArea = FName(TEXT("Announcements"));
	MessageSlot = FName(TEXT("MajorRewardMessage"));
	RedeemerOnlineText = NSLOCTEXT("RedeemerMessage", "RedeemerOnline", "Redeemer Online");
}

FText UUTRedeemerLaunchAnnounce::GetText(int32 Switch, bool bTargetsPlayerState1, class APlayerState* RelatedPlayerState_1, class APlayerState* RelatedPlayerState_2, class UObject* OptionalObject) const
{
	if (Switch == 2)
	{
		return RedeemerOnlineText;
	}
	return FText::GetEmpty();
}

FName UUTRedeemerLaunchAnnounce::GetAnnouncementName_Implementation(int32 Switch, const UObject* OptionalObject, const class APlayerState* RelatedPlayerState_1, const class APlayerState* RelatedPlayerState_2) const
{
	switch(Switch)
	{
	case 0: return TEXT("RZE_MissileIncoming"); break;
	case 1: return TEXT("RZE_LaunchConfirmed"); break;
	case 2: return TEXT("RZE_RedeemerOnline"); break;
	}
	return NAME_None;
}

void UUTRedeemerLaunchAnnounce::PrecacheAnnouncements_Implementation(class UUTAnnouncer* Announcer) const
{
	for (int32 i = 0; i < 3; i++)
	{
		FName SoundName = GetAnnouncementName(i, NULL, NULL, NULL);
		if (SoundName != NAME_None)
		{
			Announcer->PrecacheAnnouncement(SoundName);
		}
	}
}

bool UUTRedeemerLaunchAnnounce::ShouldPlayAnnouncement(const FClientReceiveData& ClientData) const
{
	return true;
}

float UUTRedeemerLaunchAnnounce::GetAnnouncementDelay(int32 Switch)
{
	return (Switch == 2) ? 0.2f : 0.f;
}

bool UUTRedeemerLaunchAnnounce::InterruptAnnouncement_Implementation(int32 Switch, const UObject* OptionalObject, TSubclassOf<UUTLocalMessage> OtherMessageClass, int32 OtherSwitch, const UObject* OtherOptionalObject) const
{
	if (Switch == 2)
	{
		// by default interrupt messages of same type, and countdown messages
		return (GetClass() == OtherMessageClass) || Cast<UUTCharacterVoice>(OtherMessageClass->GetDefaultObject());
	}
	return Super::InterruptAnnouncement_Implementation(Switch, OptionalObject, OtherMessageClass, OtherSwitch, OtherOptionalObject);
}
