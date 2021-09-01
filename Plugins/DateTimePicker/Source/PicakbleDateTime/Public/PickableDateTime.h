// Copyright 2021 Naotsun. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PickableDateTime.generated.h"

/**
 * An extended structure of FDateTime that can display the Date Time Picker and set the date and time.
 */
USTRUCT(BlueprintType)
struct FPickableDateTime
{
	GENERATED_BODY()

public:
	// Actual date and time data.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickable Date Time")
	FDateTime DateTime;

public:
	// Constructor.
	FPickableDateTime() = default;
	FPickableDateTime(const FDateTime& InDateTime) : DateTime(InDateTime) {}
	virtual ~FPickableDateTime() = default;

	// FDateTime operator overloading wrapper functions.
	// See the header of FDateTime for details.
	FPickableDateTime operator+(const FTimespan& Other) const
	{
		return FDateTime(DateTime.GetTicks() + Other.GetTicks());
	}
	
	FPickableDateTime& operator+=(const FTimespan& Other)
	{
		DateTime = DateTime.GetTicks() + Other.GetTicks();

		return *this;
	}
	
	FPickableDateTime& operator+(const FPickableDateTime& Other)
	{
		DateTime = DateTime.GetTicks() + Other.DateTime.GetTicks();

		return *this;
	}
	
	FTimespan operator-(const FPickableDateTime& Other) const
	{
		return FTimespan(DateTime.GetTicks() - Other.DateTime.GetTicks());
	}
	
	FPickableDateTime operator-(const FTimespan& Other) const
	{
		return FDateTime(DateTime.GetTicks() - Other.GetTicks());
	}
	
	FPickableDateTime& operator-=(const FTimespan& Other)
	{
		DateTime = DateTime.GetTicks() - Other.GetTicks();

		return *this;
	}
	
	bool operator==(const FPickableDateTime& Other) const
	{
		return (DateTime.GetTicks() == Other.DateTime.GetTicks());
	}

	bool operator!=(const FPickableDateTime& Other) const
	{
		return (DateTime.GetTicks() != Other.DateTime.GetTicks());
	}

	bool operator>(const FPickableDateTime& Other) const
	{
		return (DateTime.GetTicks() > Other.DateTime.GetTicks());
	}

	bool operator>=(const FPickableDateTime& Other) const
	{
		return (DateTime.GetTicks() >= Other.DateTime.GetTicks());
	}

	bool operator<(const FPickableDateTime& Other) const
	{
		return (DateTime.GetTicks() < Other.DateTime.GetTicks());
	}

	bool operator<=(const FPickableDateTime& Other) const
	{
		return (DateTime.GetTicks() <= Other.DateTime.GetTicks());
	}
};

// Define a GetTypeHash function so that it can be used as a map key.
FORCEINLINE uint32 GetTypeHash(const FPickableDateTime& PickableDateTime)
{
	return GetTypeHash(PickableDateTime.DateTime);
}
