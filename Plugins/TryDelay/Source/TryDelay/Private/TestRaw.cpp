#include "TestRaw.h"
#include "TryDelayBPLibrary.h"

FRawTest::FRawTest()
{
	//UTryDelayBPLibrary::DelayRawFunction(this, -1, 5.f, true, &FRawTest::TestPrint, GWorld->GetWorld(), 55.f);
}

void FRawTest::TestPrint(UWorld* World, float InValue)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), World->GetRealTimeSeconds()));
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Black, FString::Printf(TEXT("%f"), InValue));
}

