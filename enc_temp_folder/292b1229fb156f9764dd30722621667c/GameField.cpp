// Fill out your copyright notice in the Description page of Project Settings.


#include "GameField.h"
#include "Kismet/GameplayStatics.h"
#include <string>

// Sets default values
AGameField::AGameField()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	WinSize = 3;

	// Size of the field (8x8)
	Size = 8; 
	TileSize = 120;
	CellPadding = 0;
	Pawns_Rows = 2;
	// NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

void AGameField::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	NormalizedCellPadding = FMath::RoundToDouble(((TileSize + CellPadding) / TileSize) * 100) / 100;
}

// Called when the game starts or when spawned
void AGameField::BeginPlay()
{
	Super::BeginPlay();
	GenerateField();
}

void AGameField::ResetField()
{
	for (ATile* Obj : TileArray)
	{
		Obj->SetTileStatus(NOT_ASSIGNED, ETileStatus::EMPTY);
	}

	OnResetEvent.Broadcast();

	// gamemode
}

void AGameField::GenerateField()
{
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;
	TSubclassOf<ABasePawn> BasePawnClass;



	bool flag = false;
	for (int32 x = 0; x < Size; x++)
	{
		for (int32 y = 0; y < Size; y++)
		{
			/* if (flag)
			{
				TileClass = W_TileClass;
			}
			else
			{
				TileClass = B_TileClass;
			} */

			TileClass = flag ? W_TileClass : B_TileClass;

			FVector Location = AGameField::GetRelativeLocationByXYPosition(x, y);
			ATile* TileObj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
						
			if (TileObj != nullptr)
			{
				// [A-H][1-8]
				const int32 IdChar = 65 + y;
				const int32 IdNum = x + 1;

				
				TileObj->SetId(FString::Printf(TEXT("%c%d"), IdChar, IdNum));
				const float TileScale = TileSize / 100;
				TileObj->SetActorScale3D(FVector(TileScale, TileScale, 0.2));
				TileObj->SetGridPosition(x, y);
				TileArray.Add(TileObj);
				TileMap.Add(FVector2D(x, y), TileObj);

				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TileObj->GetId());

				if (x < Pawns_Rows || (Size - x - 1) < Pawns_Rows)
				{
					// std::vector<std::string> arr = { "W_RookClass", "ca" };
					// std::vector<ABasePawn> arr;
					
					if (x == 0)
					{
						switch (y) {
						case 0: BasePawnClass = W_RookClass; break;
						case 1: BasePawnClass = W_KnightClass; break;
						case 2: BasePawnClass = W_BishopClass; break;
						case 3: BasePawnClass = W_QueenClass; break;
						case 4: BasePawnClass = W_KingClass; break;
						case 5: BasePawnClass = W_BishopClass; break;
						case 6: BasePawnClass = W_KnightClass; break;
						case 7: BasePawnClass = W_RookClass; break;
						}
					} else if (x == 1)  BasePawnClass = W_PawnClass;
					else if (x == Size - 1)
					{
						switch (y) {
						case 0: BasePawnClass = B_RookClass; break;
						case 1: BasePawnClass = B_KnightClass; break;
						case 2: BasePawnClass = B_BishopClass; break;
						case 3: BasePawnClass = B_QueenClass; break;
						case 4: BasePawnClass = B_KingClass; break;
						case 5: BasePawnClass = B_BishopClass; break;
						case 6: BasePawnClass = B_KnightClass; break;
						case 7: BasePawnClass = B_RookClass; break;
						}
					}
					else BasePawnClass = B_PawnClass;

					
					
					FVector Origin;
					FVector BoxExtent;
					TileObj->GetActorBounds(false, Origin, BoxExtent);


					FVector PawnLocation(Location.GetComponentForAxis(EAxis::X), Location.GetComponentForAxis(EAxis::Y), Location.GetComponentForAxis(EAxis::Z) + 2*BoxExtent.GetComponentForAxis(EAxis::Z));
					ABasePawn* BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(BasePawnClass, PawnLocation, FRotator(0,90,0));
					// BasePawnObj->SetTileId(FString::Printf(TEXT("%c%d"), IdChar, IdNum));
					BasePawnObj->SetGridPosition(x, y);
					if (BasePawnObj != nullptr)
					{
						// 0.8 da mettere come attributo
						BasePawnObj->SetActorScale3D(FVector(TileScale * 0.8, TileScale * 0.8, 0.05));
						// BasePawnObj->SetGridPosition(x, y);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("ABasePawn Obj is null"));
					}
				}
				
				
				
				




				flag = !flag;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("ATile Obj is null"));
			}
		}
		flag = !flag;

	}

	
	
	

}

FVector2D AGameField::GetPosition(const FHitResult& Hit)
{
	return Cast<ATile>(Hit.GetActor())->GetGridPosition();
}

TArray<ATile*>& AGameField::GetTileArray()
{
	return TileArray;
}

FVector AGameField::GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const
{
	return TileSize * NormalizedCellPadding * FVector(InX, InY, 0);
}

FVector2D AGameField::GetXYPositionByRelativeLocation(const FVector& Location) const
{
	const double x = Location[0] / (TileSize * NormalizedCellPadding);
	const double y = Location[1] / (TileSize * NormalizedCellPadding);

	return FVector2D(x, y);
}

/* TArray<int32> AGameField::GetLine(const FVector2D Begin, const FVector2D End) const
{
	return TArray<int32>();
} */

// Called every frame
/* void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */

