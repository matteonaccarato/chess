// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tile.generated.h"

UENUM()
enum class ETileStatus : uint8
{
	EMPTY UMETA(DisplayName = "Empty"), // 0
	OCCUPIED UMETA(DisplayName = "Occupied"), // 1
	B_BISHOP, // 2
	B_KING, // 3
	B_KNIGHT, // 4
	B_PAWN, // 5
	B_QUEEN, // 6
	B_ROOK, // 7
	W_BISHOP, // 8
	W_KING, // 9
	W_KNIGHT, // 10
	W_PAWN, // 11
	W_QUEEN, // 12
	W_ROOK, // 13
};

UCLASS()
class CHESS_API ATile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATile();

	void SetId(const FString TileId);
	FString GetId() const;

	void SetTileStatus(const int32 TileOwner, const ETileStatus TileStatus);
	ETileStatus GetTileStatus() const;

	int32 GetOwner() const;

	void SetGridPosition(const double InX, const double InY);
	FVector2D GetGridPosition() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Id;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETileStatus Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PlayerOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

// public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

};
