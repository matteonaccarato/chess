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
	float CellPadding;

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

	TArray<ATile*>& GetTileArray();
	TArray<ABasePawn*>& GetPawnArray();

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


	/*
	 * Calculate the distance between two pieces given as arguments.
	 * sqrt((x-x')^2 + (y-y')^2)
	 *
	 * Piece1	const ABasePawn*	1st piece (x,y)
	 * Piece2	const ABasePawn*	2nd piece (x',y')
	 *
	 * return	int8	Distance between the two pieces given as arguments
	 */
	int8 DistancePieces(const ABasePawn* Piece1, const ABasePawn* Piece2) const;
	
	


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


	/*
	 * Spawn the pawn specified through parameters
	 *
	 * PawnType	EPawnType		: type of the pawn to spawn
	 * PawnColor	EPawnColor	: color of the pawn to spawn
	 * X			int8		: x position of the pawn to spawn
	 * Y			int8		: y position of the pawn to spawn
	 *
	 * return		ABasePawn*	: Pointer to the recently spawned pawn
	 */
	ABasePawn* SpawnPawn(EPawnType PawnType, EPawnColor PawnColor, int8 X, int8 Y, int8 PlayerOwner = ChessEnums::NOT_ASSIGNED);


	/*
	 * Despawn pawn which is on the tile specified
	 *
	 * X	int8	: x position of the pawn to despawn
	 * Y	int8	: y position of the pawn to despawn
	 */
	void DespawnPawn(int8 X, int8 Y, bool Simulate = false);

	
//protected:
	// Called when the game starts or when spawned
	//virtual void BeginPlay() override;

// public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;

};
