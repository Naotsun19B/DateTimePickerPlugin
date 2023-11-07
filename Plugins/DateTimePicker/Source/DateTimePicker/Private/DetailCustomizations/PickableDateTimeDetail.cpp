// Copyright 2021 Naotsun. All Rights Reserved.

#include "DetailCustomizations/PickableDateTimeDetail.h"
#include "Widgets/SDateTimePicker.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SComboButton.h"

#define LOCTEXT_NAMESPACE "PickableDateTimeDetail"

void FPickableDateTimeDetail::Register()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FPickableDateTime::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FPickableDateTimeDetail::MakeInstance)
	);
}

void FPickableDateTimeDetail::Unregister()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FPickableDateTime::StaticStruct()->GetFName()
	);
}

TSharedRef<IPropertyTypeCustomization> FPickableDateTimeDetail::MakeInstance()
{
	return MakeShared<FPickableDateTimeDetail>();
}

void FPickableDateTimeDetail::CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 NumChildren;
	InStructPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIndex = 0; ChildIndex < NumChildren; ChildIndex++)
	{
		if (const TSharedPtr<IPropertyHandle> ChildHandle = InStructPropertyHandle->GetChildHandle(ChildIndex))
		{
			if (const FProperty* Property = ChildHandle->GetProperty())
			{
				if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(FPickableDateTime, DateTime))
				{
					DateTimeHandle = ChildHandle;
				}
			}
		}
	}
	
	HeaderRow
		.NameContent()
		[
			InStructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(200)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(StructPickerAnchor, SComboButton)
				.ContentPadding(FMargin(2, 2, 2, 1))
				.MenuPlacement(MenuPlacement_BelowAnchor)
				.ButtonContent()
				[
					SNew(STextBlock)
					.Text(this, &FPickableDateTimeDetail::GetComboTextValue)
				]
				.OnGetMenuContent(this, &FPickableDateTimeDetail::HandleOnGetMenuContent)
			]
		];
}

void FPickableDateTimeDetail::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

TSharedPtr<FDateTime> FPickableDateTimeDetail::GetDateTime() const
{
	check(DateTimeHandle.IsValid());
	
	TArray<void*> RawData;
	DateTimeHandle->AccessRawData(RawData);

	if (RawData.Num() != 1)
	{
		return nullptr;
	}

	const auto* DateTimePtr = static_cast<FDateTime*>(RawData[0]);
	if (DateTimePtr == nullptr)
	{
		return nullptr;
	}

	return MakeShared<FDateTime>(*DateTimePtr);
}

FText FPickableDateTimeDetail::GetComboTextValue() const
{
	const TSharedPtr<FDateTime> CurrentValue = GetDateTime();
	if (!CurrentValue.IsValid())
	{
		return LOCTEXT("MultipleValues", "Multiple Values");
	}
	
	return FText::AsDateTime(
		*CurrentValue,
		EDateTimeStyle::Default,
		EDateTimeStyle::Default,
		FText::GetInvariantTimeZone()
	);
}

TSharedRef<SWidget> FPickableDateTimeDetail::HandleOnGetMenuContent()
{
	return
		SNew(SDateTimePicker)
		.InitialSelection(GetDateTime())
		.OnDateTimePicked(this, &FPickableDateTimeDetail::HandleOnDateTimePicked);
}

void FPickableDateTimeDetail::HandleOnDateTimePicked(const FDateTime& PickedDateTime)
{
	check(DateTimeHandle.IsValid());

	TArray<void*> RawData;
	DateTimeHandle->AccessRawData(RawData);

	DateTimeHandle->NotifyPreChange();
	
	for (void* RawDataInstance : RawData)
	{
		*static_cast<FDateTime*>(RawDataInstance) = PickedDateTime;
	}
	
	DateTimeHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	DateTimeHandle->NotifyFinishedChangingProperties();

	if (StructPickerAnchor.IsValid())
	{
		StructPickerAnchor->SetIsOpen(false);
	}
}

#undef LOCTEXT_NAMESPACE
