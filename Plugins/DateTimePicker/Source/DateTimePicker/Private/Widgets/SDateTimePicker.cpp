// Copyright 2021 Naotsun. All Rights Reserved.

#include "Widgets/SDateTimePicker.h"
#include "Components/VerticalBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"

namespace DateTimePickerInternal
{
	// Calendar date grid button widget.
	class SDateTimeGrid : public SButton
	{
	public:
		SLATE_BEGIN_ARGS(SDateTimeGrid) {}
			SLATE_ARGUMENT(FDateTime, DateTime)
			SLATE_ARGUMENT(FDateTime, PendingDateTime)
			SLATE_ARGUMENT(bool, ShouldBeGrayOut)
			SLATE_ARGUMENT(SDateTimePicker::EDateTimePickerMode, Mode)
			SLATE_EVENT(SDateTimePicker::FOnDateTimePicked, OnDateTimePicked)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
		{
			DateTime = InArgs._DateTime;
			PendingDateTime = InArgs._PendingDateTime;
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

			const FLinearColor GridColor = InArgs._ShouldBeGrayOut ? FLinearColor(FVector(0.3f)) : (FDateTime::Now().GetDate() == DateTime.GetDate()) ? FLinearColor(FColor::Green) : PendingDateTime.GetDate() == DateTime.GetDate() ? FLinearColor(FColor::Orange) : FLinearColor(FVector(0.8f));

			SButton::Construct(
				SButton::FArguments()
				.ButtonColorAndOpacity(GridColor)
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
		FDateTime PendingDateTime;
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
				static_cast<int32>(EDayOfWeek::Sunday), 7,
				ETimespan::TicksPerDay,
				INDEX_NONE,
				[](const FDateTime& PendingDateTime) -> FDateTime
				{
					const FDateTime FirstDate(PendingDateTime.GetYear() , PendingDateTime.GetMonth(), 1);
					const EDayOfWeek FirstDayOfWeek = FirstDate.GetDayOfWeek();
					return FirstDate.GetTicks() - (ETimespan::TicksPerDay * static_cast<int32>(FirstDayOfWeek));//DateTimePickerInternal::GetDaysThatPassedSinceSunday(FirstDayOfWeek));
				},
				[](const FDateTime& PendingDateTime, const FDateTime& DateTime) -> bool
				{
					return (PendingDateTime.GetMonth() != DateTime.GetMonth());
				}
			)
		},
				{
			SDateTimePicker::EDateTimePickerMode::Year,
			FGenerateCalenderGrids(
				// 6 * 5 grid to show years, to be ensure that the current year is in the center PendingDateTime.GetYear() - 12
			6, 5,
				ETimespan::TicksPerYear,
				INDEX_NONE,
				[](const FDateTime& PendingDateTime) -> FDateTime
				{
					const FDateTime FirstDate(PendingDateTime.GetYear() - 12, PendingDateTime.GetMonth(), PendingDateTime.GetDay());
					return FirstDate;
				},
				[](const FDateTime& PendingDateTime, const FDateTime& DateTime) -> bool
				{
					return (PendingDateTime.GetYear() != DateTime.GetYear());
				}
			)
		}
	};
}

int32 SDateTimePicker::GetNormalizedDay(int32 inMonth)
{
	const int32 DaysInMonth = FDateTime::DaysInMonth(PendingDateTime.GetYear(), inMonth);
	return PendingDateTime.GetDay() > DaysInMonth ? DaysInMonth : PendingDateTime.GetDay();
}

void SDateTimePicker::OnPressedPreviousMonth()
{
	const int32 NormalizedMonth = PendingDateTime.GetMonth() - 1;

	if (NormalizedMonth <= 0)
		PendingDateTime = FDateTime(PendingDateTime.GetYear() - 1, 12, GetNormalizedDay(12));
	else
		PendingDateTime = FDateTime(PendingDateTime.GetYear(), NormalizedMonth, GetNormalizedDay(NormalizedMonth));

	RebuildCalenderPanel();
}

void SDateTimePicker::OnPressedNextMonth()
{
	int32 NormalizedMonth = PendingDateTime.GetMonth() % 12 + 1;

	if (NormalizedMonth == 1)
		PendingDateTime = FDateTime(PendingDateTime.GetYear() + 1, NormalizedMonth, GetNormalizedDay(NormalizedMonth));
	else
		PendingDateTime = FDateTime(PendingDateTime.GetYear(), NormalizedMonth, GetNormalizedDay(NormalizedMonth));

	RebuildCalenderPanel();
}

void SDateTimePicker::OnPressedNow()
{
	PendingDateTime = FDateTime::Now();
	RebuildCalenderPanel();
}

void SDateTimePicker::OnYearChanged()
{
	Mode = (Mode == EDateTimePickerMode::Year) ? EDateTimePickerMode::Day : EDateTimePickerMode::Year;
	RebuildCalenderPanel();
}

void SDateTimePicker::OnPressedOkay()
{
	if (OnDateTimePicked.IsBound())
	{
		OnDateTimePicked.Execute(PendingDateTime);
	}
}

void SDateTimePicker::OnPressedCancel()
{
	if (OnDateTimePicked.IsBound())
	{
		OnDateTimePicked.Execute(InitialDateTimeSelected);
	}
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

	InitialDateTimeSelected = PendingDateTime;

	OnDateTimePicked = InArgs._OnDateTimePicked;

	const FSlateFontInfo FontInfo(FCoreStyle::GetDefaultFontStyle("Regular", 12));

	ChildSlot
		[
			SNew(SBox)
				.Padding(5.f)
				.WidthOverride(330)
				.HeightOverride(215)
				.MinDesiredWidth(330)
				.MinDesiredHeight(215)
				[
					// Show year and month text and button to switch between months.
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 0, 0, 2)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Left)
										.AutoWidth()
										[
											SNew(SButton)
												.Text(FText::FromString("<"))
												.OnPressed(this, &SDateTimePicker::OnPressedPreviousMonth)
										]

										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Fill)
										.FillWidth(1)
										[
											SNew(STextBlock)
												.Text(this, &SDateTimePicker::GetTitleText)
												.Font(FontInfo)
												.Justification(ETextJustify::Center)
										]

										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Right)
										.AutoWidth()
										[
											SNew(SButton)
												.Text(FText::FromString(">"))
												.OnPressed(this, &SDateTimePicker::OnPressedNextMonth)
										]

										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Right)
										.AutoWidth()
										[
											SNew(SButton)
												.Text(FText::FromString(FString::FromInt(PendingDateTime.GetYear())))
												.OnPressed(this, &SDateTimePicker::OnYearChanged)
										]

										+ SHorizontalBox::Slot()
										.HAlign(HAlign_Right)
										.AutoWidth()
										[
											SNew(SButton)
												.Text(FText::FromString("Today"))
												.OnPressed(this, &SDateTimePicker::OnPressedNow)
										]

								]
						]

						// A grid panel that displays the date and time of the calendar.
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Fill)
						.Padding(0, 3, 0, 0)
						.AutoHeight()
						[
							SAssignNew(CalendarPanel, SUniformGridPanel)
						]

						// Okay and Cancel Button to confirm the Date
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 3, 0, 0)
						[
							SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Right)
								.FillWidth(1)
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("Cancel")))
										.OnPressed(this, &SDateTimePicker::OnPressedCancel)
								]
								+ SHorizontalBox::Slot()
								.HAlign(HAlign_Right)
								.AutoWidth()
								[
									SNew(SButton)
										.Text(FText::FromString(TEXT("OK")))
										.OnPressed(this, &SDateTimePicker::OnPressedOkay)
								]
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

	//Add day name
	if (Mode == EDateTimePickerMode::Day)
	{
		for (int32 Day = 0; Day < Info.ColumnNum; Day++)
		{
			FString DayOfWeek = ShortDayNames[Day];
			CalendarPanel->AddSlot(Day, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(DayOfWeek))
						.Justification(ETextJustify::Center)
				];
		}
	}

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

			const FDateTime DateTime = (Mode == EDateTimePickerMode::Day) ? FirstDate.GetTicks() + (Info.Timespan * Index) : FirstDate + FTimespan::FromDays(365.25 * Index);
			CalendarPanel->AddSlot(Column, Row + 1)
				[
					SNew(DateTimePickerInternal::SDateTimeGrid)
						.DateTime(DateTime)
						.ShouldBeGrayOut(Info.ShouldBeGrayOut(PendingDateTime, DateTime))
						.Mode(Mode)
						.PendingDateTime(PendingDateTime)
						.OnDateTimePicked(this, &SDateTimePicker::HandleOnDateTimePicked)
				];
		}
	}
}

FText SDateTimePicker::GetTitleText() const
{
	return FText::AsDate(
		PendingDateTime,
		EDateTimeStyle::Medium,
		FText::GetInvariantTimeZone()
	);
}

void SDateTimePicker::HandleOnDateTimePicked(const FDateTime& PickedDateTime)
{
	if (OnDateTimePicked.IsBound())
	{
		PendingDateTime = PickedDateTime;

		if (Mode == EDateTimePickerMode::Year)
			Mode = EDateTimePickerMode::Day;

		RebuildCalenderPanel();
	}
}
