// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chess_GameMode.h"
#include "CoreMinimal.h"
#include "Tile.h"
#include "BasePawn.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"


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

	static const int32 NOT_ASSIGNED = -1;

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


	UPROPERTY()
	UUserWidget* TileIdWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> TileIdWidgetRef;

	// Tile Classes
	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialLight;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialLightRed;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialDark;

	UPROPERTY(EditAnywhere, Category = "Materials")
	UMaterialInterface* MaterialDarkRed;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> B_TileClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> W_TileClass;

	// Pawns Classes
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

	void LoadBoard(const TArray<FTileSaving>& Board, bool IsPlayable);

	
//protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

// public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
