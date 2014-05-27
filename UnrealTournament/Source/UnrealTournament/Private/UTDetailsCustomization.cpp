// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTDetailsCustomization.h"
#include "UTWeaponAttachment.h"
#if WITH_EDITOR
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/PropertyEditor/Public/IDetailPropertyRow.h"
#include "Editor/PropertyEditor/Public/DetailWidgetRow.h"
#include "Editor/PropertyEditor/Public/PropertyHandle.h"
#include "Editor/PropertyEditor/Public/IDetailsView.h"

template<typename OptionType>
class SComboBoxMF : public SComboBox<OptionType>
{
private:
	TSharedPtr<struct FMuzzleFlashItem> DelegateObject;
public:
	void SetDelegateObject(TSharedPtr<struct FMuzzleFlashItem> InDelegateObject)
	{
		DelegateObject = InDelegateObject;
	}
};

struct FMuzzleFlashChoice
{
	TWeakObjectPtr<UParticleSystemComponent> PSC;
	FName DisplayName;
	FMuzzleFlashChoice()
		: PSC(NULL), DisplayName(NAME_None)
	{}
	FMuzzleFlashChoice(const TWeakObjectPtr<UParticleSystemComponent>& InPSC, const FName& InName)
		: PSC(InPSC), DisplayName(InName)
	{}
	bool operator==(const FMuzzleFlashChoice& Other)
	{
		return PSC == Other.PSC;
	}
	bool operator!=(const FMuzzleFlashChoice& Other)
	{
		return PSC != Other.PSC;
	}
};

struct FMuzzleFlashItem : public TSharedFromThis<FMuzzleFlashItem>
{
	/** index in muzzle flash array */
	uint32 Index;
	/** object being modified */
	TWeakObjectPtr<UObject> Obj;
	/** builder */
	IDetailLayoutBuilder& Builder;
	/** choices */
	TArray<TSharedPtr<FMuzzleFlashChoice>> Choices;

	TSharedPtr<STextBlock> TextBlock;

	FMuzzleFlashItem(uint32 InIndex, TWeakObjectPtr<UObject> InObj, IDetailLayoutBuilder& InBuilder, const TArray<TSharedPtr<FMuzzleFlashChoice>>& InChoices)
		: Index(InIndex), Obj(InObj), Builder(InBuilder), Choices(InChoices)
	{
	}

	TSharedRef<SComboBoxMF<TSharedPtr<FMuzzleFlashChoice>>> Init(IDetailCategoryBuilder& Category)
	{
		FString CurrentText;
		{
			UParticleSystemComponent* CurrentValue = NULL;
			AUTWeapon* Weap = Cast<AUTWeapon>(Obj.Get());
			if (Weap != NULL)
			{
				CurrentValue = Weap->MuzzleFlash[Index];
			}
			else
			{
				AUTWeaponAttachment* Attachment = Cast<AUTWeaponAttachment>(Obj.Get());
				if (Attachment != NULL)
				{
					CurrentValue = Attachment->MuzzleFlash[Index];
				}
			}
			for (int32 i = 0; i < Choices.Num(); i++)
			{
				if (Choices[i]->PSC == CurrentValue)
				{
					CurrentText = Choices[i]->DisplayName.ToString();
				}
			}
		}

		TSharedRef<SComboBoxMF<TSharedPtr<FMuzzleFlashChoice>>> Ref = SNew(SComboBoxMF<TSharedPtr<FMuzzleFlashChoice>>)
			.OptionsSource(&Choices)
			.OnGenerateWidget(this, &FMuzzleFlashItem::GenerateWidget)
			.OnSelectionChanged(this, &FMuzzleFlashItem::ComboChanged)
			.Content()
			[
				SAssignNew(TextBlock, STextBlock)
				.Text(CurrentText)
			];
		Category.AddCustomRow(TEXT("MuzzleFlash"))
			[
				SNew(SSplitter)
				+ SSplitter::Slot()
				[
					SNew(STextBlock)
					.Text(FString::Printf(TEXT("Set MuzzleFlash[%d]"), Index))
				]
				+ SSplitter::Slot()
				[
					Ref
				]
			];
		return Ref;
	}

	void ComboChanged(TSharedPtr<FMuzzleFlashChoice> NewSelection, ESelectInfo::Type SelectInfo)
	{
		if (Obj.IsValid())
		{
			UParticleSystemComponent* NewValue = NewSelection->PSC.Get();
			AUTWeapon* Weap = Cast<AUTWeapon>(Obj.Get());
			if (Weap != NULL)
			{
				if (Weap->MuzzleFlash.IsValidIndex(Index))
				{
					Weap->MuzzleFlash[Index] = NewValue;
					TextBlock->SetText(NewSelection->DisplayName.ToString());
				}
			}
			else
			{
				AUTWeaponAttachment* Attachment = Cast<AUTWeaponAttachment>(Obj.Get());
				if (Attachment != NULL && Attachment->MuzzleFlash.IsValidIndex(Index))
				{
					Attachment->MuzzleFlash[Index] = NewValue;
					TextBlock->SetText(NewSelection->DisplayName.ToString());
				}
			}
		}
		Builder.ForceRefreshDetails();
	}

	TSharedRef<SWidget> GenerateWidget(TSharedPtr<FMuzzleFlashChoice> InItem)
	{
		return SNew(SBox)
			.Padding(5)
			[
				SNew(STextBlock)
				.Text(InItem->DisplayName.ToString())
			];
	}
};

void FUTDetailsCustomization::OnPropChanged(const FPropertyChangedEvent& Event)
{
	if (MostRecentBuilder != NULL && (Event.Property == NULL || Event.Property->GetFName() == FName(TEXT("MuzzleFlash"))))
	{
		MostRecentBuilder->ForceRefreshDetails();
	}
}

void FUTDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailLayout.GetObjectsBeingCustomized(Objects);
	if (Objects.Num() == 1 && Objects[0].IsValid())
	{
		MostRecentBuilder = &DetailLayout;
		const_cast<IDetailsView&>(DetailLayout.GetDetailsView()).OnFinishedChangingProperties().AddSP(this, &FUTDetailsCustomization::OnPropChanged);

		IDetailCategoryBuilder& WeaponCategory = DetailLayout.EditCategory("Weapon");

		TSharedRef<IPropertyHandle> MuzzleFlash = DetailLayout.GetProperty(TEXT("MuzzleFlash"));
		WeaponCategory.AddProperty(MuzzleFlash); // causes array to list first

		uint32 NumChildren = 0;
		MuzzleFlash->GetNumChildren(NumChildren);

		TArray<TSharedPtr<FMuzzleFlashChoice>> Choices;
		Choices.Add(MakeShareable(new FMuzzleFlashChoice(NULL, NAME_None)));
		{
			// the components editor uses names from the SCS instead so that's what we need to use
			TArray<USCS_Node*> ConstructionNodes;
			{
				TArray<const UBlueprintGeneratedClass*> ParentBPClassStack;
				UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(Objects[0].Get()->GetClass(), ParentBPClassStack);
				for (int32 i = ParentBPClassStack.Num() - 1; i >= 0; i--)
				{
					const UBlueprintGeneratedClass* CurrentBPGClass = ParentBPClassStack[i];
					if (CurrentBPGClass->SimpleConstructionScript)
					{
						ConstructionNodes += CurrentBPGClass->SimpleConstructionScript->GetAllNodes();
					}
				}
			}
			TArray<UObject*> Children;
			GetObjectsWithOuter(Objects[0].Get(), Children, true, RF_PendingKill);
			GetObjectsWithOuter(Objects[0].Get()->GetClass(), Children, true, RF_PendingKill);
			for (int32 i = 0; i < Children.Num(); i++)
			{
				UParticleSystemComponent* PSC = Cast<UParticleSystemComponent>(Children[i]);
				if (PSC != NULL)
				{
					FName DisplayName = PSC->GetFName();
					for (int32 j = 0; j < ConstructionNodes.Num(); j++)
					{
						if (ConstructionNodes[j]->ComponentTemplate == PSC)
						{
							DisplayName = ConstructionNodes[j]->VariableName;
							break;
						}
					}
					Choices.Add(MakeShareable(new FMuzzleFlashChoice(PSC, DisplayName)));
				}
			}
		}

		for (uint32 i = 0; i < NumChildren; i++)
		{
			TSharedRef<IPropertyHandle> Element = MuzzleFlash->GetChildHandle(i).ToSharedRef();

			TSharedPtr<FMuzzleFlashItem> DelegateObject = MakeShareable(new FMuzzleFlashItem(i, Objects[0], DetailLayout, Choices));
			DelegateObject->Init(WeaponCategory)->SetDelegateObject(DelegateObject);
		}
	}
}
#endif