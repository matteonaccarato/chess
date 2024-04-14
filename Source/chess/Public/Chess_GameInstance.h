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
	FString TextPlayer_1 = "Player 1";

	UPROPERTY(EditAnywhere)
	FString TextPlayer_2 = "Player 2";

	UPROPERTY(EditAnywhere)
	int32 ScorePlayer_1 = 0;

	UPROPERTY(EditAnywhere)
	int32 ScorePlayer_2 = 0;

	UPROPERTY(EditAnywhere)
	int32 GamesCounter = 0;

	UPROPERTY(EditAnywhere)
	FString StopwatchStr = "00:00";

	UPROPERTY(EditAnywhere)
	FString CurrentTurnMessage = "Current Player";

	UPROPERTY(EditAnywhere)
	EMatchMode MatchMode;


	/* PLAYER 1 */
	UFUNCTION(BlueprintCallable)
	FString GetPlayerText_1() const;
	void SetPlayerText_1(FString Text);

	UFUNCTION(BlueprintCallable)
	int32 GetScorePlayer_1() const;
	void IncrementScorePlayer_1();


	/* PLAYER 2 */
	UFUNCTION(BlueprintCallable)
	FString GetPlayerText_2() const;
	void SetPlayerText_2(FString Text);

	UFUNCTION(BlueprintCallable)
	int32 GetScorePlayer_2() const;
	void IncrementScorePlayer_2();


	/* GAMES COUNTER */
	UFUNCTION(BlueprintCallable)
	int32 GetGamesCounter() const;
	void SetGamesCounter(int32 Counter);
	void IncrementGamesCounter();

	/* STOPWATCH */
	UFUNCTION(BlueprintCallable)
	FString GetStopwatchStr() const;
	int Minutes = 0;
	int Seconds = 0;
	void SetStopwatchStr(FString Text);
	void IncrementStopwatch();


	/* TURN */
	UFUNCTION(BlueprintCallable)
	FString GetTurnMessage() const;

	UFUNCTION(BlueprintCallable)
	void SetTurnMessage(FString Message);


	/* MATCH MODE */
	UFUNCTION(BlueprintCallable)
	EMatchMode GetMatchMode() const;

	UFUNCTION(BlueprintCallable)
	void SetMatchMode(EMatchMode Mode);
};
