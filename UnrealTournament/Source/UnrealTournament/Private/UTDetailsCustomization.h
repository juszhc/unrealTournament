// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#if WITH_EDITOR

#include "EditorStyleSet.h"
#include "BaseToolkit.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"

class FUTDetailsCustomization : public IDetailCustomization
{
private:
	IDetailLayoutBuilder* MostRecentBuilder;
public:
	FUTDetailsCustomization()
		: MostRecentBuilder(NULL)
	{}
	// Makes a new instance of this detail layout class for a specific detail view requesting it
	static TSharedRef<IDetailCustomization> MakeInstance()
	{
		return MakeShareable(new FUTDetailsCustomization);
	}

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) OVERRIDE;

	void OnPropChanged(const FPropertyChangedEvent& Event);
};

#endif