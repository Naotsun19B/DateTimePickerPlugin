// Copyright 2021 Naotsun. All Rights Reserved.

#include "Widgets/SDateTimePicker.h"
#include "Components/VerticalBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SGridPanel.h"

namespace DateTimePickerInternal
{
	// Calendar date grid button widget.
	class SDateTimeGrid : public SButton
	{
	public:
		SLATE_BEGIN_ARGS(SDateTimeGrid) {}
		SLATE_ARGUMENT(FDateTime, DateTime)
		SLATE_ARGUMENT(bool, ShouldBeGrayOut)
		SLATE_ARGUMENT(SDateTimePicker::EDateTimePickerMode, Mode)
		SLATE_EVENT(SDateTimePicker::FOnDateTimePicked, OnDateTimePicked)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DateTime = InArgs._DateTime;
			OnDateTimePicked = InArgs._OnDateTimePicked;

			int32 DisplayNumber = 0;
			switch (InArgs._Mode)
			{
				case SDateTimePicker::EDateTimePickerMode::Year:
					DisplayNumber = DateTime.GetYear();
					break;
				case SDateTimePicker::EDateTimePickerMode::Month:
					DisplayNumber = DateTime.GetMonth();
					break;
				case SDateTimePicker::EDateTimePickerMode::Day:
					DisplayNumber = DateTime.GetDay();
					break;
				case SDateTimePicker::EDateTimePickerMode::Hour:
					DisplayNumber = DateTime.GetHour();
					break;
				case SDateTimePicker::EDateTimePickerMode::Minute:
					DisplayNumber = DateTime.GetMinute();
					break;
				case SDateTimePicker::EDateTimePickerMode::Second:
					DisplayNumber = DateTime.GetSecond();
					break;
				case SDateTimePicker::EDateTimePickerMode::Millisecond:
					DisplayNumber = DateTime.GetMillisecond();
					break;
				default: break;
			}
			
			SButton::Construct(
				SButton::FArguments()
				.ButtonColorAndOpacity(InArgs._ShouldBeGrayOut ? FLinearColor(FVector(0.3f)) : FLinearColor(FVector(0.8f)))
				.Text(FText::FromString(FString::FromInt(DisplayNumber)))
				.OnPressed(this, &SDateTimeGrid::HandleOnPressed)
			);
		}

	private:
		void HandleOnPressed()
		{
			if (OnDateTimePicked.IsBound())
			{
				OnDateTimePicked.Execute(DateTime);
			}
		}

	private:
		FDateTime DateTime;
		SDateTimePicker::FOnDateTimePicked OnDateTimePicked;
	};

	// EDayOfWeek starts on Monday, so get the index starting on Sunday for the calendar.
	static int32 GetDaysThatPassedSinceSunday(EDayOfWeek DayOfWeek)
	{
		int32 DaysThatPassedSinceSunday = static_cast<int32>(DayOfWeek) + 1;
		if (DaysThatPassedSinceSunday >= 7)
		{
			DaysThatPassedSinceSunday = 0;
		}

		return DaysThatPassedSinceSunday;
	}

	// The dataset needed to generate the calendar.
	struct FGenerateCalenderGrids
	{
	public:
		// The horizontal number of grids.
		int32 RowNum;
		// The vertical number of grids.
		int32 ColumnNum;
		// The time added to going through the grid.
		int64 Timespan;
		// Maximum number of grids.
		int32 MaxIndex;
		// A function that calculates the Date Time of the first grid.
		TFunction<FDateTime(const FDateTime& PendingDateTime)> GetFirstDate;
		// A function that determines whether to gray out a button.
		TFunction<bool(const FDateTime& PendingDateTime, const FDateTime& DateTime)> ShouldBeGrayOut;

		FGenerateCalenderGrids(
			int32 InRowNum,
			int32 InColumnNum,
			int64 InTimespan,
			int32 InMaxIndex = INDEX_NONE,
			TFunction<FDateTime(const FDateTime& PendingDateTime)> InGetFirstDate = nullptr,
			TFunction<bool(const FDateTime& PendingDateTime, const FDateTime& DateTime)> InShouldBeGrayOut = nullptr
		)
			: RowNum(InRowNum)
			, ColumnNum(InColumnNum)
			, Timespan(InTimespan)
			, MaxIndex(InMaxIndex)
			, GetFirstDate(InGetFirstDate)
			, ShouldBeGrayOut(InShouldBeGrayOut)
		{
			if (GetFirstDate == nullptr)
			{
				GetFirstDate = [](const FDateTime& PendingDateTime) -> FDateTime
				{
					return PendingDateTime;
				};
			}
			
			if (ShouldBeGrayOut == nullptr)
			{
				ShouldBeGrayOut = [](const FDateTime& PendingDateTime, const FDateTime& DateTime) -> bool
				{
					return false;
				};
			}
		}
	};

	// Information about calendar grid generation for each mode.
	static const TMap<SDateTimePicker::EDateTimePickerMode, FGenerateCalenderGrids> GenerateCalenderGridsInfos =
	{
		{
			SDateTimePicker::EDateTimePickerMode::Day,
			FGenerateCalenderGrids(
				// The row is the number of days of the week, so 7 and the column should be 6 so
				// that no matter what day of the week the first day is, problems will not occur.
				static_cast<int32>(EDayOfWeek::Sunday) + 1, 6,
				ETimespan::TicksPerDay,
				INDEX_NONE,
				[](const FDateTime& PendingDateTime) -> FDateTime
				{
					const FDateTime FirstDate(PendingDateTime.GetYear(), PendingDateTime.GetMonth(), 1);
					const EDayOfWeek FirstDayOfWeek = FirstDate.GetDayOfWeek();
					return FirstDate.GetTicks() - (ETimespan::TicksPerDay * DateTimePickerInternal::GetDaysThatPassedSinceSunday(FirstDayOfWeek));
				},
				[](const FDateTime& PendingDateTime, const FDateTime& DateTime) -> bool
				{
					return (PendingDateTime.GetMonth() != DateTime.GetMonth());
				}
			)
		}
	};
}

void SDateTimePicker::Construct(const FArguments& InArgs)
{
	if (InArgs._InitialSelection.IsValid())
	{
		PendingDateTime = *InArgs._InitialSelection;
	}
	else
	{
		PendingDateTime = FDateTime::Now();
	}

	OnDateTimePicked = InArgs._OnDateTimePicked;
	
	ChildSlot
	[
		SNew(SBox)
		.Padding(5.f)
		[
			// Show year and month text and button to switch between months.
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.MaxHeight(50.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(this, &SDateTimePicker::GetTitleText)
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SButton)
					]
					+ SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					[
						SNew(SButton)
					]
				]
			]
			// A grid panel that displays the date and time of the calendar.
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(CalendarPanel, SGridPanel)
			]
		]
	];

	RebuildCalenderPanel();
}

void SDateTimePicker::RebuildCalenderPanel()
{
	check(CalendarPanel.IsValid());

	CalendarPanel->ClearChildren();

	const DateTimePickerInternal::FGenerateCalenderGrids& Info = DateTimePickerInternal::GenerateCalenderGridsInfos[Mode];
	const FDateTime FirstDate = Info.GetFirstDate(PendingDateTime);
	
	// Fill the contents of the calendar.
	for (int32 Row = 0; Row < Info.RowNum; Row++)
	{
		for (int32 Column = 0; Column < Info.ColumnNum; Column++)
		{
			const int32 Index = (Row * Info.ColumnNum) + Column;
			if (Info.MaxIndex != INDEX_NONE && Index > Info.MaxIndex)
			{
				break;
			}
			
			const FDateTime DateTime = FirstDate.GetTicks() + (Info.Timespan * Index);
			CalendarPanel->AddSlot(Column, Row)
			[
				SNew(DateTimePickerInternal::SDateTimeGrid)
				.DateTime(DateTime)
				.ShouldBeGrayOut(Info.ShouldBeGrayOut(PendingDateTime, DateTime))
				.Mode(Mode)
				.OnDateTimePicked(this, &SDateTimePicker::HandleOnDateTimePicked)
			];
		}
	}
}

FText SDateTimePicker::GetTitleText() const
{
	return FText::AsDateTime(
		PendingDateTime,
		EDateTimeStyle::Medium,
		EDateTimeStyle::Short,
		FText::GetInvariantTimeZone()
	);
}

void SDateTimePicker::HandleOnDateTimePicked(const FDateTime& PickedDateTime)
{
	if (OnDateTimePicked.IsBound())
	{
		OnDateTimePicked.Execute(PickedDateTime);
	}
}
