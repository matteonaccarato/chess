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
#include "BasePawn.h"
#include "ChessEnums.h"
#include "Managers/ReplayManager.h"
#include "GameFramework/GameModeBase.h"
#include "Chess_GameInstance.h"
#include "Chess_GameMode.generated.h"

class IChess_PlayerInterface;
class AActor;
class AGameField;
class ABasePawn;

enum class EPawnType : uint8;
enum class EPawnColor : int8;


USTRUCT(BlueprintType)
struct FPieceSaving
{
	GENERATED_BODY()

	int8 X;
	int8 Y;
	EPawnStatus Status;
};


USTRUCT(BlueprintType)
struct FCastlingInfo
{
	GENERATED_BODY()

	bool KingMoved = false;
	bool RooksMoved[2] = { false, false }; // left / right
};


/**
 * TODO => mettere attributi come protected
 */
UCLASS()
class CHESS_API AChess_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	UChess_GameInstance* GameInstance;

	bool IsGameOver;
	int8 ReplayInProgress; // shows the number of turn currently replaying
	TArray<IChess_PlayerInterface*> Players;
	int32 CurrentPlayer;
	TArray<std::pair<int8, int8>> ShownPossibleMoves;

	TArray<TArray<std::pair<int8, int8>>> TurnPossibleMoves;
	TArray<int8> WhitePiecesCanMove;
	TArray<int8> BlackPiecesCanMove;
	
	FCastlingInfo CastlingInfoWhite;
	FCastlingInfo CastlingInfoBlack; 

	int32 MoveCounter;
	TArray<FString> RecordMoves;

	TArray<TArray<FPieceSaving>> GameSaving;
	TArray<FPieceSaving> CurrentBoard;

	int8 KingWhitePieceNum = -1;
	int8 KingBlackPieceNum = -1;

	// Notifies check situation || NONE || WHITE || BLACK || BOTH
	EPawnColor CheckFlag; 

	// Match result
	EMatchResult MatchStatus; 

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	/* UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 FieldSize; */

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<class UUserWidget> ButtonWidgetRef;

	UPROPERTY()
	UUserWidget* ButtonWidget;


	// Type chosen for pawn promotion
	EPawnType PawnPromotionType;

	// Used to remember which information about last move
	ABasePawn* LastPiece;
	FVector2D LastGridPosition;
	bool LastEatFlag = false;

	// Usefult for the seventy-five move rule
	int LastPawnMoveHappened = 0;
	int LastCaptureHappened = 0;

	// Used to remember previous grid position in notation
	FVector2D PreviousGridPosition;



	AChess_GameMode();
	virtual void BeginPlay() override;

	void ChoosePlayerAndStartGame();
	int32 GetNextPlayer(int32 Player);
	void TurnNextPlayer();
	void EndTurn(const int32 PlayerNumber, const bool PiecePromotionFlag = false);


	/*
	*/
	UFUNCTION(BlueprintCallable)
	void SetPawnPromotionChoice(EPawnType PawnType);

	/*
	*/
	UFUNCTION(BlueprintCallable)
	void ReplayMove(UTextBlock* TxtBlock);

	


	/*
	*/
	EPawnColor IsCheck(ABasePawn* Pawn = nullptr, const int8 NeX = -1, const int8 NewY = -1);

	EMatchResult ComputeMatchResult(TArray<int8>& WhitePieces, TArray<int8>& BlackPieces);

	/*
	* ShowAttackable: bool => just when i wanna compute attackable tiles (it uses pawns only in diagonal)
	*/
	TArray<std::pair<int8, int8>> ShowPossibleMoves(ABasePawn* Pawn, const bool ShowAttackable = false, const bool CheckCheckFlag = true, const bool UpdateWhoCanGoFlag = false);
	void ComputeAttackableTiles();

	/*
	*/
	bool IsValidMove(ABasePawn* Pawn, const int8 NewX, const int8 NewY, const bool TestFlag = false, const bool ShowAttackable = false, const bool CheckCheckFlag = true, const bool CastlingFlag = false);

	bool MakeMove(ABasePawn* Piece, const int8 NewX, const int8 NewY, bool Simulate = false);
	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> ComputeAllPossibleMoves(EPawnColor Color, const bool ShowAttackable = false, const bool CheckCheck = true, const bool UpdateTurnMoves = true);



	void BackupTiles(TArray<FTileStatus>& TilesStatus) const;
	void RestoreTiles(TArray<FTileStatus>& TilesStatusBackup);

	void BackupPiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfo) const;
	void RestorePiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfoBackup);


private:
	EPawnColor CheckKingsUnderAttack() const;
	void InitTurn();


	bool SameConfigurationBoard(const int8 Times) const;
	bool SeventyFive_MoveRule() const;
	bool ImpossibilityToCheckmate() const;
};
