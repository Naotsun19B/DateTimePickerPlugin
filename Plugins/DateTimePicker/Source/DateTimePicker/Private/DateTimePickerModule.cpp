// Copyright 2021 Naotsun. All Rights Reserved.

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogDateTimePicker);

class FDateTimePickerModule : public IModuleInterface
{
public:
	// IModuleInterface interface.
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End of IModuleInterface interface.
};

void FDateTimePickerModule::StartupModule()
{
	
}

void FDateTimePickerModule::ShutdownModule()
{
	
}

IMPLEMENT_MODULE(FDateTimePickerModule, DateTimePicker)
