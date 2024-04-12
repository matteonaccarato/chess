// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chess_GameMode.h"
#include "CoreMinimal.h"
#include "ChessEnums.h"
#include "Tile.h"
#include "BasePawn.h"
#include "Players/Chess_PlayerInterface.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

struct FPieceSaving;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);

UCLASS()
class CHESS_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:	


	// TILES
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;


	// PAWNS
	UPROPERTY(Transient)
	TArray<ABasePawn*> PawnArray;
	
	UPROPERTY(Transient)
	TMap<FVector2D, ABasePawn*> PawnMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float NormalizedCellPadding;

	// BlueprintAssignable usable with multicast delegates only.
	// Exposes the property for assigning in Blueprints.
	// Declare a variable of type FOnReset (delegate)
	UPROPERTY(BlueprintAssignable)
	FOnReset OnResetEvent;

	// Size of field
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Size;

	// Number of rows where pawns have to be placed
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Pawns_Rows;

	// Size of winning line
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WinSize;

	// Tile Materials
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialLight;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialLightRed;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialDark;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialDarkRed;

	// TODO => fare unica tile (Set material in seguito)
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> B_TileClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> W_TileClass;

	// Tile Letters/Numbers Materials
	UPROPERTY(EditAnywhere, Category = "Materials")
	TArray<UMaterialInterface*> Letters;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TArray<UMaterialInterface*> Numbers;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterA;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterB;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterC;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterD;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterE;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterF;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterG;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* LetterH;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number1;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number2;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number3;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number4;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number5;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number6;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number7;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* Number8;

	// Pawns Classes
	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ABasePawn>> ChessPieces;

	// Blacks
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> B_BishopClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> B_KingClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> B_KnightClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> B_PawnClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> B_QueenClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> B_RookClass;

	// Whites
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> W_BishopClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> W_KingClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> W_KnightClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> W_PawnClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> W_QueenClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePawn> W_RookClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> LetterNumberClass;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CellPadding;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;

	// Sets default values for this actor's properties
	AGameField();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void ResetField();
	void GenerateField();

	FVector2D GetPosition(const FHitResult& Hit);

	TArray<ATile*>& GetTileArray();
	TArray<ABasePawn*>& GetPawnArray();

	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;
	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	/*
	*/
	bool IsValidTile(const int8 X, const int8 Y) const;

	int8 DistancePieces(const ABasePawn* Piece1, const ABasePawn* Piece2) const;
	
	/*
	*/
	std::pair<int8, int8> GetXYOffset(const int8 Steps, const ECardinalDirection Direction, const EPawnColor PieceColor) const;

	/*
	*/
	bool CheckDirection(const EDirection Direction, ABasePawn* Pawn, const FVector2D NewGridPosition, const FVector2D CurrGridPosition, const bool TestFlag = false) const;




	/*
	*/
	bool IsLineClear(const ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const;





	void LoadBoard(const TArray<FPieceSaving>& Board);


	/*
	*/
	ABasePawn* SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y, int8 PlayerOwner = ChessEnums::NOT_ASSIGNED);

	/*
	*/
	void DespawnPawn(int8 X, int8 Y, bool Simulate = false);

	
//protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

// public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
