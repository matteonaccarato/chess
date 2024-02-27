// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Players/Chess_PlayerInterface.h"
// #include "GameField.h"
// #include "BasePawn.h"
// #include "Tile.h"
#include "GameFramework/GameModeBase.h"
#include "Chess_GameMode.generated.h"

class AActor;
class AGameField;
class ABasePawn;

enum class EPawnColor : int8;
// struct FPosition;

UENUM()
enum class EDirection : uint8
{
	FORWARD,
	BACKWARD,
	HORIZONTAL,
	/* RIGHT,
	LEFT, */
	DIAGONAL,
	KNIGHT
};

UENUM()
enum class ELine : uint8
{
	HORIZONTAL,
	VERTICAL,
	DIAGONAL
};

/**
 * TODO => mettere attributi come protected
 */
UCLASS()
class CHESS_API AChess_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	bool IsGameOver;
	TArray<IChess_PlayerInterface*> Players;
	int32 CurrentPlayer;

	int32 MoveCounter;

	EPawnColor CheckFlag; // which color is under check

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	// Pawns Classes
	// Blacks
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> B_BishopClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> B_KingClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> B_KnightClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> B_PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> B_QueenClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> B_RookClass;

	// Whites
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> W_BishopClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> W_KingClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> W_KnightClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> W_PawnClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> W_QueenClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> W_RookClass;

	AChess_GameMode();

	virtual void BeginPlay() override;

	void ChoosePlayerAndStartGame();

	void SetCellPawn(const int32 PlayerNumber, const FVector& SpawnPosition);

	int32 GetNextPlayer(int32 Player);

	void TurnNextPlayer();

	bool IsCheck();

	TArray<std::pair<int8, int8>> ShowPossibleMoves(ABasePawn* Pawn, const bool CheckTest = false);
	bool IsValidMove(ABasePawn* Pawn, const int8 NewX, const int8 NewY, /*const bool EatFlag,*/ const bool TestFlag = false);
	// test flag => to do not decrement pawn max moves if just for showing possible moves
	bool CheckDirection(const EDirection Direction, ABasePawn* Pawn, const FVector2D NewGridPosition, const FVector2D CurrGridPosition, const bool TestFlag = false) const;
	bool IsLineClear(const ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const;
	bool IsValidTile(const int8 X, const int8 Y) const;
};
