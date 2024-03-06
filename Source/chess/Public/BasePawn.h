// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "vector"
// #include "Chess_GameMode.h"
// #include "Tile.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BasePawn.generated.h"

class AChess_GameMode;

UENUM()
enum class EPawnType : uint8
{
	NONE, // 0
	ROOK, // 1
	KNIGHT, // 2
	BISHOP, // 3
	QUEEN, // 4
	KING, // 5
	PAWN, // 6
};

// Color (Black = -1, Whitee = 1) per cambiare direzione se vincolata, tipo pedoni
UENUM()
enum class EPawnColor : int8
{
	NONE = 0,
	WHITE = 1,
	BLACK = -1
};

UENUM()
enum class ECardinalDirection : uint8
{
	NORTH,
	NORTHEAST,
	EAST,
	SOUTHEAST,
	SOUTH,
	SOUTHWEST,
	WEST,
	NORTHWEST,
	KNIGHT_TR, // top-right
	KNIGHT_TL, // top-left
	KNIGHT_RT, // right-top
	KNIGHT_RB, // right-bottom
	KNIGHT_BR, // bottom-right
	KNIGHT_BL, // bottom-left
	KNIGHT_LB, // left-bottom
	KNIGHT_LT // left-top
};

UENUM()
enum class EPawnStatus: int8
{
	ALIVE, // 0
	DEAD // 1
};

UCLASS()
class CHESS_API ABasePawn : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePawn();

	FString GetId() const;

	TArray<ECardinalDirection> GetCardinalDirections() const;

	void SetMaxNumberSteps(int32 NumberSteps);
	int32 GetMaxNumberSteps() const;

	void SetColor(EPawnColor PawnColor);
	EPawnColor GetColor() const;

	
	void SetType(EPawnType PawnType);
	EPawnType GetType() const;

	void SetStatus(EPawnStatus PawnStatus);
	EPawnStatus GetStatus() const;

	void SetGridPosition(const double InX, const double InY);
	FVector2D GetGridPosition() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// TODO uproperty a cosa serve, va messo a tutti?
	/* UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TileId; */

	// TODO => FString ID per log

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Id;

	// Color (Black = -1, Whitee = 1) per cambiare direzione se vincolata, tipo pedoni
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnColor Color;

	// TODO int8 ?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 MaxNumberSteps;

	// Type (e.g. ROOK)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnType Type;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<ECardinalDirection> CardinalDirections;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnStatus Status;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector2D TileGridPosition;

public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void SelfDestroy();
};
