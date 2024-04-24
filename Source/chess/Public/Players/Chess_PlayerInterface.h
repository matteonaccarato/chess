// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "../Chess_GameMode.h"
#include "../Chess_GameInstance.h"
#include "Chess_PlayerInterface.generated.h"

enum class EPieceColor : int8;

class AChess_GameField;
class ABasePiece;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UChess_PlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CHESS_API IChess_PlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	static constexpr int8 TIMER_NONE = 1;

	int32 PlayerNumber;
	EPieceColor Color;
	TArray<TArray<std::pair<int8, int8>>> AttackableTiles;
	bool IsMyTurn = false;
	bool bIsActivePlayer = true;

	UChess_GameInstance* GameInstance;

	virtual void OnTurn() {};
	virtual void OnWin() {};
	virtual void OnLose() {};
	void OnDraw();
};
