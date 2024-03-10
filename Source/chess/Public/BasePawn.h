// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// #include "vector"
// #include "Chess_GameMode.h"
#include "Tile.h"
#include "CoreMinimal.h"
#include "ChessEnums.h"
#include "GameFramework/Actor.h"
#include "BasePawn.generated.h"

// class ATile;
class AChess_GameMode;

UCLASS()
class CHESS_API ABasePawn : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePawn();

	FString GetId() const;

	int GetPieceNum() const;
	void SetPieceNum(int Num);

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

	// !simulate is used to actually move actor in the scene
	void Move(ATile* OldTile, ATile* NewTile, bool Simulate = false);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// TODO uproperty a cosa serve, va messo a tutti?
	/* UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString TileId; */

	// TODO => FString ID per log

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Id;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int PieceNum = -1;

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
