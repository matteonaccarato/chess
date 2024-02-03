// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawn.h"
// #include "GameField.h"
// #include "Chess_GameMode.h"

// Sets default values
ABasePawn::ABasePawn()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// TODO: forse da settare su true per movimento dinamico (come cono)

}

/* EPawnMovement ABasePawn::GetMovement() const
{
	return Movement;
} */

void ABasePawn::SetMaxNumberSteps(int32 NumberSteps)
{
	MaxNumberSteps = NumberSteps;
}

int32 ABasePawn::GetMaxNumberSteps() const
{
	return MaxNumberSteps;
}

void ABasePawn::SetColor(EPawnColor PawnColor)
{
	Color = PawnColor;
}


EPawnColor ABasePawn::GetColor() const
{
	return Color;
}

void ABasePawn::SetType(EPawnType PawnType)
{
	Type = PawnType;
}

EPawnType ABasePawn::GetType() const
{
	return Type;
}

void ABasePawn::SetStatus(EPawnStatus PawnStatus)
{
	Status = PawnStatus;
}

EPawnStatus ABasePawn::GetStatus() const
{
	return Status;
}

void ABasePawn::SetGridPosition(const double InX, const double InY)
{
	TileGridPosition.Set(InX, InY);
}

FVector2D ABasePawn::GetGridPosition() const
{
	return TileGridPosition;
}

/* void ABasePawn::SetTileId(const FString TileIdParam)
{
	TileId = TileIdParam;
}

FString ABasePawn::GetTileId() const
{
	return TileId;
} */

// Called when the game starts or when spawned
void ABasePawn::BeginPlay()
{
	Super::BeginPlay();


	/*AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	if (GameMode != nullptr)
	{
		// TODO => riposizionare pedina nel punto di partenza
		GameMode->GField->OnResetEvent.AddDynamic(this, &ABasePawn::SelfDestroy);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("GameMode is null"));
	}*/
	
}

// Called every frame
/* void ABasePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
} */

void ABasePawn::SelfDestroy()
{
	Destroy();
}

