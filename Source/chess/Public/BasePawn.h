// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chess_GameMode.h"
#include "Tile.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasePawn.generated.h"


UENUM()
enum class EPawnMovement : uint8
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	DIAGONAL

	/* NORTH, // 0
	NORTHEAST, // 1
	EAST, // 2 
	SOUTHEAST, // 3
	SOUTH, // 4
	SOUTHWEST, // 5
	WEST, // 6
	NORTHWEST // 7 */
};

UCLASS()
class CHESS_API ABasePawn : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePawn();

	/* void SetTileId(const FString TileId);
	FString GetTileId() const; */

	EPawnMovement GetMovement() const;

	void SetMaxNumberSteps(int32 NumberSteps);
	int32 GetMaxNumberSteps() const;

	void SetColor(EPawnsColors PawnColor);
	EPawnsColors GetColor() const;

	
	void SetType(ETileStatus TileStatus);
	ETileStatus GetType() const;

	void SetGridPosition(const double InX, const double InY);
	FVector2D GetGridPosition() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TileId; */

	// TODO => FString ID per log




	// Color (Black = -1, Whitee = 1) per cambiare direzione se vincolata, tipo pedoni
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnsColors Color;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnMovement Movement;

	// TODO uint8 ?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxNumberSteps;

	// Color + Type (e.g. Black Rook)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ETileStatus Type;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SelfDestroy();
};
