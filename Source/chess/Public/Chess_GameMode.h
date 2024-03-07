// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


// UI
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/TextBlock.h"

#include "CoreMinimal.h"
#include "Players/Chess_PlayerInterface.h"
#include "BasePawn.h"
#include "GameFramework/GameModeBase.h"
#include "Chess_GameMode.generated.h"

class AActor;
class AGameField;
class ABasePawn;

enum class EPawnType : uint8;
enum class EPawnColor : int8;

UENUM()
enum class EDirection : uint8
{
	FORWARD,
	BACKWARD,
	HORIZONTAL,
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

USTRUCT(BlueprintType)
struct FTileSaving
{
	GENERATED_BODY()

	int8 PieceNum;
	EPawnType PawnType;
	EPawnColor PawnColor;
	EPawnStatus PawnStatus;
	int8 X;
	int8 Y;
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
	bool CanPlay; // true => user can play, false => user cannot play (e.g. while watching a replay)
	TArray<IChess_PlayerInterface*> Players;
	int32 CurrentPlayer;

	int32 MoveCounter;
	TArray<FString> RecordMoves;

	TArray<TArray<FTileSaving>> GameSaving;
	TArray<FTileSaving> CurrentBoard;

	// Who is under check || NONE || WHITE || BLACK
	EPawnColor CheckFlag; 

	// Who has been Check Mated || NONE || WHITE || BLACK
	EPawnColor CheckMateFlag; 

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize;

	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> PawnPromotionMenuWidgetRef;

	UPROPERTY()
	UUserWidget* PawnPromotionWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> ReplayWidgetRef;

	UPROPERTY()
	UUserWidget* ReplayWidget;

	static const int8 MAX_REPLAY_SHOW_MOVES = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> ButtonWidgetRef;

	UPROPERTY()
	UUserWidget* ButtonWidget;


	// Type chosen for pawn promotion
	EPawnType PawnPromotionType;

	// Used to remember which pawn to promote
	FVector2D LastGridPosition;

	// Used to remember previous grid position in notation
	FVector2D PreviousGridPosition;



	AChess_GameMode();

	virtual void BeginPlay() override;

	void ChoosePlayerAndStartGame();

	void EndTurn(const int32 PlayerNumber);

	int32 GetNextPlayer(int32 Player);

	void TurnNextPlayer();

	// void ComputeCheck();

	/*
	*/
	UFUNCTION(BlueprintCallable)
	void SetPawnPromotionChoice(EPawnType PawnType);

	/*
	*/
	UFUNCTION(BlueprintCallable)
	void ReplayMove(UTextBlock* TxtBlock);

	void AddToReplay(const ABasePawn* Pawn, const bool EatFlag = false);
	FString ComputeMoveName(const ABasePawn* Pawn, const bool EatFlag = false) const;

	/*
	*/
	ABasePawn* SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y);

	/*
	*/
	void DespawnPawn(int8 X, int8 Y);

	/*
	*/
	EPawnColor IsCheck(ABasePawn* Pawn = nullptr, const int8 NeX = -1, const int8 NewY = -1);

	/*
	* ShowAttackable: bool => just when i wanna compute attackable tiles (it uses pawns only in diagonal)
	*/
	TArray<std::pair<int8, int8>> ShowPossibleMoves(ABasePawn* Pawn, const bool CheckTest = false, const bool ShowAttackable = false, const bool CheckCheckFlag = true);

	/*
	*/
	bool IsValidMove(ABasePawn* Pawn, const int8 NewX, const int8 NewY, const bool TestFlag = false, const bool ShowAttackable = false, const bool CheckCheckFlag = true);

	/*
	*/
	bool CheckDirection(const EDirection Direction, ABasePawn* Pawn, const FVector2D NewGridPosition, const FVector2D CurrGridPosition, const bool TestFlag = false) const;

	/*
	*/
	bool IsLineClear(const ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const;

	/*
	*/
	bool IsValidTile(const int8 X, const int8 Y) const;
};
