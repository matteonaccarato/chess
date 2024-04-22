// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chess_GameMode.h"
#include "CoreMinimal.h"
#include "ChessEnums.h"
#include "Tile.h"
// #include "BasePawn.h"
#include "Players/Chess_PlayerInterface.h"
#include "GameFramework/Actor.h"
#include "GameField.generated.h"

class ABasePawn;
struct FPieceSaving;

UENUM()
enum class ETileMaterialType : uint8
{
	STANDARD,
	ACTIVE		// when showing possible moves
};


UCLASS()
class CHESS_API AGameField : public AActor
{
	GENERATED_BODY()
	
public:	

	/* CONST EXPRs */
	static constexpr int8 GAMEBOARD_SIZE		= 8;
	static constexpr int8 TILE_SIZE				= 120;
	static constexpr int8 TILE_PADDING			= 0;
	static constexpr int8 PAWN_ROWS				= 2;

	static constexpr float LETTERS_NUMS_SCALE	= 0.7;
	static constexpr float CHESS_PIECES_SCALE	= 0.8;
	static constexpr float CHESS_PIECES_Z		= 0.03;
	static constexpr float TILES_Z				= 0.2;

	static constexpr EPawnType PawnTypesOnRow[] = {
		EPawnType::ROOK,
		EPawnType::KNIGHT,
		EPawnType::BISHOP,
		EPawnType::QUEEN,
		EPawnType::KING,
		EPawnType::BISHOP,
		EPawnType::KNIGHT,
		EPawnType::ROOK
	};



	// TILES
	UPROPERTY(Transient)
	TArray<ATile*> TileArray;

	UPROPERTY(Transient)
	TMap<FVector2D, ATile*> TileMap;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ATile> TileClass; 

	UPROPERTY(EditAnywhere, Category = "Materials")
	TMap<ETileMaterialType, UMaterialInterface*> MaterialsLight;	// standard | active (when showing possible moves)

	UPROPERTY(EditAnywhere, Category = "Materials")
	TMap<ETileMaterialType, UMaterialInterface*> MaterialsDark;		// standard | active (when showing possible moves)


	// PIECES
	UPROPERTY(EditDefaultsOnly)
	TMap<EPawnType, TSubclassOf<ABasePawn>> ChessPieces;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TMap<EPawnType, UMaterialInterface*> ChessPiecesWhiteMaterials;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	TMap<EPawnType, UMaterialInterface*> ChessPiecesBlackMaterials;

	UPROPERTY(Transient)
	TArray<ABasePawn*> PawnArray;
	
	UPROPERTY(Transient)
	TMap<FVector2D, ABasePawn*> PawnMap;


	// Letters / Numbers
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> LetterNumberClass;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TArray<UMaterialInterface*> Letters;

	UPROPERTY(EditAnywhere, Category = "Materials")
	TArray<UMaterialInterface*> Numbers;




	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float NormalizedCellPadding;

	// Size of field
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Size;

	// Number of rows where pawns have to be placed
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Pawns_Rows; 

	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TilePadding; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TileSize;

	// Sets default values for this actor's properties
	AGameField();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;


	/*
	 * Reset the game board (graphically and in data structures)
	 */
	UFUNCTION(BlueprintCallable)
	void ResetField(bool bRestartGame);

	/*
	*/
	void GenerateField();

	FVector2D GetPosition(const FHitResult& Hit);

	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;
	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;

	
	/*
	 * Specify if X and Y are related to a valid tile or not (>= 0 AND < Gameboard.size)
	 *
	 * X	const int8	Coordinate X
	 * Y	const int8	Coordinate Y
	 *
	 * return	bool	true  -> no pieces along the movement
	 *					false -> there is a piece along the movement
	 */
	bool IsValidTile(const int8 X, const int8 Y) const;


	/* TODO => rifare commento
	 * Calculate the distance between two pieces given as arguments.
	 * sqrt((x-x')^2 + (y-y')^2)
	 *
	 * Piece1	const ABasePawn*	1st piece (x,y)
	 * Piece2	const ABasePawn*	2nd piece (x',y')
	 *
	 * return	int8	Distance between the two pieces given as arguments
	 */
	// bool CanReachBlockOpponentKing(const ABasePawn* Piece, const ABasePawn* OpponentKing) const;
	
	


	/*
	 *  Load the board specified as argument
	 *
	 * Line					ELine				Line along the movement is performed (HORIZONTAL | VERTICAL | DIAGONAL)
	 * CurrGridPosition		const FVector2D		Current grid position (e.g. [3,2])
	 * DeltaX				const int8			Movement delta X
	 * DeltaY				const int8			Movement delta Y
	 *
	 * return				bool				true  -> no pieces along the movement
	 *											false -> there is a piece along the movement
	 */
	bool IsLineClear(const ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const;




	/*
	 * Load the board specified as argument
	 *
	 * Board	const TArray<FPieceSaving>&		Board to load
	 */
	void LoadBoard(const TArray<FPieceSaving>& Board);


	/* TODO => rifare commento
	 * Spawn the pawn specified through parameters
	 *
	 * PawnType	EPawnType		: type of the pawn to spawn
	 * PawnColor	EPawnColor	: color of the pawn to spawn
	 * X			int8		: x position of the pawn to spawn
	 * Y			int8		: y position of the pawn to spawn
	 *
	 * return		ABasePawn*	: Pointer to the recently spawned pawn
	 */
	ABasePawn* SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y, int8 PlayerOwner = ChessEnums::NOT_ASSIGNED, bool Simulate = false);


	/*
	 * Despawn pawn which is on the tile specified
	 *
	 * X	int8	: x position of the pawn to despawn
	 * Y	int8	: y position of the pawn to despawn
	 */
	void DespawnPawn(int8 X, int8 Y, bool Simulate = false);


	void BackupTiles(TArray<FTileStatus>& TilesStatus) const;
	void RestoreTiles(TArray<FTileStatus>& TilesStatusBackup);

	void BackupPiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfo) const;
	void RestorePiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfoBackup);

	
//protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

// public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
};
