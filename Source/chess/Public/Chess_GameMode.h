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

#include "Components/AudioComponent.h"

#include "HAL/PlatformFileManager.h"
#include "CoreMinimal.h"
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

	/* CONST EXPRs */
	static constexpr int8 SHORT_CASTLING_OFFSET = -2;
	static constexpr int8 LONG_CASTLING_OFFSET	= 3;

	static constexpr int8 MIN_NUMBER_SPAWN_PLAYERS = 2;
	static constexpr float CAMERA_POS_Z = 1250.0f;

	static constexpr char* STATISTICS_DIRECTORY_NAME = "GameData";
	static constexpr char* FILENAME_HUMAN_RANDOM	 = "/human_random.csv";
	static constexpr char* FILENAME_HUMAN_MINIMAX	 = "/human_minimax.csv";
	static constexpr char* FILENAME_RANDOM_RANDOM	 = "/random_random.csv";
	static constexpr char* FILENAME_RANDOM_MINIMAX	 = "/random_minimax.csv";
	static constexpr char* FILENAME_MINIMAX_MINIMAX	 = "/minimax_minimax.csv";


	/* ATTRIBUTES */
	UChess_GameInstance* GameInstance;

	bool IsGameOver;
	bool bIsHumanPlaying = false;
	int ReplayInProgress; // shows the number of turn currently replaying
	TArray<IChess_PlayerInterface*> Players;
	int32 CurrentPlayer;
	TArray<std::pair<int8, int8>> ShownPossibleMoves;

	TArray<TArray<std::pair<int8, int8>>> TurnPossibleMoves;
	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> WhitePiecesCanMove;
	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>> BlackPiecesCanMove;
	
	FCastlingInfo CastlingInfoWhite;
	FCastlingInfo CastlingInfoBlack; 

	int32 MoveCounter;
	TArray<FString> RecordMoves;

	TArray<TArray<FPieceSaving>> GameSaving;
	TArray<FPieceSaving> CurrentBoard;

	/* Useful piece numbers to store */
	int8 KingWhitePieceNum = -1;
	int8 KingBlackPieceNum = -1;
	int8 RookWhiteLeftPieceNum = -1;
	int8 RookWhiteRightPieceNum = -1;
	int8 RookBlackLeftPieceNum = -1;
	int8 RookBlackRightPieceNum = -1;


	// Notifies check situation || NONE || WHITE || BLACK || BOTH
	EPawnColor CheckFlag; 

	// Match result
	EMatchResult MatchStatus; 

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AGameField> GameFieldClass;

	UPROPERTY(VisibleAnywhere)
	AGameField* GField;

	FTimerHandle StopwatchTimerHandle;

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

	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* GameStartSound;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* MoveSound;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* CastlingSound;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* CaptureSound;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* CheckSound;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* GameOverCheckmateSound;
	UPROPERTY(EditDefaultsOnly, Category = "Components") class USoundBase* GameOverDrawSound;


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



	/* METHODS */
	AChess_GameMode();
	virtual void BeginPlay() override;

	/* TURN HANDLING */
	void ChoosePlayerAndStartGame();
	int32 GetNextPlayer(int32 Player);
	void TurnNextPlayer();
	void InitTurn();
	void EndTurn(const int32 PlayerNumber, const bool PiecePromotionFlag = false);


	/*
	 * If no parameters are passed the check situation is computed calculating the attackable tiles
	 * and verifying if a king is under attack, after this it is assigned to the gamemode attribute.
	 * Otherwise, the new check situation obtained by simulating the move of Piece on [NewX, NewY] is evaluated
	 *
	 * @param Pawn	ABasePawn* = nullptr	Piece to move on new x and new y
	 * @param NewX	const int8 = -1			New X position of the piece to move
	 * @param NewY	const int8 = -1			New Y position of the piece to move
	 *
	 * @return		EPawnColor				Check situation
	 */
	EPawnColor IsCheck(
		ABasePawn* Pawn			= nullptr, 
		const int8 NeX			= -1, 
		const int8 NewY			= -1, 
		const bool CastlingFlag = false);


	/*
	 * Compute all attackable tiles based on the current situation of data structure
	 *		( GameMode->GField->TileArray, GameMode->GField->PawnArray ).
	 * Update the AttackableTileStatus attribute of each Tile where necessary
	 */
	void ComputeAttackableTiles();


	/*
	 * Look if each king is on a tile which is attackable from an opponent piece
	 *
	 * @return	 EPawnColor		Color of the king under check (NONE, WHITE, BLACK, BOTH). BOTH means a NOT valid situation
	 */
	EPawnColor CheckKingsUnderAttack() const;


	/*
	 * Compute match result based on white and black pieces which can be moved, check situation and draw conditions
	 *
	 * @param WhitePieces	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>&		Reference to white pieces which can be moved.
	 *																					Each element of the TArray is like <piece_number, <new_x, new_y>>
	 * @param BlackPieces	TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>&		Reference to black pieces which can be moved.
	 *																					Each element of the TArray is like <piece_number, <new_x, new_y>
	 * 
	 * @return				EMatchResult												Match result computed
	 */
	EMatchResult ComputeMatchResult(
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& WhitePieces,
		TArray<std::pair<int8, TArray<std::pair<int8, int8>>>>& BlackPieces
	);


	/*
	 * Compute the eligible moves of the chess piece passed as parameter
	 *
	 * @param Pawn								ABasePawn*							Piece on which to calculate the eligible moves
	 * @param ConsiderOnlyAttackableTiles		bool = false						Flag to determine if only the attackable tiles should be taken into account 
	 *																				(possible tiles to move on and attackable tiles are different for pawns)
	 * @param CheckCheckFlag					bool = false						Flag to determine if checking the new check situation should be evaluated
	 * @param UpdateWhoCanGoFlag				bool = false						Flag to notify if updating WhoCanGo: FTileStatus, 
	 *																				attribute of the tile taken into account should be updated with the current piece
	 *
	 * @return									TArray<std::pair<int8, int8>>		TArray made of new possible X,Y of the chess piece
	 */
	TArray<std::pair<int8, int8>> ShowPossibleMoves(
		ABasePawn* Pawn, 
		const bool ConsiderOnlyAttackableTiles = false,
		const bool CheckCheckFlag			   = true, 
		const bool UpdateWhoCanGoFlag		   = false
	);


	/*
	 * Compute if a move (specified through parameters) is valid or not (following the rule of chess game)
	 *
	 * @param Pawn							ABasePawn*			Piece to try to move on new x and new y 
	 * @param NewX							const int8			New x position of the piece
	 * @param NewY							const int8			New y position of the piece
	 * @param ConsiderOnlyAttackableTiles	const bool = false	Determine if only the attackable tiles should be taken into account 
	 *															(possible tiles to move on and attackable tiles are different for pawns)
	 * @param CheckCheckFlag				const bool = true	Determine if checking check state after this move is required
	 * @param CastlingFlag					const bool = false	Determine if checking castling situation is required
	 * 
	 * @param return 						bool				Determine if a move is valid or not
	 */
	bool IsValidMove(
		ABasePawn* Pawn, 
		const int8 NewX, 
		const int8 NewY, 
		const bool ConsiderOnlyAttackableTiles = false,
		const bool CheckCheckFlag = true, 
		const bool CastlingFlag = false
	);


	/*
	 * Make the move specified through paramters
	 *
	 * @param Piece		ABasePawn*		Piece to move
	 * @param NewX		const int8		X to end to
	 * @param NewY		const int8		Y to end to
	 * @param Simulate	bool = false	Determine if the move is just a simulation or not.
	 *									If so, graphically moving the piece is not required
	 *
	 * @return			bool			Flag to notify if a capture happened
	 */
	bool MakeMove(ABasePawn* Piece, const int8 NewX, const int8 NewY, bool Simulate = false);


	/*
	 * Promote the pawn with a new chess piece (type passed as parameter)
	 *
	 * @param PawnType	EPawnType	Type of the new chess piece to spawn
	 */
	UFUNCTION(BlueprintCallable)
	void SetPawnPromotionChoice(EPawnType PawnType);
	


	/* DRAW */
	/*
	 * If the same position occurs five times during the course of the game, the game is automatically a draw.
	 * 
	 * @param Times	int8	Times which the same board configuration has to occured to make the game a draw.
	 * 
	 * @return		bool	If the same configuration board happened "times" times
	 */
	bool SameConfigurationBoard(const int8 Times) const;

	/*
	 * If no capture or no pawn move has occurred in the last 75 moves (by both players), the game is automatically a draw.
	 * 
	 * @return	bool If this condition happened 
	 */
	bool SeventyFive_MoveRule() const;

	/*
	 * Impossibility of checkmate evaluation:
	 *	If a position arises in which neither player could possibly give checkmate by a series of legal moves, the game is a draw. 
	 *	Such a position is called a dead position. This is usually because there is insufficient material left.
	 * Combinations:
	 *  - king versus king
	 *	- king and bishop versus king
	 *  - king and knight versus king
	 *  - king and bishop versus king and bishop with the bishops on the same color.
	 * 
	 * @return	bool	If this condition happened
	 */
	bool ImpossibilityToCheckmate() const;


	/*
	 * Load the turn specified as parameter (e.g. "1. e4" => Replay of the turn number 4.
	 * If user decides to rewind the game (clicking the move N and selecting one of his pieces) at turn N,
	 * the board will be bring back to the status at turn N and the user is allowed to start again the turn N+1
	 *
	 * @param TxtBlock	UTextBlock*		Text block containing the turn to replay (e.g. "1. e4)"
	 */
	UFUNCTION(BlueprintCallable)
	void ReplayMove(UTextBlock* TxtBlock);


	/*
	 * Look if a word match the sequence of TCHARs.
	 * Its repetitively call allows to search if a sequence of TCHARs composes a specific word (TArray<TCHAR>)
	 *
	 * @param Chr					TChar			Current char to analyse
	 * @param WordToSearch			TArray<TCHAR>&	Word to search in the sequence of TCHAR
	 * @param WordToSearch_Idx		int&			Index of the last success char analysed in the word to search
	 * @param NotifyWordCompletion	bool&			Flag to notify when the word is completed
	 * 
	 * @return						bool			If Chr matches the char at WordToSearch_Idx in the word to search
	 */
	static bool SearchWordByChar(TCHAR Chr, TArray<TCHAR>& WordToSearch, int& WordToSearch_Idx, bool& NotifyWordCompletion);


private:

	/*
	 * Save game information on file CSV.
	 * Read file to get previous information and update with the data of current game data.
	 * Header CSV:
	 *	#GAME,
	 *	PLAYER_1_STATUS,
	 *	PLAYER_2_STATUS,
	 *	#MOVES,
	 *	DURATION(seconds),
	 *	PL_1_WINS/GAMES,
	 *	PL_1_LOSSES/GAMES,
	 *	PL_2_WINS/GAMES,
	 *	PL_2_LOSSES/GAMES,
	 * 	DRAWS/GAMES
	 *
	 * @param FilePath			FString		File path of the file to read/write on
	 * @param bOutSuccess		bool&		Notify if operations concluded successfully
	 * @param OutInfoMessage	FString&	Output message
	 */
	void SaveGameOnFile(FString& FilePath, bool& bOutSuccess, FString& OutInfoMessage) const;
};
