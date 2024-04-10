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
	int32 ScorePlayer_1 = 0;

	UPROPERTY(EditAnywhere)
	int32 ScorePlayer_2 = 0;

	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	UPROPERTY(EditAnywhere)
	EMatchMode MatchMode;

	void IncrementScorePlayer_1();
	void IncrementScorePlayer_2();

	UFUNCTION(BlueprintCallable)
	int32 GetScorePlayer_1() const;

	UFUNCTION(BlueprintCallable)
	int32 GetScorePlayer_2() const;

	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage() const;

	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);

	UFUNCTION(BlueprintCallable)
	EMatchMode GetMatchMode() const;

	UFUNCTION(BlueprintCallable)
	void SetMatchMode(EMatchMode Mode);
};
