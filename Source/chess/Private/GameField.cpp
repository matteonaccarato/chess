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
		Obj->SetPlayerOwner(NOT_ASSIGNED);
		TArray<bool> TmpFalse;
		TmpFalse.Add(false);
		TmpFalse.Add(false);
		Obj->SetTileStatus({ 1, TmpFalse, TArray<ABasePawn*>(), EPawnColor::NONE, EPawnType::NONE});
	}

	OnResetEvent.Broadcast();

	AChess_GameMode* GameMode = Cast<AChess_GameMode>(GetWorld()->GetAuthGameMode());
	GameMode->IsGameOver = false;
	GameMode->MoveCounter = 0;
	GameMode->ChoosePlayerAndStartGame();
}

void AGameField::GenerateField()
{
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;
	TSubclassOf<ABasePawn> BasePawnClass;


	UWorld* World = GetWorld();
	bool flag = false;
	int8 PieceIdx = 0;
	for (int32 x = 0; x < Size; x++)
	{
		for (int32 y = 0; y < Size; y++)
		{
			TileClass = flag ? W_TileClass : B_TileClass;

			FVector Location = AGameField::GetRelativeLocationByXYPosition(x, y);
			ATile* TileObj = GetWorld()->SpawnActor<ATile>(TileClass, Location, FRotator::ZeroRotator);
						
			if (TileObj != nullptr)
			{
				// [A-H][1-8]
				const int32 IdChar = 65 + y;
				const int32 IdNum = x + 1;

				TileObj->SetLetterId(FString::Printf(TEXT("%c"), IdChar));
				TileObj->SetNumberId(IdNum);

				// Bottom (Letters)
				if (x == 0 && World)
				{
					UUserWidget* WidgetTxt = CreateWidget(World, TileIdWidgetRef);
					if (WidgetTxt)
					{
						WidgetTxt->AddToViewport(1);

						UTextBlock* BtnText = Cast<UTextBlock>(WidgetTxt->GetWidgetFromName(TEXT("txtId")));
						if (BtnText)
						{
							BtnText->SetText(FText::FromString(FString::Printf(TEXT("%c"), IdChar)));
						}

						FVector2D WidgetPosition = FVector2D(TileObj->GetActorLocation()[0], TileObj->GetActorLocation()[1]); 
						WidgetTxt->SetPositionInViewport(WidgetPosition);
					}
				}
				
				// Left (Numbers)
				if (y == 0 && World)
				{
					UUserWidget* WidgetTxt = CreateWidget(World, TileIdWidgetRef);
					if (WidgetTxt)
					{
						WidgetTxt->AddToViewport(1);

						UTextBlock* BtnText = Cast<UTextBlock>(WidgetTxt->GetWidgetFromName(TEXT("txtId")));
						if (BtnText)
						{
							BtnText->SetText(FText::FromString(FString::Printf(TEXT("%d"), IdNum)));
						}

						FVector2D WidgetPosition = FVector2D(TileObj->GetActorLocation()[0], TileObj->GetActorLocation()[1]);
						WidgetTxt->SetPositionInViewport(WidgetPosition);
					}
				}


				const float TileScale = TileSize / 100;
				TileObj->SetActorScale3D(FVector(TileScale, TileScale, 0.2));
				TileObj->SetGridPosition(x, y);
				TileArray.Add(TileObj);
				TileMap.Add(FVector2D(x, y), TileObj);

				TArray<bool> TmpFalse;
				TmpFalse.Add(false);
				TmpFalse.Add(false);
				FTileStatus TileStatus = { 1, TmpFalse, TArray<ABasePawn*>(), EPawnColor::NONE, EPawnType::NONE};
				int32 PlayerOwner = -1;

				if (x < Pawns_Rows || (Size - x - 1) < Pawns_Rows)
				{
					TileStatus.EmptyFlag = 0;
					
					EPawnType PawnTypes[] = { EPawnType::ROOK, EPawnType::KNIGHT, EPawnType::BISHOP, EPawnType::QUEEN, EPawnType::KING, EPawnType::BISHOP, EPawnType::KNIGHT, EPawnType::ROOK };
					
					if (x == 0)
					{
						TSubclassOf<ABasePawn> W_PawnsClasses[] = { W_RookClass, W_KnightClass, W_BishopClass, W_QueenClass, W_KingClass, W_BishopClass, W_KnightClass, W_RookClass };

						BasePawnClass = W_PawnsClasses[y];
						TileStatus.PawnColor = EPawnColor::WHITE;
						TileStatus.PawnType = PawnTypes[y];

						PlayerOwner = 0;
					}
					else if (x == 1)
					{
						BasePawnClass = W_PawnClass;
						
						TileStatus.PawnColor = EPawnColor::WHITE;
						TileStatus.PawnType = EPawnType::PAWN;
						PlayerOwner = 0;
					}
					else if (x == Size - 1)
					{
						TSubclassOf<ABasePawn> B_PawnsClasses[] = { B_RookClass, B_KnightClass, B_BishopClass, B_QueenClass, B_KingClass, B_BishopClass, B_KnightClass, B_RookClass };
						
						BasePawnClass = B_PawnsClasses[y];
						TileStatus.PawnColor = EPawnColor::BLACK;
						TileStatus.PawnType = PawnTypes[y];
						PlayerOwner = 1;
					}
					else
					{
						BasePawnClass = B_PawnClass;

						TileStatus.PawnColor = EPawnColor::BLACK;
						TileStatus.PawnType = EPawnType::PAWN;
						PlayerOwner = 1;
					}

					FVector Origin;
					FVector BoxExtent;
					TileObj->GetActorBounds(false, Origin, BoxExtent);

					FVector PawnLocation(Location.GetComponentForAxis(EAxis::X), Location.GetComponentForAxis(EAxis::Y), Location.GetComponentForAxis(EAxis::Z) + 2*BoxExtent.GetComponentForAxis(EAxis::Z) + 0.1);
					ABasePawn* BasePawnObj = GetWorld()->SpawnActor<ABasePawn>(BasePawnClass, PawnLocation, FRotator(0,90,0));
					if (BasePawnObj != nullptr)
					{
						BasePawnObj->SetGridPosition(x, y);
						// TODO: 0.8 da mettere come attributo
						BasePawnObj->SetActorScale3D(FVector(TileScale * 0.8, TileScale * 0.8, 0.03));
						
						BasePawnObj->SetPieceNum(PieceIdx);
						BasePawnObj->SetType(TileStatus.PawnType);
						BasePawnObj->SetColor(TileStatus.PawnColor);
						BasePawnObj->SetStatus(EPawnStatus::ALIVE);

						PawnArray.Add(BasePawnObj);
						PawnMap.Add(FVector2D(x, y), BasePawnObj);
						PieceIdx++;
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("ABasePawn Obj is null"));
					}

					TileObj->SetPawn(BasePawnObj);
				}

				TileObj->SetPlayerOwner(PlayerOwner);
				TileObj->SetTileStatus(TileStatus);
				
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

TArray<ABasePawn*>& AGameField::GetPawnArray()
{
	return PawnArray;
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

void AGameField::LoadBoard(const TArray<FTileSaving>& Board, bool IsPlayable)
{
	// TODO => trovare modo più efficiente


	// memorizzo in ordine come pawnarray [0..32] e ottengo tile X Y | for()
	// in load board
	//		for (pawnarray)	pawn->setactorlocation()

	for (int8 i = 0; i < PawnArray.Num(); i++)
	{
		switch (PawnArray[i]->GetStatus())
		{
		case EPawnStatus::ALIVE:
			PawnArray[i]->SetActorHiddenInGame(false);
			PawnArray[i]->SetActorEnableCollision(true);
			PawnArray[i]->SetActorTickEnabled(true);
			break;
		case EPawnStatus::DEAD:
			PawnArray[i]->SetActorHiddenInGame(true);
			PawnArray[i]->SetActorEnableCollision(false);
			PawnArray[i]->SetActorTickEnabled(false);
			break;
		}
		
		PawnArray[i]->SetActorLocation(GetRelativeLocationByXYPosition(Board[i].X, Board[i].Y) + FVector(0, 0, 20));
	}
}


// Called every frame
/* void AGameField::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

} */