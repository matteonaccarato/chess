// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tile.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReset);

UCLASS()
class CHESS_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:	
	
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

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

	// Size of winning line
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WinSize;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass;

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

	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;

	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	/*
	isWinPosition
	isWinLine
	isValidPosition
	*/

	TArray<int32> GetLine(const FVector2D Begin, const FVector2D End) const;

	// allEqual
	// 
//protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

// public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
