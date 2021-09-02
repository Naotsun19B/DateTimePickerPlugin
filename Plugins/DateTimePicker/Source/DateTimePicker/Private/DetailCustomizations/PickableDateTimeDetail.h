// Copyright 2021 Naotsun. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DetailCustomizations.h"
#include "IPropertyTypeCustomization.h"

class SComboButton;
class FDetailWidgetRow;
class IDetailChildrenBuilder;
class IPropertyHandle;

/**
 * Detail Customizetion that allows you to specify a structure that inherits from 
 * FPulldownStructBase using the structure picker.
 */
class DATETIMEPICKER_API FPickableDateTimeDetail : public IPropertyTypeCustomization
{
public:
	// Register-Unregister and instantiate this customization.
	static void Register();
	static void Unregister();
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// IPropertyTypeCustomization interface.
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> InStructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	// End of IPropertyTypeCustomization interface.
	
private:
	// Get the actual value from the DateTimeHandle.
	TSharedPtr<FDateTime> GetDateTime() const;
	
	// Returns the text displayed on the combo button.
	FText GetComboTextValue() const;

	// Create the date time picker widget.
	TSharedRef<SWidget> HandleOnGetMenuContent();

	// Called when a structure is selected from the list.
	void HandleOnDateTimePicked(const FDateTime& PickedDateTime);
	
private:
	// Handle for accessing FPickableDateTime::DateTime.
	TSharedPtr<IPropertyHandle> DateTimeHandle;

	// ComboButton to launch the date time picker when you click on a property.
	TSharedPtr<SComboButton> StructPickerAnchor;
};