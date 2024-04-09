// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Chess_GameInstance.generated.h"

UENUM()
enum class EMatchMode : uint8
{
	HUMAN_CPU_RANDOM, 
	HUMAN_CPU_MINIMAX,
	RANDOM_RANDOM,
	RANDOM_MINIMAX,
	MINIMAX_MINIMAX
};

/**
 * 
 */
UCLASS()
class CHESS_API UChess_GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	int32 ScoreHumanPlayer = 0;

	UPROPERTY(EditAnywhere)
	int32 ScoreAiPlayer = 0;

	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	UPROPERTY(EditAnywhere)
	EMatchMode MatchMode;

	void IncrementScoreHumanPlayer();
	void IncrementScoreAiPlayer();

	UFUNCTION(BlueprintCallable)
	int32 GetScoreHumanPlayer() const;

	UFUNCTION(BlueprintCallable)
	int32 GetScoreAiPlayer() const;

	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage() const;

	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);

	UFUNCTION(BlueprintCallable)
	EMatchMode GetMatchMode() const;

	UFUNCTION(BlueprintCallable)
	void SetMatchMode(EMatchMode Mode);
};
