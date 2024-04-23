// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Tile.h"
#include "CoreMinimal.h"
#include "ChessEnums.h"
#include "GameField.h"
#include "GameFramework/Actor.h"
#include "BasePawn.generated.h"

class AChess_GameMode;
class AGameField;

UCLASS()
class CHESS_API ABasePawn : public AActor
{
	GENERATED_BODY()
	
public:	

	/* CONST EXPRs */
	static constexpr int8 KING_MAX_NUMBER_STEPS		= 1;
	static constexpr int8 QUEEN_MAX_NUMBER_STEPS	= 8;
	static constexpr int8 ROOK_MAX_NUMBER_STEPS		= 8;
	static constexpr int8 BISHOP_MAX_NUMBER_STEPS	= 8;
	static constexpr int8 KNIGHT_MAX_NUMBER_STEPS	= 1;
	static constexpr int8 PAWN_MAX_NUMBER_STEPS		= 2;


	/* METHODS */
	/*
	 * Construct the piece assigning its moving properties 
	 *	(type, max number of steps and movement direction)
	 */
	ABasePawn();

	UStaticMeshComponent* GetStaticMeshComponent() const;

	FString GetId() const;

	int GetPieceNum() const;
	void SetPieceNum(int Num);

	TArray<ECardinalDirection> GetCardinalDirections() const;

	void SetMaxNumberSteps(int NumberSteps);
	int GetMaxNumberSteps() const;

	void SetColor(EPawnColor PawnColor);
	EPawnColor GetColor() const;
	
	void SetType(EPawnType PawnType);
	EPawnType GetType() const;

	void SetStatus(EPawnStatus PawnStatus);
	EPawnStatus GetStatus() const;

	void SetGridPosition(const double InX, const double InY);
	FVector2D GetGridPosition() const;


	/*
	 * Compute the offsets (X,Y) based on the paramaters (steps, direction and color)
	 *
	 * @param Steps			const int8				Number of steps to perform
	 * @param Direction		ECardinalDirection		Direction to follow during the move
	 * @param PieceColor	EPawnColor				Color of the piece
	 *
	 * @return				std::pair<int8, int8>	Pair containing XOffset as first argument and YOffset as second one
	 */
	std::pair<int8, int8> GetXYOffset(const int8 Steps, const ECardinalDirection Direction) const;


	/*
	 * Check if the movement along a specific direction is allowed or not: the line is clear and [X,Y] match the constraints of the selected direction.
	 * It does not take into account the piece rules, but only the given parameters
	 *
	 * @param Board				const AGameField*		Board to refer to
	 * @param Direction			const EDirection		Direction to do the check on
	 * @param NewGridPosition	const FVector2D			X and Y of the new grid position
	 * @param CurrGridPosition	const FVector2D			X and Y of the current grid position
	 *
	 * @return	bool	Whether the movement along the specified direction is allowed or not
	 */
	bool CheckDirection(const AGameField* GameBoard, const EDirection Direction, const FVector2D NewGridPosition, const FVector2D CurrGridPosition);


	/*
	 * Make the move specified through paramters
	 *
	 * @param OldTile	ATile*			Tile where the piece starts from
	 * @param NewTile	ATile*			Tile where the piece moves to
	 * @param Simulate	bool = false	Determine if the move is just a simulation or not.
	 *									If so, graphically moving the piece is not required
	 */
	void Move(ATile* OldTile, ATile* NewTile, bool Simulate = false);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	/* ATTRIBUTES */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString Id;

	// Uniquely identifier of the piece in the chess board
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int PieceNum = -1;

	// Color (e.g. Black = -1, White = 1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EPawnColor Color;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int MaxNumberSteps;

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
