// Fill out your copyright notice in the Description page of Project Settings.

#include "Portal.h"

#include "FMODAudioComponent.h"
#include "FMODBlueprintStatics.h"
#include "Components/BoxComponent.h"
#include "Portal/PortalWaypoint.h"
#include "HealthSystem/HealthComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "NiagaraComponent.h"


// Sets default values
APortal::APortal()
{
  // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  MeshRootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshRootComponent"));
  BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
  BoxComponent->SetupAttachment(RootComponent);
  DestroyEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DestroyEffectComponent"));
  FloatingMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Floating Movement Component"));
}

// Called when the game starts or when spawned
void APortal::BeginPlay()
{
  Super::BeginPlay();

  AIController = Cast<AAIController>(GetController());
  DestroyEffectComponent->DeactivateImmediate();
  PortalAudioEventInstance = UFMODBlueprintStatics::PlayEventAttached(PortalAudioEvent, MeshRootComponent, NAME_None, GetActorLocation(), EAttachLocation::SnapToTarget, true, true, true);

  PortalDestroy.AddDynamic(this, &APortal::StartDestroyAudio);
}

// Called every frame
void APortal::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}

void APortal::StopAudio()
{
  if (IsValid(PortalAudioEventInstance))
  {
    PortalAudioEventInstance->StudioInstance->stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
  }
}

void APortal::StartDestroyAudio(bool f, int a)
{
  DestroyPortalAudioEventInstance = UFMODBlueprintStatics::PlayEventAttached(DestroyPortalAudioEvent, MeshRootComponent, NAME_None, GetActorLocation(), EAttachLocation::SnapToTarget, true, true, true);
}

bool APortal::DestroyPortal()
{
  bool bDestroy = false;
  if (IsMovablePortal)
  {
    if (Hits < TotalHits)
    {
      if (Waypoints.Num() > 0)
      {
        APortalWaypoint* Point = Waypoints[Hits];
        if (IsValid(Point) && AIController)
        {
          AIController->MoveToActor(Point);
        }
      }
      PortalDestroy.Broadcast(false, Index);
      StopAudio();
    }
    else
    {
      PortalDestroy.Broadcast(true, Index);
      StopAudio();
      bDestroy = true;
    }

    Hits++;
    Index++;
  }
  else
  {
    BoxComponent->Deactivate();
    PortalDestroy.Broadcast(true, Index);
    StopAudio();
    bDestroy = true;
  }

  return bDestroy;
}

void APortal::BeginDestroy()
{
  StopAudio();
  Super::BeginDestroy();
}
