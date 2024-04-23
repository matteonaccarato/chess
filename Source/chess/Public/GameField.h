// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Chess_GameMode.h"
#include "CoreMinimal.h"
#include "ChessEnums.h"
#include "Tile.h"
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


	/* ATTRIBUTES */
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


	/* METHODS */
	// Sets default values for this actor's properties
	AGameField();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	FVector2D GetPosition(const FHitResult& Hit);
	FVector GetRelativeLocationByXYPosition(const int32 InX, const int32 InY) const;
	FVector2D GetXYPositionByRelativeLocation(const FVector& Location) const;


	/*
	 * Generate the game board (graphically and initializing data structures)
	 */
	void GenerateField();


	/*
	 * Reset the game board (graphically and resetting data structures)
	 *
	 * @param bRestartGame	bool	Flag to notify if starting a new game is required or not after having reset the board
	 */
	UFUNCTION(BlueprintCallable)
	void ResetField(bool bRestartGame);


	/*
	 * Load the board specified as argument
	 *
	 * @param Board		const TArray<FPieceSaving>&		Board to load
	 */
	void LoadBoard(const TArray<FPieceSaving>& Board);


	/*
	 * Determine if X and Y are related to a valid tile or not (>= 0 AND < Gameboard.size)
	 *	
	 * @param X		const int8		Coordinate X
	 * @param Y		const int8		Coordinate Y
	 *
	 * @return		bool			true  -> no pieces along the movement
	 *								false -> there is a piece along the movement
	 */
	bool IsValidTile(const int8 X, const int8 Y) const;


	/*
	 * Check if the line from current grid position to a new position (delta_x, delta_y) is clear from other pieces or not
	 * 
	 * @param Line					ELine				Line along the movement is performed (HORIZONTAL | VERTICAL | DIAGONAL)
	 * @param CurrGridPosition		const FVector2D		Current grid position (e.g. [3,2])
	 * @param DeltaX				const int8			Movement delta X 
	 * @param DeltaY				const int8			Movement delta Y
	 * 
	 * @param return				bool				true  -> no pieces along the movement
	 *													false -> there is a piece along the movement
	 */
	bool IsLineClear(const ELine Line, const FVector2D CurrGridPosition, const int8 DeltaX, const int8 DeltaY) const;


	/* 
     * Spawn the pawn specified through parameters
	 *
	 * @param PawnType		EPawnType	Type of the pawn to spawn
	 * @param PawnColor		EPawnColor	Color of the pawn to spawn
	 * @param X				int8		X position of the pawn to spawn
	 * @param Y				int8		Y position of the pawn to spawn
	 * @param PlayerOwner	int8		Player owner of the new piece
	 * @param Simulate		bool		Flag if the spawn should be simulated or not (graphically show the piece or not)
	 *
	 * @return				ABasePawn*	Pointer to the spawned pawn
	 */
	ABasePawn* SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y, int8 PlayerOwner = ChessEnums::NOT_ASSIGNED, bool Simulate = false);


	/*
	 * Despawn piece which is on the tile specified
	 *
	 * @param X			int8	X position of the piece to despawn
	 * @param Y			int8	Y position of the piece to despawn
	 * @param Simulate	bool	Flag if the spawn should be simulated or not (graphically hide the piece or not)
	 */
	void DespawnPawn(int8 X, int8 Y, bool Simulate = false);


	/*
	 * Do a backup of the tiles status information in the data structured passed as parameter
	 *
	 * @param TilesStatus  TArray<FTileStatus>&		Ordered collection where to store tiles status information
	 */
	void BackupTiles(TArray<FTileStatus>& TilesStatus) const;

	/*
	 * Restore the tiles status information through the data structured passed as parameter
	 *
	 * @paramn TilesStatusBackup  TArray<FTileStatus>&	Ordered collection where to get tiles status information to restore
	 */
	void RestoreTiles(TArray<FTileStatus>& TilesStatusBackup);

	/*
	 * Do a backup of pieces information in the data structured passed as parameter
	 *
	 * @param PiecesInfo  TArray<std::pair<EPawnStatus, FVector2D>>&	Ordered collection (by PieceNum) to store pieces information
	 *																	1st element: piece status (ALIVE / DEAD)
	 *																	2nd element: grid position
	 */
	void BackupPiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfo) const;

	/*
	 * Restore pieces information in the data structured through the TArray passed as parameter
	 *
	 * @param PiecesInfoBackup  TArray<std::pair<EPawnStatus, FVector2D>>&		Ordered collection (by PieceNum) to store pieces information
	 *																			1st element: piece status (ALIVE / DEAD)
	 *																			2nd element: grid position
	 */
	void RestorePiecesInfo(TArray<std::pair<EPawnStatus, FVector2D>>& PiecesInfoBackup);

	
//protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

// public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
};
