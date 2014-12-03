// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "SlateBasics.h"
#include "SUWDialog.h"

#if !UE_SERVER

#include "Http.h"

class SUWRedirectDialog : public SUWDialog
{
public:

	SLATE_BEGIN_ARGS(SUWRedirectDialog)
		: _DialogSize(FVector2D(0.5f, 0.25f))
		, _bDialogSizeIsRelative(true)
		, _DialogPosition(FVector2D(0.5f, 0.5f))
		, _DialogAnchorPoint(FVector2D(0.5f, 0.5f))
		, _ContentPadding(FVector2D(10.0f, 5.0f))
		, _ButtonMask(UTDIALOG_BUTTON_CANCEL)
	{}
	SLATE_ARGUMENT(TWeakObjectPtr<class UUTLocalPlayer>, PlayerOwner)
		SLATE_ARGUMENT(FText, DialogTitle)
		SLATE_ARGUMENT(FVector2D, DialogSize)
		SLATE_ARGUMENT(bool, bDialogSizeIsRelative)
		SLATE_ARGUMENT(FVector2D, DialogPosition)
		SLATE_ARGUMENT(FVector2D, DialogAnchorPoint)
		SLATE_ARGUMENT(FVector2D, ContentPadding)
		SLATE_ARGUMENT(uint16, ButtonMask)
		SLATE_EVENT(FDialogResultDelegate, OnDialogResult)
		SLATE_ARGUMENT(FString, RedirectToURL)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);
protected:

	virtual FReply OnButtonClick(uint16 ButtonID);

	TOptional<float> GetProgressFile() const
	{
		if (AssetsTotalSize == 0)
		{
			return 0.0f;
		}

		return float(AssetsDownloadedAmount) / float(AssetsTotalSize);
	}

	FText GetFileName() const
	{
		return FText::FromString(RedirectToURL);
	}
	
	FText GetProgressFileText() const
	{
		if (AssetsTotalSize == 0)
		{
			return FText::FromString(TEXT("Connecting..."));
		}

		return FText::FromString(FString::FromInt(AssetsDownloadedAmount) + TEXT(" bytes / ") + FString::FromInt(AssetsTotalSize) + TEXT(" bytes"));
	}

	bool DownloadFile(FString URL);
	void CancelDownload();

	bool bDownloadCanceled;

	void HttpRequestProgress(FHttpRequestPtr HttpRequest, int32 NumBytes);
	void HttpRequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
	FHttpRequestPtr HttpRequest;

	FString RedirectToURL;
	
	int32 AssetsTotalSize;
	int32 AssetsDownloadedAmount;
public:
	virtual void Tick(const FGeometry & AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
};

#endif