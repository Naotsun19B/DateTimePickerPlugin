// Copyright 2021 Naotsun. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SUniformGridPanel;

/**
 * Widget that displays the calendar and lets you select the date and time.
 */
class DATETIMEPICKER_API SDateTimePicker : public SCompoundWidget
{
public:
	// Defines an event to be called when a date and time is selected in the DateTimePicker.
	DECLARE_DELEGATE_OneParam(FOnDateTimePicked, const FDateTime&);

	// Defines the mode type of DateTimePicker.
	enum class EDateTimePickerMode : uint8
	{
		Year,
		Month,
		Day,
		Hour,
		Minute,
		Second,
		Millisecond,
	};
	
public:
	SLATE_BEGIN_ARGS(SDateTimePicker)
		: _InitialSelection(nullptr)
	{}

	// Specifies the item that should be selected first.
	SLATE_ARGUMENT(TSharedPtr<FDateTime>, InitialSelection)

	// Called when a date and time is selected by the DateTimePicker.
	SLATE_EVENT(FOnDateTimePicked, OnDateTimePicked)
	
	SLATE_END_ARGS()

	void OnPressedPreviousMonth();
	int32 GetNormalizedDay(int32 InMonth);
	void OnPressedNextMonth();
	void OnPressedNow();
	void OnYearChanged();
	void OnPressedOkay();
	void OnPressedCancel();

	void Construct(const FArguments& InArgs);

private:
	// Rebuild the date and time on the calendar.
	void RebuildCalenderPanel();

	// Gets the year and month text to display as the calendar title.
	FText GetTitleText() const;

	// Called when a structure is selected from the list.
	void HandleOnDateTimePicked(const FDateTime& PickedDateTime);
	
private:
	// Current DateTimePicker mode.
	EDateTimePickerMode Mode = EDateTimePickerMode::Day;

	//An Array of day name
	const TCHAR* ShortDayNames[7] = { TEXT("Mon"), TEXT("Tue"), TEXT("Wed"), TEXT("Thu"), TEXT("Fri"), TEXT("Sat"), TEXT("Sun") };
	
	// Currently selected DateTime.
	FDateTime PendingDateTime;

	// Selected DataTime once open the Calendar
	FDateTime InitialDateTimeSelected;

	// A grid panel that displays the date and time of the calendar.
	TSharedPtr<SUniformGridPanel> CalendarPanel;
	
	// An event that is called when a date and time is selected by the DateTimePicker.
	FOnDateTimePicked OnDateTimePicked;
};
