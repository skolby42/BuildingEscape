// Copyright Scott Kolby 2020

#include "OpenDoor.h"
#include "Components/AudioComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"


// Sets default values for this component's properties
UOpenDoor::UOpenDoor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UOpenDoor::BeginPlay()
{
	Super::BeginPlay();

	InitialYaw = GetOwner()->GetActorRotation().Yaw;
	CurrentYaw = InitialYaw;
	OpenYaw += InitialYaw;

	FindAudioComponent();
	FindPressurePlate();
}

// Called every frame
void UOpenDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (GetTotalMassOfActors() >= DoorOpeningMass)
	{
		OpenDoor(DeltaTime);
		DoorLastOpened = CurrentTime;
	}
	else
	{
		if (CurrentTime - DoorLastOpened > DoorCloseDelay)
			CloseDoor(DeltaTime);
	}
}

void UOpenDoor::OpenDoor(float DeltaTime)
{
	CurrentYaw = FMath::FInterpTo(CurrentYaw, OpenYaw, DeltaTime, DoorOpenSpeed);
	FRotator DoorRotation = GetOwner()->GetActorRotation();
	DoorRotation.Yaw = CurrentYaw;
	GetOwner()->SetActorRotation(DoorRotation);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentYaw);

	CloseDoorSoundPlayed = false;
	if (AudioComponent && !OpenDoorSoundPlayed)
	{
		AudioComponent->Play();
		OpenDoorSoundPlayed = true;
	}
}

void UOpenDoor::CloseDoor(float DeltaTime)
{
	CurrentYaw = FMath::FInterpTo(CurrentYaw, InitialYaw, DeltaTime, DoorCloseSpeed);
	FRotator DoorRotation = GetOwner()->GetActorRotation();
	DoorRotation.Yaw = CurrentYaw;
	GetOwner()->SetActorRotation(DoorRotation);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentYaw);

	OpenDoorSoundPlayed = false;
	if (AudioComponent && !CloseDoorSoundPlayed)
	{
		AudioComponent->Play();
		CloseDoorSoundPlayed = true;
	}
}

float UOpenDoor::GetTotalMassOfActors() const
{
	float TotalMass = 0.f;

	if (!PressurePlate)
		return TotalMass;

	TArray<AActor*> OverlappingActors;
	PressurePlate->GetOverlappingActors(OUT OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == nullptr)
			continue;

		UPrimitiveComponent* PrimitiveComponent = Actor->FindComponentByClass<UPrimitiveComponent>();
		TotalMass += PrimitiveComponent->GetMass();

		//UE_LOG(LogTemp, Warning, TEXT("%s %f"), *Actor->GetName(), PrimitiveComponent->GetMass());
	}

	return TotalMass;
}

void UOpenDoor::FindAudioComponent()
{
	AudioComponent = GetOwner()->FindComponentByClass<UAudioComponent>();
	if (!AudioComponent)
		UE_LOG(LogTemp, Error, TEXT("Audio component not found on %s"), *GetOwner()->GetName())
}

void UOpenDoor::FindPressurePlate()
{
	if (!PressurePlate)
		UE_LOG(LogTemp, Error, TEXT("%s actor does not have a pressure plate set!"), *GetOwner()->GetName());
}
